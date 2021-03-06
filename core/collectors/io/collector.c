/*******************************************************************************
** Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
** Copyright (c) 2007,2008 William Hachfeld. All Rights Reserved.
** Copyright (c) 2007-2012 Krell Institute.  All Rights Reserved.
**
** This library is free software; you can redistribute it and/or modify it under
** the terms of the GNU Lesser General Public License as published by the Free
** Software Foundation; either version 2.1 of the License, or (at your option)
** any later version.
**
** This library is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
** details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this library; if not, write to the Free Software Foundation, Inc.,
** 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*******************************************************************************/

/** @file
 *
 * Definition of the IO event tracing collector's runtime.
 *
 */

/* #define DEBUG 1 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "KrellInstitute/Messages/DataHeader.h"
#include "KrellInstitute/Messages/IO.h"
#include "KrellInstitute/Messages/IO_data.h"
#include "KrellInstitute/Messages/ToolMessageTags.h"
#include "KrellInstitute/Messages/Thread.h"
#include "KrellInstitute/Messages/ThreadEvents.h"
#include "KrellInstitute/Services/Assert.h"
#include "KrellInstitute/Services/Collector.h"
#include "KrellInstitute/Services/Common.h"
#include "KrellInstitute/Services/Context.h"
#include "KrellInstitute/Services/Data.h"
#include "KrellInstitute/Services/Time.h"
#include "KrellInstitute/Services/Timer.h"
#include "KrellInstitute/Services/Unwind.h"
#include "KrellInstitute/Services/TLS.h"
#include "IOTraceableFunctions.h"
#include "monitor.h"

// FIXME: What include are these defined in?
extern bool cbtf_connected_to_mrnet();
extern bool cbtf_mpi_init_done();

/** String uniquely identifying this collector. */
#if defined(PROFILE)
const char* const cbtf_collector_unique_id = "iop";
#else
#if defined(EXTENDEDTRACE)
const char* const cbtf_collector_unique_id = "iot";
#else
const char* const cbtf_collector_unique_id = "io";
#endif
#endif
#if defined(CBTF_SERVICE_USE_FILEIO)
const char* const data_suffix = "cbtf-data";
#endif


/** Number of overhead frames in each stack frame to be skipped. */
#if defined(PROFILE)
const unsigned OverheadFrameCount = 2;
#else
#if defined(CBTF_SERVICE_USE_OFFLINE)
const unsigned OverheadFrameCount = 1;
#else
#if defined(__linux) && defined(__ia64)
const unsigned OverheadFrameCount = 2 /*3*/;
#else
const unsigned OverheadFrameCount = 2;
#endif
#endif
#endif

/*
 * NOTE: For some reason GCC doesn't like it when the following three macros are
 *       replaced with constant unsigned integers. It complains about the arrays
 *       in the tls structure being "variable-size type declared outside of any
 *       function" even though the size IS constant... Maybe this can be fixed?
 */

#if defined(PROFILE)
#define MaxFramesPerStackTrace 100
#else
/** Maximum number of frames to allow in each stack trace. */
/* what is a reasonable default here. 32? */
#define MaxFramesPerStackTrace 64
#endif

#if defined(PROFILE)
#define StackTraceBufferSize (CBTF_BlobSizeFactor * 512)
#else
/** Number of stack trace entries in the tracing buffer. */
/** event.stacktrace buffer is 64*8=512 bytes */
/** allows for 6 unique stacktraces (384*8/512) */
#define StackTraceBufferSize (CBTF_BlobSizeFactor * 384)
#endif


/** Number of event entries in the tracing buffer. */
/** CBTF_io_event is 32 bytes , CBTF_iot_event is 80 bytes */
#if defined(EXTENDEDTRACE)
#define EventBufferSize (CBTF_BlobSizeFactor * 140)
#define PathBufferSize  (CBTF_BlobSizeFactor * 2048)
/* FIXME: is this thread safe? */
char currentpathname[PATH_MAX];
#elif !defined(PROFILE)
#define EventBufferSize (CBTF_BlobSizeFactor * 415)
#endif

