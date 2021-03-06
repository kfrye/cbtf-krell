/*******************************************************************************
** Copyright (c) 2011-2012 Krell Institute.  All Rights Reserved.
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
 * Definition of the Mem event tracing collector's runtime.
 *
 */

/* #define DEBUG 1 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "KrellInstitute/Messages/DataHeader.h"
#include "KrellInstitute/Messages/Mem.h"
#include "KrellInstitute/Messages/Mem_data.h"
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
#include "MemTraceableFunctions.h"
#include "monitor.h"

#ifdef USE_EXPLICIT_TLS
#undef USE_EXPLICIT_TLS
#endif

extern bool cbtf_connected_to_mrnet();
extern bool cbtf_mpi_init_done();

/** String uniquely identifying this collector. */
const char* const cbtf_collector_unique_id = "mem";
#if defined(CBTF_SERVICE_USE_FILEIO)
const char* const data_suffix = "cbtf-data";
#endif

/** Number of overhead frames in each stack frame to be skipped. */
#if defined(CBTF_SERVICE_USE_OFFLINE)
const unsigned OverheadFrameCount = 2;
#else
#if defined(__linux) && defined(__ia64)
const unsigned OverheadFrameCount = 2 /*3*/;
#else
const unsigned OverheadFrameCount = 2;
#endif
#endif

/*
 * NOTE: For some reason GCC doesn't like it when the following three macros are
 *       replaced with constant unsigned integers. It complains about the arrays
 *       in the tls structure being "variable-size type declared outside of any
 *       function" even though the size IS constant... Maybe this can be fixed?
 */

/** Maximum number of frames to allow in each stack trace. */
/* what is a reasonable default here. 32? */
#define MaxFramesPerStackTrace 48

/** Number of stack trace entries in the tracing buffer. */
/** event.stacktrace buffer is 64*8=512 bytes */
/** allows for 6 unique stacktraces (384*8/512) */
#define StackTraceBufferSize (CBTF_BlobSizeFactor * 384)


/** Number of event entries in the tracing buffer. */
/** CBTF_mem_event is 32 bytes */
/** CBTF_memt_event is 64 bytes */
// FIXME:  300 was too high given the current stack buffer size
// and the size of a memt event.  Should find the best fit going forward.
//#define EventBufferSize (CBTF_BlobSizeFactor * 200)
#define EventBufferSize (CBTF_BlobSizeFactor * 100)

/** Type defining the items stored in thread-local storage. */
typedef struct {

    CBTF_DataHeader header;  /**< Header for following data blob. */
    CBTF_mem_exttrace_data data;        /**< Actual data blob. */

    /** Tracing buffer. NOTE: using exended data memt for buffer*/
    struct {
	uint64_t stacktraces[StackTraceBufferSize];  /**< Stack traces. */
	CBTF_memt_event events[EventBufferSize];     /**< Mem call events. */
    } buffer;

#if defined (CBTF_SERVICE_USE_OFFLINE)
    char CBTF_mem_traced[PATH_MAX];
#endif
    
    /** Nesting depth within the mem wrappers. */
    unsigned nesting_depth;
    bool_t do_trace;
    bool_t defer_sampling;
    int event_count;
} TLS;

#ifndef NDEBUG
static bool IsCollectorDebugEnabled = false;
#endif

#if defined(USE_EXPLICIT_TLS)

/**
 * Key used to look up our thread-local storage. This key <em>must</em> be
 * unique from any other key used by any of the CBTF services.
 */
static const uint32_t TLSKey = 0x00001EFA;
int mem_init_tls_done = 0;

#else

/** Thread-local storage. */
static __thread TLS the_tls;

#endif

