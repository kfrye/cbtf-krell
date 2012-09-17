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
 * Declaration and definition of the HWC sampling collector's runtime.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "KrellInstitute/Messages/DataHeader.h"
#include "KrellInstitute/Messages/Hwcsamp.h"
#include "KrellInstitute/Messages/Hwcsamp_data.h"
#include "KrellInstitute/Messages/ToolMessageTags.h"
#include "KrellInstitute/Messages/Thread.h"
#include "KrellInstitute/Messages/ThreadEvents.h"
#include "KrellInstitute/Services/Assert.h"
#include "KrellInstitute/Services/Collector.h"
#include "KrellInstitute/Services/Common.h"
#include "KrellInstitute/Services/Context.h"
#include "KrellInstitute/Services/Data.h"
#include "KrellInstitute/Services/PapiAPI.h"
#include "KrellInstitute/Services/Time.h"
#include "KrellInstitute/Services/Timer.h"
#include "KrellInstitute/Services/TLS.h"

/** String uniquely identifying this collector. */
const char* const cbtf_collector_unique_id = "hwcsamp";


/** Type defining the items stored in thread-local storage. */
typedef struct {

    CBTF_DataHeader header;  /**< Header for following data blob. */
    CBTF_hwcsamp_data data;        /**< Actual data blob. */

    CBTF_HWCPCData buffer;      /**< PC sampling data buffer. */

    bool_t defer_sampling;
    int EventSet;
} TLS;

static int hwcsamp_papi_init_done = 0;
static long_long evalues[6] = { 0, 0, 0, 0, 0, 0 };

#if defined(USE_EXPLICIT_TLS)

/**
 * Key used to look up our thread-local storage. This key <em>must</em> be
 * unique from any other key used by any of the CBTF services.
 */
static const uint32_t TLSKey = 0x00001FF3;

#else

/** Thread-local storage. */
static __thread TLS the_tls;

#endif


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

    tls->header.time_begin = CBTF_GetTime();
    tls->header.time_end = 0;
    tls->header.addr_begin = ~0;
    tls->header.addr_end = 0;
    
    /* Initialize the actual data blob */
    tls->data.pc.pc_val = tls->buffer.pc;
    tls->data.count.count_val = tls->buffer.count;

    /* Re-initialize the actual data blob */
    tls->data.pc.pc_len = 0;
    tls->data.count.count_len = 0;
    tls->data.events.events_val = tls->buffer.hwccounts;
    tls->data.events.events_len = tls->buffer.length;

    /* Re-initialize the sampling buffer */
    tls->buffer.addr_begin = ~0;
    tls->buffer.addr_end = 0;
    tls->buffer.length = 0;
    memset(tls->buffer.hash_table, 0, sizeof(tls->buffer.hash_table));
    memset(tls->buffer.hwccounts, 0, sizeof(tls->buffer.hwccounts));
    memset(evalues,0,sizeof(evalues));
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

static void send_samples (TLS* tls)
{
    Assert(tls != NULL);

    tls->header.id = strdup(cbtf_collector_unique_id);
    tls->header.time_end = CBTF_GetTime();
    tls->header.addr_begin = tls->buffer.addr_begin;
    tls->header.addr_end = tls->buffer.addr_end;
    tls->data.pc.pc_len = tls->buffer.length;
    tls->data.count.count_len = tls->buffer.length;
    tls->data.events.events_len = tls->buffer.length;

#ifndef NDEBUG
    if (getenv("CBTF_DEBUG_COLLECTOR") != NULL) {

#if 0
int bufsize = tls->buffer.length * sizeof(tls->buffer);
int pcsize =  tls->buffer.length * sizeof(tls->data.pc.pc_val);
int countsize =  tls->buffer.length * sizeof(tls->data.count.count_val);

fprintf(stderr,"send_samples: size of tls data is %d, buffer is %d\n",sizeof(tls->data), sizeof(tls->buffer));
fprintf(stderr,"send_samples: size of tls PC data is %d  %d, COUNT is %d  %d\n",tls->buffer.length , pcsize,tls->buffer.length , countsize);
fprintf(stderr,"send_samples: size of tls HASH is %d\n", sizeof(tls->buffer.hash_table));
fprintf(stderr,"send_samples: size of tls data pc buff is %d\n", sizeof(tls->data.pc.pc_val)*CBTF_HWCPCBufferSize);
fprintf(stderr,"send_samples: size of tls data count buff is %d\n", sizeof(tls->data.count.count_val)*CBTF_HWCPCBufferSize);
fprintf(stderr,"send_samples: size of tls hwccounts is %d\n", sizeof(tls->buffer.hwccounts));
fprintf(stderr,"send_samples: size of CBTF_evcounts is %d\n", sizeof(CBTF_evcounts)*CBTF_HWCPCBufferSize);
fprintf(stderr,"send_samples: size of lon long is %d\n", sizeof(long long));
fprintf(stderr,"send_samples: size of uint64_t is %d\n", sizeof(uint64_t));
fprintf(stderr,"send_samples: size of CBTF_HWCPCData is %d\n", sizeof(CBTF_HWCPCData));
int eventssize =  tls->buffer.length * sizeof(tls->data.events.events_val);
fprintf(stderr,"send_samples: size of eventssize = %d\n",eventssize);
#endif


	    fprintf(stderr, "hwcsamp send_samples:\n");
	    fprintf(stderr, "time_range(%#lu,%#lu) addr range[%#lx, %#lx] pc_len(%d) count_len(%d)\n",
		tls->header.time_begin,tls->header.time_end,
		tls->header.addr_begin,tls->header.addr_end,
		tls->data.pc.pc_len,
		tls->data.count.count_len);
	}
#endif

    cbtf_collector_send(&(tls->header), (xdrproc_t)xdr_CBTF_hwcsamp_data, &(tls->data));

    /* Re-initialize the data blob's header */
    initialize_data(tls);
}