/** Type defining the items stored in thread-local storage. */
typedef struct {

    CBTF_DataHeader header;  /**< Header for following data blob. */
#if defined(PROFILE)
    CBTF_io_profile_data data;
#else
#if defined(EXTENDEDTRACE)
    CBTF_io_exttrace_data data;              /**< Actual data blob. */
#else
    CBTF_io_trace_data data;              /**< Actual data blob. */
#endif
#endif

    /** Tracing buffer. */
#if defined(PROFILE)
    struct {
        uint64_t stacktraces[StackTraceBufferSize];  /**< Stack traces. */
        uint64_t time[StackTraceBufferSize];  /**< Stack traces. */
        uint8_t count[StackTraceBufferSize];  /**< Stack traces. */
    } buffer;
#else
    struct {
        uint64_t stacktraces[StackTraceBufferSize];  /**< Stack traces. */
#if defined(EXTENDEDTRACE)
        CBTF_iot_event events[EventBufferSize]; /**< IO call events. */
	char pathnames[PathBufferSize];                 /**< pathname buffer */
#else
        CBTF_io_event events[EventBufferSize];          /**< IO call events. */
#endif
    } buffer;
#endif
    
#if defined (CBTF_SERVICE_USE_OFFLINE)
    char CBTF_io_traced[PATH_MAX];
#endif
    
    /** Nesting depth within the IO function wrappers. */
    unsigned nesting_depth;
    bool do_trace;
    bool defer_sampling;
} TLS;

#ifndef NDEBUG
static bool IsCollectorDebugEnabled = false;
#endif


#if defined(USE_EXPLICIT_TLS)

/**
 * Key used to look up our thread-local storage. This key <em>must</em> be
 * unique from any other key used by any of the CBTF services.
 */
#if defined(PROFILE)
static const uint32_t TLSKey = 0x00001EF9;
#else
#if defined(EXTENDEDTRACE)
static const uint32_t TLSKey = 0x00001EF8;
#else
static const uint32_t TLSKey = 0x00001EF7;
#endif
#endif

int io_init_tls_done = 0;

#else

/** Thread-local storage. */
static __thread TLS the_tls;

#endif

void defer_trace(bool defer_tracing) {
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);
    tls->do_trace = defer_tracing;
}


/**
 * Initialize the performance data header and blob contained within the given
 * thread-local storage. This function <em>must</em> be called before any of
 * the collection routines attempts to add a message.
 *
 * @param tls    Thread-local storage to be initialized.
 */
static void initialize_data(TLS* tls)
{
    Assert(tls != NULL);

    tls->nesting_depth = 0;

    tls->header.time_begin = CBTF_GetTime();
    tls->header.time_end = 0;
    tls->header.addr_begin = ~0;
    tls->header.addr_end = 0;
    
    /* Re-initialize the actual data blob */
    tls->data.stacktraces.stacktraces_val = tls->buffer.stacktraces;
    tls->data.stacktraces.stacktraces_len = 0;
#if defined(PROFILE)
    tls->data.count.count_val = tls->buffer.count;
    tls->data.count.count_len = 0;
    tls->data.time.time_val = tls->buffer.time;
    tls->data.time.time_len = 0;
#else
    tls->data.events.events_len = 0;
    tls->data.events.events_val = tls->buffer.events;
#endif

    /* Re-initialize the sampling buffer */
    memset(tls->buffer.stacktraces, 0, sizeof(tls->buffer.stacktraces));
#if defined(PROFILE)
    memset(tls->buffer.count, 0, sizeof(tls->buffer.count));
    memset(tls->buffer.time, 0, sizeof(tls->buffer.time));
#else
    memset(tls->buffer.events, 0, sizeof(tls->buffer.events));
#if defined(EXTENDEDTRACE)
    tls->data.pathnames.pathnames_len = 1;
    //tls->buffer.pathnames[0] = 0;
    tls->data.pathnames.pathnames_val = tls->buffer.pathnames;
    memset(tls->buffer.pathnames, 0, sizeof(tls->buffer.pathnames));
#endif
#endif
}


/**
 * Update the performance data header contained within the given thread-local
 * storage with the specified time. Insures that the time interval defined by
 * time_begin and time_end contain the specified time.
 *
 * @param tls     Thread-local storage to be updated.
 * @param time    Time with which to update.
 */
inline void update_header_with_time(TLS* tls, uint64_t time)
{
    Assert(tls != NULL);

    if (time < tls->header.time_begin)
    {
        tls->header.time_begin = time;
    }
    if (time >= tls->header.time_end)
    {
        tls->header.time_end = time + 1;
    }
}