void defer_trace(int defer_tracing) {
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
    tls->data.stacktraces.stacktraces_len = 0;
    tls->data.stacktraces.stacktraces_val = tls->buffer.stacktraces;
    tls->data.events.events_len = 0;
    tls->data.events.events_val = tls->buffer.events;

    /* Re-initialize the sampling buffer */
    memset(tls->buffer.stacktraces, 0, sizeof(tls->buffer.stacktraces));
    memset(tls->buffer.events, 0, sizeof(tls->buffer.events));
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
 */
static void send_samples(TLS *tls)
{
    int saved_do_trace = tls->do_trace;
    tls->do_trace = 0;

    tls->header.id = strdup(cbtf_collector_unique_id);
    tls->header.time_end = CBTF_GetTime();
    /* rank is not filled until mpi_init finished. safe to set here*/
    tls->header.rank = monitor_mpi_comm_rank();

#ifndef NDEBUG
    if (IsCollectorDebugEnabled) {
	fprintf(stderr, "[%ld:%d] mem send_samples:\n",tls->header.pid, tls->header.omp_tid);
	fprintf(stderr, "[%ld:%d] time_range(%lu,%lu) addr range[%lx, %lx] stacktraces_len(%d) events_len(%d)\n",
		tls->header.pid, tls->header.omp_tid,
		tls->header.time_begin,tls->header.time_end,
		tls->header.addr_begin,tls->header.addr_end,
		tls->data.stacktraces.stacktraces_len,
		tls->data.events.events_len);
    }
#endif

    cbtf_collector_send(&(tls->header), (xdrproc_t)xdr_CBTF_mem_exttrace_data, &(tls->data));

    /* Re-initialize the data blob's header */
    initialize_data(tls);
    tls->do_trace = saved_do_trace;
}

/**
 * Start an event.
 *
 * Called by the Mem function wrappers each time an event is to be started.
 * Initializes the event record and increments the wrappers nesting depth.
 *
 * @param event    Event to be started.
 */
/*
NO DEBUG PRINT STATEMENTS HERE.
*/
void mem_start_event(CBTF_memt_event* event)
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    int saved_do_trace = tls->do_trace;
    tls->do_trace = 0;

    /* Increment the Mem function wrapper nesting depth */
    ++tls->nesting_depth;

    /* Initialize the event record. */
    memset(event, 0, sizeof(CBTF_memt_event));

    tls->do_trace = saved_do_trace;
}


    
/**
 * Record an event.
 *
 * Called by the Mem function wrappers each time an event is to be recorded.
 * Extracts the stack trace from the current thread context and places it, along
 * with the specified event record, into the tracing buffer. When the tracing
 * buffer is full, it is sent to the framework for storage in the experiment's
 * database.
 *
 * @param event       Event to be recorded.
 * @param function    Address of the Mem function for which the event is being
 *                    recorded.
 */