/**
 * Timer event handler.
 *
 * Called by the timer handler each time a sample is to be taken. Extracts the
 * program counter (PC) address from the signal context and places it into the
 * sample buffer. When the sample buffer is full, it is sent to the framework
 * for storage in the experiment's database.
 *
 * @note    
 * 
 * @param context    Thread context at timer interrupt.
 */
static void hwcsampTimerHandler(const ucontext_t* context)
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    if(tls->defer_sampling == TRUE) {
        return;
    }
 
    /* Obtain the program counter (PC) address from the thread context */
    uint64_t pc = CBTF_GetPCFromContext(context);

    /* This is supposed to reset counters */
    CBTF_HWCAccum(tls->EventSet, evalues);

    /* Update the sampling buffer and check if it has been filled */
    if(CBTF_UpdateHWCPCData(pc, &tls->buffer,evalues)) {
	/* Send these samples */
	send_samples(tls);
    }

    /* reset our values */
    memset(evalues,0,sizeof(evalues));

#ifndef NDEBUG
    if (getenv("CBTF_DEBUG_COLLECTOR_DETAILS") != NULL) {
      int i;
      for (i = 0; i < 6; i++) {
        if (tls->buffer.hwccounts[tls->buffer.length-1][i] > 0) {
            fprintf(stderr,"%#lx HWC sampTimerHandler %d count %d is %ld\n",pc,tls->buffer.length-1,i, tls->buffer.hwccounts[tls->buffer.length-1][i]);
        }
      }
    }
#endif
}


/**
 * Called by the CBTF collector service in order to start data collection.
 */