/**
 * Update the performance data header contained within the given thread-local
 * storage with the specified address. Insures that the address range defined
 * by addr_begin and addr_end contain the specified address.
 *
 * @param tls     Thread-local storage to be updated.
 * @param addr    Address with which to update.
 */
inline void update_header_with_address(TLS* tls, uint64_t addr)
{
    Assert(tls != NULL);

    if (addr < tls->header.addr_begin)
    {
        tls->header.addr_begin = addr;
    }
    if (addr >= tls->header.addr_end)
    {
        tls->header.addr_end = addr + 1;
    }
}


/**
 * Send events.
 *
 * Sends all the events in the tracing buffer to the framework for storage in 
 * the experiment's database. Then resets the tracing buffer to the empty state.
 * This is done regardless of whether or not the buffer is actually full.
 * NO DEBUG PRINT STATEMENTS HERE.
 */
static void send_samples(TLS *tls)
{
    Assert(tls != NULL);

    tls->header.id = strdup(cbtf_collector_unique_id);
    tls->header.time_end = CBTF_GetTime();
    /* rank is not filled until mpi_init finished. safe to set here*/
    tls->header.rank = monitor_mpi_comm_rank();

#ifndef NDEBUG
	if (IsCollectorDebugEnabled) {
	    fprintf(stderr, "[%ld,%d] io send_samples:\n",tls->header.pid, tls->header.omp_tid);
	    fprintf(stderr, "[%ld,%d] time_range(%lu,%lu) addr range[%lx, %lx] stacktraces_len(%d)\n",
		tls->header.pid, tls->header.omp_tid,
		tls->header.time_begin,tls->header.time_end,
		tls->header.addr_begin,tls->header.addr_end,
		tls->data.stacktraces.stacktraces_len
		);
	}
#endif

#if defined(PROFILE)
    cbtf_collector_send(&(tls->header), (xdrproc_t)xdr_CBTF_io_profile_data, &(tls->data));
#else
#if defined(EXTENDEDTRACE)
    cbtf_collector_send(&(tls->header), (xdrproc_t)xdr_CBTF_io_exttrace_data, &(tls->data));
#else
    cbtf_collector_send(&(tls->header), (xdrproc_t)xdr_CBTF_io_trace_data, &(tls->data));
#endif
#endif

    /* Re-initialize the data blob's header */
    initialize_data(tls);
}

/**
 * Start an event.
 *
 * Called by the IO function wrappers each time an event is to be started.
 * Initializes the event record and increments the wrappers nesting depth.
 *
 * @param event    Event to be started.
 */
/*
NO DEBUG PRINT STATEMENTS HERE.
*/
#if defined(PROFILE)
void io_start_event(CBTF_iop_event* event)
#else
#if defined(EXTENDEDTRACE)
void io_start_event(CBTF_iot_event* event)
#else
void io_start_event(CBTF_io_event* event)
#endif
#endif
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    /* Increment the IO function wrapper nesting depth */
    ++tls->nesting_depth;

    /* Initialize the event record. */

#if defined(PROFILE)
    memset(event, 0, sizeof(CBTF_iop_event));
#else
#if defined(EXTENDEDTRACE)
    memset(event, 0, sizeof(CBTF_iot_event));
    memset(currentpathname,0, sizeof(currentpathname));
#else
    memset(event, 0, sizeof(CBTF_io_event));
#endif
#endif
}


    
/**
 * Record an event.
 *
 * Called by the IO function wrappers each time an event is to be recorded.
 * Extracts the stack trace from the current thread context and places it, along
 * with the specified event record, into the tracing buffer. When the tracing
 * buffer is full, it is sent to the framework for storage in the experiment's
 * database.
 *
 * @param event       Event to be recorded.
 * @param function    Address of the IO function for which the event is being
 *                    recorded.
 * NO DEBUG PRINT STATEMENTS HERE IF TRACING "write, __libc_write".
 */
#if defined(PROFILE)
void io_record_event(const CBTF_iop_event* event, uint64_t function)
#else
#if defined(EXTENDEDTRACE)
void io_record_event(const CBTF_iot_event* event, uint64_t function)
#else
void io_record_event(const CBTF_io_event* event, uint64_t function)
#endif
#endif
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    tls->do_trace = false;

    uint64_t stacktrace[MaxFramesPerStackTrace];
    unsigned stacktrace_size = 0;
    unsigned entry = 0, start, i;