/*
NO DEBUG PRINT STATEMENTS HERE IF TRACING "write, __libc_write".
*/
void mem_record_event(const CBTF_memt_event* event, uint64_t function)
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    int saved_do_trace = tls->do_trace;
    tls->do_trace = 0;
    tls->event_count++;

    uint64_t stacktrace[MaxFramesPerStackTrace];
    unsigned stacktrace_size = 0;
    unsigned entry = 0, start, i;

    /* Decrement the Mem function wrapper nesting depth */
    --tls->nesting_depth;

    /*
     * Don't record events for any recursive calls to our Mem function wrappers.
     * The place where this occurs is when the Mem implemetnation calls itself.
     * We don't record that data here because we are primarily interested in
     * direct calls by the application to the Mem library - not in the internal
     * implementation details of that library.
     */
    if(tls->nesting_depth > 0) {
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

    /*
     * Replace the first entry in the call stack with the address of the Mem
     * function that is being wrapped. On most platforms, this entry will be
     * the address of the call site of mem_record_event() within the calling
     * wrapper. On IA64, because OverheadFrameCount is one higher, it will be
     * the mini-tramp for the wrapper that is calling mem_record_event().
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
	    send_samples(tls);
	}
	
	/* Add each frame in the stack trace to the tracing buffer. */	
	entry = tls->data.stacktraces.stacktraces_len;
	for(i = 0; i < stacktrace_size; ++i) {
	    if (stacktrace[i] != 0) {
//fprintf(stderr,"stack[%d]:%p\n",i,stacktrace[i]);   
	    /* Add the i'th frame to the tracing buffer */
	    tls->buffer.stacktraces[entry + i] = stacktrace[i];
	    }
	    
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
    memcpy(&(tls->buffer.events[tls->data.events.events_len]),
	   event, sizeof(CBTF_memt_event));
    tls->buffer.events[tls->data.events.events_len].stacktrace = entry;
    tls->data.events.events_len++;
    
    /* Send events if the tracing buffer is now filled with events */
    if(tls->data.events.events_len == EventBufferSize) {
	send_samples(tls);
    }

    tls->do_trace = saved_do_trace;
}



/**
 * Called by the CBTF collector service in order to start data collection.
 */
void cbtf_collector_start(const CBTF_DataHeader* const header)
{
/**
 * Start tracing.
 *
 * Starts Mem extended event tracing for the thread executing this function.
 * Initializes the appropriate thread-local data structures.
 *
 * @param arguments    Encoded function arguments.
 */
    /* Create and access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = malloc(sizeof(TLS));
    Assert(tls != NULL);
    CBTF_SetTLS(TLSKey, tls);
    mem_init_tls_done = 1;
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    tls->defer_sampling = 1;
    tls->do_trace = 0;
    tls->event_count = 0;
    IsCollectorDebugEnabled = (getenv("CBTF_DEBUG_COLLECTOR") != NULL);

    /* Decode the passed function arguments */
    // Need to handle the arguments...
    CBTF_mem_start_sampling_args args;
    memset(&args, 0, sizeof(args));
    
#if 0
    CBTF_DecodeParameters(arguments,
			  (xdrproc_t)xdr_CBTF_mem_start_sampling_args,
			   &args);
#endif

#if defined(CBTF_SERVICE_USE_OFFLINE)

    /* If CBTF_Mem_TRACED is set to a valid list of io functions, trace only
     * those functions.
     * If CBTF_Mem_TRACED is set and is empty, trace all functions.
     * For any misspelled function name in CBTF_Mem_TRACED, silently ignore.
     * If all names in CBTF_Mem_TRACED are misspelled or not part of
     * TraceableFunctions, nothing will be traced.
     */
    const char* mem_traced = getenv("CBTF_MEM_TRACED");

    if (mem_traced != NULL && strcmp(mem_traced,"") != 0) {
	strcpy(tls->CBTF_mem_traced,mem_traced);
    } else {
	strcpy(tls->CBTF_mem_traced,traceable);
    }
#endif

    memcpy(&tls->header, header, sizeof(CBTF_DataHeader));

    /* Initialize the actual data blob */
    initialize_data(tls);

    /* Initialize the mem function wrapper nesting depth */
    tls->nesting_depth = 0;
 
    /* Begin sampling */
    tls->header.time_begin = CBTF_GetTime();

#ifndef NDEBUG
    if (IsCollectorDebugEnabled) {
	fprintf(stderr,"[%ld:%d] cbtf_collector_start ENTERED.\n",
		tls->header.pid, tls->header.omp_tid);
    }
#endif

#if !defined(CBTF_SERVICE_USE_FILEIO)
    // in mpi programs we wish to defer tracing until after the
    // mpi init process is finished.
    if (cbtf_connected_to_mrnet()) {
	tls->defer_sampling = 0;
	tls->do_trace = 1;
#ifndef NDEBUG
	if (IsCollectorDebugEnabled) {
	    fprintf(stderr,"[%ld:%d] cbtf_collector_start mem tracing active.\n",
		tls->header.pid, tls->header.omp_tid);
	}
#endif
    } else {
#ifndef NDEBUG
	if (IsCollectorDebugEnabled) {
	    fprintf(stderr,"[%ld:%d] cbtf_collector_start mem tracing not enabled due to no mrnet connection.\n",
		tls->header.pid, tls->header.omp_tid);
	}
#endif
    }
#else
    if (cbtf_mpi_init_done()) {
	tls->defer_sampling = 0;
	tls->do_trace = 1;
#ifndef NDEBUG
	if (IsCollectorDebugEnabled) {
	    fprintf(stderr,"[%ld:%d] cbtf_collector_start mem tracing active.\n",
		tls->header.pid, tls->header.omp_tid);
	}
#endif
    } else {
#ifndef NDEBUG
	if (IsCollectorDebugEnabled) {
	    fprintf(stderr,"[%ld:%d] cbtf_collector_start mem tracing not enabled due to in mpi startup.\n",
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

    tls->defer_sampling = 1;
    tls->do_trace = 0;
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

    tls->defer_sampling = 0;
    tls->do_trace = 1;
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
 * Stops Mem event tracing for the thread executing this function.
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
    /* If there are events then there are stacktraces. */
    /* If there are stacktraces then there are events. */
    /* So this should be an and op rather than an or. */
    if(tls->data.events.events_len > 0 && tls->data.stacktraces.stacktraces_len > 0) {
	send_samples(tls);
    }

#ifndef NDEBUG
    if (IsCollectorDebugEnabled) {
	fprintf(stderr,"[%ld:%d] cbtf_collector_stop events:%d\n",
		tls->header.pid, tls->header.omp_tid,
		tls->event_count);
    }
#endif

    /* Destroy our thread-local storage */
#ifdef CBTF_SERVICE_USE_EXPLICIT_TLS
    free(tls);
    CBTF_SetTLS(TLSKey, NULL);
#endif
}

bool_t mem_do_trace(const char* traced_func)
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls;
    if (mem_init_tls_done) {
        tls = CBTF_GetTLS(TLSKey);
    } else {
	return FALSE;
    }
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

#if defined (CBTF_SERVICE_USE_OFFLINE)

    int saved_do_trace = tls->do_trace;
    if (tls->do_trace == 0) {
	if (tls->nesting_depth > 1)
	    --tls->nesting_depth;
	return FALSE;
    }

    /* See if this function has been selected for tracing */
    char *tfptr, *saveptr, *tf_token;

    tls->do_trace = 0;
    tfptr = strdup(tls->CBTF_mem_traced);

    int i;
    for (i = 1;  ; i++, tfptr = NULL) {
	tf_token = strtok_r(tfptr, ":,", &saveptr);
	if (tf_token == NULL)
	    break;
	if ( strcmp(tf_token,traced_func) == 0) {
	
    	    if (tfptr)
		free(tfptr);
	    tls->do_trace = saved_do_trace;
	    return TRUE;
	}
    }

    /* Remove any nesting due to skipping mem_start_event/mem_record_event for
     * potentially nested iop calls that are not being traced.
     */

    if (tls->nesting_depth > 1)
	--tls->nesting_depth;

    tls->do_trace = saved_do_trace;
    return FALSE;
#else
    /* Always return true for dynamic instrumentors since these collectors
     * can be passed a list of traced functions for use with executeInPlaceOf.
     */

    if (tls->do_trace == 0) {
	if (tls->nesting_depth > 1)
	    --tls->nesting_depth;
	return FALSE;
    }
    return TRUE;
#endif
}
