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
 * Definition of the Pthread event tracing collector's runtime.
 *
 */

/* #define DEBUG 1 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "KrellInstitute/Messages/DataHeader.h"
#include "KrellInstitute/Messages/Pthreads.h"
#include "KrellInstitute/Messages/Pthreads_data.h"
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
#include "PthreadTraceableFunctions.h"
#include "monitor.h"

/** String uniquely identifying this collector. */
const char* const cbtf_collector_unique_id = "pthreads";
#if defined(CBTF_SERVICE_USE_FILEIO)
const char* const data_suffix = "cbtf-data";
#endif


/** Number of overhead frames in each stack frame to be skipped. */
#if defined(CBTF_SERVICE_USE_OFFLINE)
const unsigned OverheadFrameCount = 1;
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
/** FIXME: VERIFY: CBTF_pthreadt_event is 32 bytes */
#define EventBufferSize (CBTF_BlobSizeFactor * 415)

/** Type defining the items stored in thread-local storage. */
typedef struct {

    /** Nesting depth within the Pthread function wrappers. */
    unsigned nesting_depth;

    CBTF_DataHeader header;  /**< Header for following data blob. */
    CBTF_pthreads_exttrace_data data;      /**< Actual data blob. */

    /** Tracing buffer. */
    struct {
        uint64_t stacktraces[StackTraceBufferSize];  /**< Stack traces. */
        CBTF_pthreadt_event events[EventBufferSize]; /**< Pthread call events. */
    } buffer;

#if defined (CBTF_SERVICE_USE_OFFLINE)
    char CBTF_pthreads_traced[PATH_MAX];
#endif
    int defer_sampling;
    int do_trace;

} TLS;

#if defined(USE_EXPLICIT_TLS)

/**
 * Key used to look up our thread-local storage. This key <em>must</em> be
 * unique from any other key used by any of the CBTF services.
 */
static const uint32_t TLSKey = 0x00001EFE ;
int pthreads_init_tls_done = 0;

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
	if (getenv("CBTF_DEBUG_COLLECTOR") != NULL) {
	    fprintf(stderr, "pthread send_samples:\n");
	    fprintf(stderr, "time_range(%lu,%lu) addr range[%lx, %lx] stacktraces_len(%d) events_len(%d)\n",
		tls->header.time_begin,tls->header.time_end,
		tls->header.addr_begin,tls->header.addr_end,
		tls->data.stacktraces.stacktraces_len,
		tls->data.events.events_len);
	}
#endif

    cbtf_collector_send(&(tls->header), (xdrproc_t)xdr_CBTF_pthreads_exttrace_data, &(tls->data));

    /* Re-initialize the data blob's header */
    initialize_data(tls);
}

/**
 * Start an event.
 *
 * Called by the Pthread function wrappers each time an event is to be started.
 * Initializes the event record and increments the wrappers nesting depth.
 *
 * @param event    Event to be started.
 */
/*
NO DEBUG PRINT STATEMENTS HERE.
*/
int pthreads_start_event(CBTF_pthreadt_event* event)
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    // return 0 to indicate we are not yet ready to start collecting
    // or have finished collecting and out tls is gone. This is mainly
    // an issue with explicit tls.
    if (tls == NULL)
	return 0;

    int saved_do_trace = tls->do_trace;
    tls->do_trace = 0;

    /* Increment the Mem function wrapper nesting depth */
    ++tls->nesting_depth;

    /* Initialize the event record. */
    memset(event, 0, sizeof(CBTF_pthreadt_event));

    tls->do_trace = saved_do_trace;
    return 1;
}


    
/**
 * Record an event.
 *
 * Called by the Pthread function wrappers each time an event is to be recorded.
 * Extracts the stack trace from the current thread context and places it, along
 * with the specified event record, into the tracing buffer. When the tracing
 * buffer is full, it is sent to the framework for storage in the experiment's
 * database.
 *
 * @param event       Event to be recorded.
 * @param function    Address of the Pthread function for which the event is being
 *                    recorded.
 */
void pthreads_record_event(const CBTF_pthreadt_event* event, uint64_t function)
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

    uint64_t stacktrace[MaxFramesPerStackTrace];
    unsigned stacktrace_size = 0;
    unsigned entry = 0, start, i;