#ifdef DEBUG
#if defined(PROFILE)
fprintf(stderr,"ENTERED io_record_event, sizeof event=%d, sizeof stacktrace=%d, NESTING=%d\n",sizeof(CBTF_iop_event),sizeof(stacktrace),tls->nesting_depth);
#else
#if defined(EXTENDEDTRACE)
fprintf(stderr,"ENTERED io_record_event, sizeof event=%d, sizeof stacktrace=%d, NESTING=%d\n",sizeof(CBTF_iot_event),sizeof(stacktrace),tls->nesting_depth);
#else
fprintf(stderr,"ENTERED io_record_event, sizeof event=%d, sizeof stacktrace=%d, NESTING=%d\n",sizeof(CBTF_io_event),sizeof(stacktrace),tls->nesting_depth);
#endif
#endif
#endif


#if defined(EXTENDEDTRACE)
    /* need to search pathnames to see if this pathname already exists. */
    unsigned pathindex = 0;
    int curpathlen = 0;
    if (currentpathname[0] > 0) {
	curpathlen = strlen(currentpathname);
    }

    for(start = 0, i = 0;
	(i < curpathlen) &&
	    ((start + i) < tls->data.pathnames.pathnames_len);
	++i)
    {
	/* Do the i'th charactes differ? */
	if(currentpathname[i] != tls->buffer.pathnames[start + i]) {
	    
	    /* Advance in the tracing buffer to the end of this stack trace */
	    for(start += i;
		(tls->buffer.pathnames[start] != 0) &&
		    (start < tls->data.pathnames.pathnames_len);
		++start);
	    
	    /* Advance in the pathnames buffer past the terminating zero */
	    ++start;

	    /* Begin comparing at the zero'th frame again */
	    i = 0;
	}
    }

    /* Did we find a match in the pathnames buffer? */
    if(i == curpathlen) {
	entry = pathindex = start;
#ifdef DEBUG
	char *s = &(tls->buffer.pathnames[entry]);
	printf("iot_record_event: tls->buffer.pathnames[%d]=%s, currentpathname=%s\n",entry, s, currentpathname );
#endif

    /* Otherwise add this pathname to the pathnames buffer */
    } else {
	
	/* Send events if there is insufficient room for this pathname */
	if((tls->data.pathnames.pathnames_len + curpathlen + 1) >=
	   PathBufferSize) {
#ifdef DEBUG
fprintf(stderr,"PathBufferSize is full, call send_samples\n");
#endif
	    send_samples(tls);
	}
	
	/* Add each char in the pathname to the pathnames buffer. */	
	entry = tls->data.pathnames.pathnames_len;
	for(i = 0; i < curpathlen; ++i) {
	    
	    /* Add the i'th frame to the tracing buffer */
	    tls->buffer.pathnames[entry + i] = currentpathname[i];
	}

	pathindex = entry;
	
	/* Add a terminating zero  to the pathnames buffer */
	tls->buffer.pathnames[entry + curpathlen] = 0;
	
	/* Set the new size of the pathnames buffer */
	tls->data.pathnames.pathnames_len += (curpathlen + 1);
	
    }
#endif

    /* Decrement the IO function wrapper nesting depth */
    --tls->nesting_depth;

    /*
     * Don't record events for any recursive calls to our IO function wrappers.
     * The place where this occurs is when the IO implemetnation calls itself.
     * We don't record that data here because we are primarily interested in
     * direct calls by the application to the IO library - not in the internal
     * implementation details of that library.
     */
    if(tls->nesting_depth > 0) {
#ifdef DEBUG
	fprintf(stderr,"io_record_event RETURNS EARLY DUE TO NESTING\n");
#endif
	return;
    }
    
    /* Newer versions of libunwind now make io calls (open a file in /proc/<self>/maps)
     * that cause a thread lock in the libunwind dwarf parser. We are not interested in
     * any io done by libunwind while we get the stacktrace for the current context.
     * So we need to bump the nesting_depth before requesting the stacktrace and
     * then decrement nesting_depth after aquiring the stacktrace
     */

    ++tls->nesting_depth;
    /* Obtain the stack trace from the current thread context */
    CBTF_GetStackTraceFromContext(NULL, FALSE, OverheadFrameCount,
				    MaxFramesPerStackTrace,
				    &stacktrace_size, stacktrace);
    --tls->nesting_depth;