void cbtf_collector_start(const CBTF_DataHeader* const header)
{
/**
 * Start sampling.
 *
 * Starts hardware counter (HWC) sampling for the thread executing this
 * function. Initializes the appropriate thread-local data structures and
 * then enables the sampling counter.
 *
 * @param arguments    Encoded function arguments.
 */
    /* Create and access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
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
    CBTF_hwcsamp_start_sampling_args args;
    memset(&args, 0, sizeof(args));
    args.sampling_rate = 100;

    /* First set defaults */
    int hwcsamp_rate = 100;
    char* hwcsamp_papi_event = "PAPI_TOT_CYC,PAPI_FP_OPS";

#if defined (CBTF_SERVICE_USE_OFFLINE)
    char* hwcsamp_event_param = getenv("CBTF_HWCSAMP_EVENTS");
    if (hwcsamp_event_param != NULL) {
        hwcsamp_papi_event=hwcsamp_event_param;
    }

    const char* sampling_rate = getenv("CBTF_HWCSAMP_RATE");
    if (sampling_rate != NULL) {
        hwcsamp_rate=atoi(sampling_rate);
    }
    args.collector = 1;
    args.experiment = 0;
    tls->data.interval = (uint64_t)(1000000000) / (uint64_t)(hwcsamp_rate);;
#endif

#if defined(CBTF_SERVICE_USE_FILEIO)
    CBTF_SetSendToFile(cbtf_collector_unique_id, "cbtf-data");
#endif

    /* Initialize the actual data blob */
    memcpy(&tls->header, header, sizeof(CBTF_DataHeader));
    initialize_data(tls);
    tls->header.time_begin = CBTF_GetTime();

    if(hwcsamp_papi_init_done == 0) {
	CBTF_init_papi();
	tls->EventSet = PAPI_NULL;
	tls->data.clock_mhz = (float) hw_info->mhz;
	hwcsamp_papi_init_done = 1;
    } else {
	tls->data.clock_mhz = (float) hw_info->mhz;
    }


    /* PAPI SETUP */
    CBTF_Create_Eventset(&tls->EventSet);

    int rval = PAPI_OK;

#ifndef NDEBUG
    if (getenv("CBTF_DEBUG_COLLECTOR") != NULL) {
       fprintf(stderr, "PAPI Version: %d.%d.%d.%d\n", PAPI_VERSION_MAJOR( PAPI_VERSION ),
                        PAPI_VERSION_MINOR( PAPI_VERSION ),
                        PAPI_VERSION_REVISION( PAPI_VERSION ),
                        PAPI_VERSION_INCREMENT( PAPI_VERSION ) );
       fprintf(stderr,"System has %d hardware counters.\n", PAPI_num_counters());
    }
#endif

/* In Component PAPI, EventSets must be assigned a component index
 * before you can fiddle with their internals. 0 is always the cpu component */
#if (PAPI_VERSION_MAJOR(PAPI_VERSION)>=4)
    rval = PAPI_assign_eventset_component( tls->EventSet, 0 );
    if (rval != PAPI_OK) {
        CBTF_PAPIerror(rval,"CBTF_Create_Eventset assign_eventset_component");
        return;
    }
#endif

    if (getenv("CBTF_HWCSAMP_MULTIPLEX") != NULL) {
#if !defined(TARGET_OS_BGP) 
	rval = PAPI_set_multiplex( tls->EventSet );
	if ( rval == PAPI_ENOSUPP) {
	    fprintf(stderr,"CBTF_Create_Eventset: Multiplex not supported\n");
	} else if (rval != PAPI_OK)  {
	    CBTF_PAPIerror(rval,"CBTF_Create_Eventset set_multiplex");
	}
#endif
    }

    /* TODO: check return values of direct PAPI calls
     * and handle them as needed.
     */
    /* Rework the code here to call PAPI directly rather than
     * call any OPENSS helper functions due to inconsitent
     * behaviour seen on various lab systems
     */
    int eventcode = 0;
    rval = PAPI_OK;
    if (hwcsamp_papi_event != NULL) {
	char *tfptr, *saveptr, *tf_token;
	tfptr = strdup(hwcsamp_papi_event);
	int i;
	for (i = 1;  ; i++, tfptr = NULL) {
	    tf_token = strtok_r(tfptr, ",", &saveptr);
	    if (tf_token == NULL) {
		break;
	    }
	    PAPI_event_name_to_code(tf_token,&eventcode);
	    rval = PAPI_add_event(tls->EventSet,eventcode);

	    if (tfptr) free(tfptr);
	}
	
    } else {
	PAPI_event_name_to_code("PAPI_TOT_CYC",&eventcode);
	rval = PAPI_add_event(tls->EventSet,eventcode);
	PAPI_event_name_to_code("PAPI_FP_OPS",&eventcode);
	rval = PAPI_add_event(tls->EventSet,eventcode);
    }

    /* Begin sampling */
    tls->header.time_begin = CBTF_GetTime();
    CBTF_Start(tls->EventSet);
    CBTF_Timer(tls->data.interval, hwcsampTimerHandler);
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
    if (hwcsamp_papi_init_done == 0 || tls == NULL)
	return;

    tls->defer_sampling=TRUE;
    CBTF_Stop(tls->EventSet, evalues);
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
    if (hwcsamp_papi_init_done == 0 || tls == NULL)
	return;

    tls->defer_sampling=FALSE;
    CBTF_Start(tls->EventSet);
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
 * Called by the CBTF collector service in order to stop data collection.
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

    if (tls->EventSet == PAPI_NULL) {
	/*fprintf(stderr,"hwcsamp_stop_sampling RETURNS - NO EVENTSET!\n");*/
	/* we are called before eny events are set in papi. just return */
        return;
    }

    /* Stop counters */
    CBTF_Stop(tls->EventSet, evalues);

    /* Stop sampling */
    CBTF_Timer(0, NULL);

    tls->header.time_end = CBTF_GetTime();

    /* Are there any unsent samples? */
    if(tls->buffer.length > 0) {
	/* Send these samples */
	send_samples(tls);
    }

    /* Destroy our thread-local storage */
#ifdef CBTF_SERVICE_USE_EXPLICIT_TLS
    destroy_explicit_tls();
#endif
}



#if defined (CBTF_SERVICE_USE_OFFLINE)
void cbtf_offline_service_start_timer()
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    if (hwcsamp_papi_init_done == 0 || tls == NULL)
	return;
    CBTF_Start(tls->EventSet);
    CBTF_Timer(tls->data.interval, hwcsampTimerHandler);
}

void cbtf_offline_service_stop_timer()
{
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    CBTF_Stop(tls->EventSet, evalues);
    CBTF_Timer(0, NULL);
}
#endif