#ifdef DEBUG
fprintf(stderr,"ENTERED pthreads_record_event, sizeof event=%d, sizeof stacktrace=%d, NESTING=%d\n",sizeof(CBTF_pthreadt_event),sizeof(stacktrace),tls->nesting_depth);
#endif


    /* Decrement the Pthread function wrapper nesting depth */
    --tls->nesting_depth;

    /*
     * Don't record events for any recursive calls to our Pthread function wrappers.
     * The place where this occurs is when the Pthread implemetnation calls itself.
     * We don't record that data here because we are primarily interested in
     * direct calls by the application to the Pthread library - not in the internal
     * implementation details of that library.
     */
    if(tls->nesting_depth > 0) {
#ifdef DEBUG
	fprintf(stderr,"pthreads_record_event RETURNS EARLY DUE TO NESTING\n");
#endif
	return;
    }
    
    ++tls->nesting_depth;
    /* Obtain the stack trace from the current thread context */
    CBTF_GetStackTraceFromContext(NULL, FALSE, OverheadFrameCount,
				    MaxFramesPerStackTrace,
				    &stacktrace_size, stacktrace);
    --tls->nesting_depth;

    /*
     * Replace the first entry in the call stack with the address of the Pthread
     * function that is being wrapped. On most platforms, this entry will be
     * the address of the call site of pthreads_record_event() within the calling
     * wrapper. On IA64, because OverheadFrameCount is one higher, it will be
     * the mini-tramp for the wrapper that is calling pthreads_record_event().
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
    memcpy(&(tls->buffer.events[tls->data.events.events_len]),
	   event, sizeof(CBTF_pthreadt_event));
    tls->buffer.events[tls->data.events.events_len].stacktrace = entry;
    tls->data.events.events_len++;
    
    /* Send events if the tracing buffer is now filled with events */
    if(tls->data.events.events_len == EventBufferSize) {
#ifdef DEBUG
fprintf(stderr,"Event Buffer is full, call send_samples\n");
#endif
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
 * Starts Pthread extended event tracing for the thread executing this function.
 * Initializes the appropriate thread-local data structures.
 *
 * @param arguments    Encoded function arguments.
 */
    /* Create and access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    // this is the one and only place for the collector
    // to setup explicit tls. if it fails here then assert.
    // Any other collector function accessing tls before this
    // malloc was early in process startup or mpi startup where
    // we are not yet ready to start our collector.
    TLS* tls = malloc(sizeof(TLS));
    Assert(tls != NULL);
    CBTF_SetTLS(TLSKey, tls);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    tls->defer_sampling=FALSE;

    /* Decode the passed function arguments */
    // Need to handle the arguments...
    CBTF_pthreads_start_sampling_args args;
    memset(&args, 0, sizeof(args));
    
#if 0
    CBTF_DecodeParameters(arguments,
			  (xdrproc_t)xdr_CBTF_pthreads_start_sampling_args,
			   &args);
#endif

#if defined(CBTF_SERVICE_USE_OFFLINE)

    /* If CBTF_PTHREAD_TRACED is set to a valid list of io functions, trace only
     * those functions.
     * If CBTF_PTHREAD_TRACED is set and is empty, trace all functions.
     * For any misspelled function name in CBTF_PTHREAD_TRACED, silently ignore.
     * If all names in CBTF_PTHREAD_TRACED are misspelled or not part of
     * TraceableFunctions, nothing will be traced.
     */
    const char* pthreads_traced = getenv("CBTF_PTHREAD_TRACED");

    if (pthreads_traced != NULL && strcmp(pthreads_traced,"") != 0) {
	strcpy(tls->CBTF_pthreads_traced,pthreads_traced);
    } else {
	strcpy(tls->CBTF_pthreads_traced,traceable);
    }
#endif

    memcpy(&tls->header, header, sizeof(CBTF_DataHeader));

    /* Initialize the actual data blob */
    initialize_data(tls);

    /* Initialize the IO function wrapper nesting depth */
    tls->nesting_depth = 0;
 
    /* Begin sampling */
    tls->header.time_begin = CBTF_GetTime();
    tls->do_trace = TRUE;
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

    tls->defer_sampling=TRUE;
    tls->do_trace = FALSE;
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

    tls->defer_sampling=FALSE;
    tls->do_trace = TRUE;
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

    if (tls == NULL) {
	//fprintf(stderr,"cbtf_collector_stop called without TLS!\n");
	return;
    }

    tls->header.time_end = CBTF_GetTime();

    /* Stop sampling */
    defer_trace(0);

    /* Are there any unsent samples? */
    if(tls->data.events.events_len > 0 || tls->data.stacktraces.stacktraces_len > 0) {
	send_samples(tls);
    }

    /* Destroy our thread-local storage */
#ifdef CBTF_SERVICE_USE_EXPLICIT_TLS
    free(tls);
    CBTF_SetTLS(TLSKey, NULL);
#endif
}

bool_t pthreads_do_trace(const char* traced_func)
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    if (tls == NULL) {
	//fprintf(stderr,"pthreads_do_trace called without TLS!\n");
	return FALSE;
    }

#if defined (CBTF_SERVICE_USE_OFFLINE)

    int saved_do_trace = tls->do_trace;
    if (tls->do_trace == 0) {
	if (tls->nesting_depth > 1)
	    --tls->nesting_depth;
	return FALSE;
    }

    //fprintf(stderr,"TRACING %s\n",traced_func);

    /* See if this function has been selected for tracing */
    char *tfptr, *saveptr, *tf_token;

    tls->do_trace = 0;
    tfptr = strdup(tls->CBTF_pthreads_traced);

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

    /* Remove any nesting due to skipping pthreads_start_event/pthreads_record_event for
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