#if defined(PROFILE)

    bool_t stack_already_exists = FALSE;

    if(stacktrace_size > 0)
	stacktrace[0] = function;

    int j;
    int stackindex = 0;
    /* search individual stacks via count/indexing array */
    for (i = 0; i < tls->data.count.count_len ; i++ )
    {
	/* a count > 0 indexes the top of stack in the data buffer. */
	/* a count == 255 indicates this stack is at the count limit. */

	if (tls->buffer.count[i] == 0) {
	    continue;
	}
	if (tls->buffer.count[i] == 255) {
	    continue;
	}

	/* see if the stack addresses match */
	for (j = 0; j < stacktrace_size ; j++ )
	{
	    if ( tls->buffer.stacktraces[i+j] != stacktrace[j] ) {
		   break;
	    }
	}

	if ( j == stacktrace_size) {
	    stack_already_exists = TRUE;
	    stackindex = i;
	}
    }

    /* if the stack already exisits in the buffer, update its count
     * and return. If the stack is already at the count limit.
    */
    if (stack_already_exists && tls->buffer.count[stackindex] < 255 ) {
	/* update count for this stack */
	tls->buffer.count[stackindex] = tls->buffer.count[stackindex] + 1;
	tls->buffer.time[stackindex] += event->time;
	// reset do_trace to true.
	tls->do_trace = true;
	return;
    }

    /* sample buffer has no room for these stack frames.*/
    int buflen = tls->data.stacktraces.stacktraces_len + stacktrace_size;
    if ( buflen > StackTraceBufferSize) {
	/* send the current sample buffer. (will init a new buffer) */
	send_samples(tls);
    }

    /* add frames to sample buffer, compute addresss range */
    for (i = 0; i < stacktrace_size ; i++)
    {
	/* always add address to buffer bt */
	tls->buffer.stacktraces[tls->data.stacktraces.stacktraces_len] = stacktrace[i];

	/* top of stack indicated by a positive count. */
	/* all other elements are 0 */
	if (i > 0 ) {
	    tls->buffer.count[tls->data.count.count_len] = 0;
	} else {
	    tls->buffer.count[tls->data.count.count_len] = 1;
	    tls->buffer.time[tls->data.time.time_len] = event->time;
	}

	if (stacktrace[i] < tls->header.addr_begin ) {
	    tls->header.addr_begin = stacktrace[i];
	}
	if (stacktrace[i] > tls->header.addr_end ) {
	    tls->header.addr_end = stacktrace[i];
	}
	tls->data.stacktraces.stacktraces_len++;
	tls->data.count.count_len++;
	tls->data.time.time_len++;
    }

#else
    /*
     * Replace the first entry in the call stack with the address of the IO
     * function that is being wrapped. On most platforms, this entry will be
     * the address of the call site of io_record_event() within the calling
     * wrapper. On IA64, because OverheadFrameCount is one higher, it will be
     * the mini-tramp for the wrapper that is calling io_record_event().
     */
    if(stacktrace_size > 0)
	stacktrace[0] = function;
    
    /*
     * Search the tracing buffer for an existing stack trace matching the stack
     * trace from the current thread context. For now do a simple linear search.
     */
    for(start = 0, i = 0;
	(i < stacktrace_size) &&
	    ((start + i) < tls->data.stacktraces.stacktraces_len);
	++i)
	
	/* Do the i'th frames differ? */
	if(stacktrace[i] != tls->buffer.stacktraces[start + i]) {
	    
	    /* Advance in the tracing buffer to the end of this stack trace */
	    for(start += i;
		(tls->buffer.stacktraces[start] != 0) &&
		    (start < tls->data.stacktraces.stacktraces_len);
		++start);
	    
	    /* Advance in the tracing buffer past the terminating zero */
	    ++start;
	    
	    /* Begin comparing at the zero'th frame again */
	    i = 0;
	    
	}
    
    /* Did we find a match in the tracing buffer? */
    if(i == stacktrace_size)
	entry = start;
    
    /* Otherwise add this stack trace to the tracing buffer */
    else {
	
	/* Send events if there is insufficient room for this stack trace */
	if((tls->data.stacktraces.stacktraces_len + stacktrace_size + 1) >=
	   StackTraceBufferSize) {
#ifdef DEBUG
fprintf(stderr,"StackTraceBufferSize is full, call send_samples\n");
#endif
	    send_samples(tls);
	}
	
	/* Add each frame in the stack trace to the tracing buffer. */	
	entry = tls->data.stacktraces.stacktraces_len;
	for(i = 0; i < stacktrace_size; ++i) {
	    
	    /* Add the i'th frame to the tracing buffer */
	    tls->buffer.stacktraces[entry + i] = stacktrace[i];
	    
	    /* Update the address interval in the data blob's header */
	    if(stacktrace[i] < tls->header.addr_begin)
		tls->header.addr_begin = stacktrace[i];
	    if(stacktrace[i] > tls->header.addr_end)
		tls->header.addr_end = stacktrace[i];
	    
	}
	
	/* Add a terminating zero frame to the tracing buffer */
	tls->buffer.stacktraces[entry + stacktrace_size] = 0;
	
	/* Set the new size of the tracing buffer */
	tls->data.stacktraces.stacktraces_len += (stacktrace_size + 1);
	
    }
    
    /* Add a new entry for this event to the tracing buffer. */
#if defined(EXTENDEDTRACE)
    memcpy(&(tls->buffer.events[tls->data.events.events_len]),
	   event, sizeof(CBTF_iot_event));
    tls->buffer.events[tls->data.events.events_len].pathindex = pathindex;
#else
    memcpy(&(tls->buffer.events[tls->data.events.events_len]),
	   event, sizeof(CBTF_io_event));
#endif
    tls->buffer.events[tls->data.events.events_len].stacktrace = entry;
    tls->data.events.events_len++;
    
    /* Send events if the tracing buffer is now filled with events */
    if(tls->data.events.events_len == EventBufferSize) {
#ifdef DEBUG
fprintf(stderr,"Event Buffer is full, call send_samples\n");
#endif
	send_samples(tls);
    }
#endif

    tls->do_trace = true;
}



/**
 * Called by the CBTF collector service in order to start data collection.
 */
void cbtf_collector_start(const CBTF_DataHeader* const header)
{
/**
 * Start tracing.
 *
 * Starts IO extended event tracing for the thread executing this function.
 * Initializes the appropriate thread-local data structures.
 *
 * @param arguments    Encoded function arguments.
 */
    /* Create and access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = malloc(sizeof(TLS));
    Assert(tls != NULL);
    CBTF_SetTLS(TLSKey, tls);
    io_init_tls_done = true;
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    tls->defer_sampling=true;
    tls->do_trace=false;

    /* Decode the passed function arguments */
    // Need to handle the arguments...
    CBTF_io_start_sampling_args args;
    memset(&args, 0, sizeof(args));
    
#if defined(CBTF_SERVICE_USE_OFFLINE)

    /* If CBTF_IO_TRACED is set to a valid list of io functions, trace only
     * those functions.
     * If CBTF_IO_TRACED is set and is empty, trace all functions.
     * For any misspelled function name in CBTF_IO_TRACED, silently ignore.
     * If all names in CBTF_IO_TRACED are misspelled or not part of
     * TraceableFunctions, nothing will be traced.
     */
    const char* io_traced = getenv("CBTF_IO_TRACED");

    if (io_traced != NULL && strcmp(io_traced,"") != 0) {
	strcpy(tls->CBTF_io_traced,io_traced);
    } else {
	strcpy(tls->CBTF_io_traced,traceable);
    }
#endif

    memcpy(&tls->header, header, sizeof(CBTF_DataHeader));

    /* Initialize the actual data blob */
    initialize_data(tls);

#ifndef NDEBUG
    IsCollectorDebugEnabled = (getenv("CBTF_DEBUG_COLLECTOR") != NULL);
    if (IsCollectorDebugEnabled) {
	fprintf(stderr,"[%ld,%d] ENTERED cbtf_collector_start.\n",tls->header.pid, tls->header.omp_tid);
    }
#endif

    /* Initialize the IO function wrapper nesting depth */
    tls->nesting_depth = 0;
 
    /* Begin sampling */
    tls->header.time_begin = CBTF_GetTime();

#if !defined(CBTF_SERVICE_USE_FILEIO)
    // in mpi programs we wish to defer tracing until after the
    // mpi init process is finished.
    if (cbtf_connected_to_mrnet()) {
	tls->defer_sampling = false;
	tls->do_trace = true;
#ifndef NDEBUG
	if (IsCollectorDebugEnabled) {
	    fprintf(stderr,"[%ld:%d] cbtf_collector_start IO tracing active.\n",
		tls->header.pid, tls->header.omp_tid);
	}
#endif
    } else {
#ifndef NDEBUG
	if (IsCollectorDebugEnabled) {
	    fprintf(stderr,"[%ld:%d] cbtf_collector_start IO tracing not enabled due to no mrnet connection.\n",
		tls->header.pid, tls->header.omp_tid);
	}
#endif
    }
#else
    if (cbtf_mpi_init_done()) {
	tls->defer_sampling = false;
	tls->do_trace = true;
#ifndef NDEBUG
	if (IsCollectorDebugEnabled) {
	    fprintf(stderr,"[%ld:%d] cbtf_collector_start IO tracing active.\n",
		tls->header.pid, tls->header.omp_tid);
	}
#endif
    } else {
#ifndef NDEBUG
	if (IsCollectorDebugEnabled) {
	    fprintf(stderr,"[%ld:%d] cbtf_collector_start IO tracing not enabled due to in mpi startup.\n",
		tls->header.pid, tls->header.omp_tid);
	}
#endif
    }
#endif
}


/**
 * Called by the CBTF collector service in order to pause data collection.
 */
void cbtf_collector_pause()
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    if (tls == NULL)
	return;

    tls->defer_sampling = true;
    tls->do_trace = false;
}



/**
 * Called by the CBTF collector service in order to resume data collection.
 */
void cbtf_collector_resume()
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    if (tls == NULL)
	return;

    tls->defer_sampling = false;
    tls->do_trace = true;
}


#ifdef USE_EXPLICIT_TLS
void destroy_explicit_tls() {
    TLS* tls = CBTF_GetTLS(TLSKey);
    /* Destroy our thread-local storage */
    if (tls) {
        free(tls);
    }
    CBTF_SetTLS(TLSKey, NULL);
}
#endif


/**
 * Stop tracing.
 *
 * Stops IO event tracing for the thread executing this function.
 * Sends any events remaining in the buffer.
 *
 * @param arguments    Encoded (unused) function arguments.
 */
void cbtf_collector_stop()
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif

    Assert(tls != NULL);

    tls->header.time_end = CBTF_GetTime();

    /* Stop sampling */
    defer_trace(0);

    /* Are there any unsent samples? */
#if defined(PROFILE)
    if(tls->data.count.count_len > 0 || tls->data.stacktraces.stacktraces_len > 0) {
	send_samples(tls);
    }
#else
    if(tls->data.events.events_len > 0 || tls->data.stacktraces.stacktraces_len > 0) {
	send_samples(tls);
    }
#endif

    /* Destroy our thread-local storage */
#ifdef CBTF_SERVICE_USE_EXPLICIT_TLS
    free(tls);
    CBTF_SetTLS(TLSKey, NULL);
#endif
}

bool_t io_do_trace(const char* traced_func)
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls;
    if (io_init_tls_done) {
        tls = CBTF_GetTLS(TLSKey);
    } else {
	return FALSE;
    }
#else
    TLS* tls = &the_tls;
#endif
    // we may be called early in process startup but we have
    // not called collectors start collecting.  Just return false.
    if (tls == NULL) {
	return FALSE;
    }

#if defined (CBTF_SERVICE_USE_OFFLINE)

    if (tls->do_trace == false) {
	if (tls->nesting_depth > 1)
	    --tls->nesting_depth;
	return false;
    }

    /* See if this function has been selected for tracing */
    char *tfptr, *saveptr, *tf_token;
    tfptr = strdup(tls->CBTF_io_traced);
    int i;
    for (i = 1;  ; i++, tfptr = NULL) {
	tf_token = strtok_r(tfptr, ":,", &saveptr);
	if (tf_token == NULL)
	    break;
	if ( strcmp(tf_token,traced_func) == 0) {
	
    	    if (tfptr)
		free(tfptr);
	    return TRUE;
	}
    }

    /* Remove any nesting due to skipping io_start_event/io_record_event for
     * potentially nested io calls that are not being traced.
     */

    if (tls->nesting_depth > 1)
	--tls->nesting_depth;

    return false;
#else
    /* Always return true for dynamic instrumentors since these collectors
     * can be passed a list of traced functions for use with executeInPlaceOf.
     */

    if (tls->do_trace == false) {
	if (tls->nesting_depth > 1)
	    --tls->nesting_depth;
	return false;
    }
    return true;
#endif
}
