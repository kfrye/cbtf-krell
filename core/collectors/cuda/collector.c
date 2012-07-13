/*******************************************************************************
** Copyright (c) 2012 Argo Navis Technologies. All Rights Reserved.
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

/** @file Implementation of the CUDA collector. */

#include <cupti.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "KrellInstitute/Services/Assert.h"
#include "KrellInstitute/Services/Collector.h"
#include "KrellInstitute/Services/Time.h"
#include "KrellInstitute/Services/TLS.h"

#include "CUDA_data.h"



/**
 * Checks that the given CUPTI function call returns the value "CUPTI_SUCCESS".
 * If the call was unsuccessful, the returned error is reported on the standard
 * error stream and the application is aborted.
 *
 * @param x    CUPTI function call to be checked.
 */
#define CUPTI_CHECK(x)                                              \
    do {                                                            \
        CUptiResult retval = x;                                     \
        if (retval != CUPTI_SUCCESS)                                \
        {                                                           \
            const char* description = NULL;                         \
            if (cuptiGetResultString(retval, &description) ==       \
                CUPTI_SUCCESS)                                      \
            {                                                       \
                fprintf(stderr, "[CBTF/CUDA] %s(): %s = %d (%s)\n", \
                        __func__, #x, retval, description);         \
            }                                                       \
            else                                                    \
            {                                                       \
                fprintf(stderr, "[CBTF/CUDA] %s(): %s = %d\n",      \
                        __func__, #x, retval);                      \
            }                                                       \
            fflush(stderr);                                         \
            abort();                                                \
        }                                                           \
    } while (0)



/**
 * Checks that the given pthread function call returns the value "0". If the
 * call was unsuccessful, the returned error is reported on the standard error
 * stream and the application is aborted.
 *
 * @param x    Pthread function call to be checked.
 */
#define PTHREAD_CHECK(x)                                   \
    do {                                                   \
        int retval = x;                                    \
        if (retval != 0)                                   \
        {                                                  \
            fprintf(stderr, "[CBTF/CUDA] %s(): %s = %d\n", \
                    __func__, #x, retval);                 \
            fflush(stderr);                                \
            abort();                                       \
        }                                                  \
    } while (0)



/** String uniquely identifying this collector. */
const char* const cbtf_collector_unique_id = "cuda";

/**
 * The number of threads for which are are collecting data (actively or not).
 * This value is atomically incremented in cbtf_collector_start(), decremented
 * in cbtf_collector_stop(), and is used by those functions to determine when
 * to perform process-wide initialization and finalization.
 */
static struct {
    int value;
    pthread_mutex_t mutex;
} thread_count = { 0, PTHREAD_MUTEX_INITIALIZER };

/** Boolean flag indicating if debugging is enabled. */
static int debug = 0;

/** CUPTI subscriber handle for this collector. */
static CUpti_SubscriberHandle cupti_subscriber_handle;



/** Type defining the data stored in thread-local storage. */
typedef struct {

    /**
     * Performance data header applied to all of this thread's data. The fields
     * time_begin, time_end, addr_begin, and addr_end are overwritten at will by
     * the various collection routines.
     */
    CBTF_DataHeader data_header;
    
    /* ... */
    
} TLS;

#if defined(USE_EXPLICIT_TLS)

/**
 * Key used to look up our thread-local storage. This key <em>must</em> be
 * unique from any other key used by any of the CBTF services.
 */
static const uint32_t TLSKey = 0xBAD0C0DA;

#else

/** Thread-local storage. */
static __thread TLS the_tls;

#endif



#include "cuLaunchKernel.h"
#include "cuModuleGetFunction.h"
#include "cuModuleLoad.h"
#include "cuModuleLoadData.h"
#include "cuModuleLoadDataEx.h"
#include "cuModuleLoadFatBinary.h"
#include "cuModuleUnload.h"



/**
 * Callback invoked by CUPTI every time a CUDA event occurs for which we have
 * a subscription. Subscriptions are setup within cbtf_collector_start(), and
 * are setup once for the entire process.
 *
 * @param userdata    User data supplied at subscription of the callback.
 * @param domain      Domain of the callback.
 * @param id          ID of the callback.
 * @param data        Data passed to the callback.
 */
static void cupti_callback(void* userdata,
                           CUpti_CallbackDomain domain,
                           CUpti_CallbackId id,
                           const void* data)
{
    /* Determine the CUDA event that has occurred and handle it */
    switch (domain)
    {

    case CUPTI_CB_DOMAIN_DRIVER_API:
        {
            const CUpti_CallbackData* const cbdata = (CUpti_CallbackData*)data;
            
            switch (id)
            {

            case CUPTI_DRIVER_TRACE_CBID_cuLaunchKernel :
                {
                    const cuLaunchKernel_params* const params =
                        (cuLaunchKernel_params*)cbdata->functionParams;

                    if (cbdata->callbackSite == CUPTI_API_ENTER)
                    {
                        cuLaunchKernel_enter_callback(params);
                    }
                    else if (cbdata->callbackSite == CUPTI_API_EXIT)
                    {
                        cuLaunchKernel_exit_callback(params);
                    }
                }
                break;

            case CUPTI_DRIVER_TRACE_CBID_cuModuleGetFunction :
                {
                    const cuModuleGetFunction_params* const params =
                        (cuModuleGetFunction_params*)cbdata->functionParams;
                    
                    if (cbdata->callbackSite == CUPTI_API_ENTER)
                    {
                        cuModuleGetFunction_enter_callback(params);
                    }
                    if (cbdata->callbackSite == CUPTI_API_EXIT)
                    {
                        cuModuleGetFunction_exit_callback(params);
                    }
                }
                break;

            case CUPTI_DRIVER_TRACE_CBID_cuModuleLoad :
                {
                    const cuModuleLoad_params* const params =
                        (cuModuleLoad_params*)cbdata->functionParams;

                    if (cbdata->callbackSite == CUPTI_API_ENTER)
                    {
                        cuModuleLoad_enter_callback(params);
                    }
                    if (cbdata->callbackSite == CUPTI_API_EXIT)
                    {
                        cuModuleLoad_exit_callback(params);
                    }
                }
                break;

            case CUPTI_DRIVER_TRACE_CBID_cuModuleLoadData :
                {
                    const cuModuleLoadData_params* const params =
                        (cuModuleLoadData_params*)cbdata->functionParams;

                    if (cbdata->callbackSite == CUPTI_API_ENTER)
                    {
                        cuModuleLoadData_enter_callback(params);
                    }
                    if (cbdata->callbackSite == CUPTI_API_EXIT)
                    {
                        cuModuleLoadData_exit_callback(params);
                    }
                }
                break;

            case CUPTI_DRIVER_TRACE_CBID_cuModuleLoadDataEx :
                {
                    const cuModuleLoadDataEx_params* const params =
                        (cuModuleLoadDataEx_params*)cbdata->functionParams;

                    if (cbdata->callbackSite == CUPTI_API_ENTER)
                    {
                        cuModuleLoadDataEx_enter_callback(params);
                    }
                    if (cbdata->callbackSite == CUPTI_API_EXIT)
                    {
                        cuModuleLoadDataEx_exit_callback(params);
                    }
                }
                break;

            case CUPTI_DRIVER_TRACE_CBID_cuModuleLoadFatBinary :
                {
                    const cuModuleLoadFatBinary_params* const params =
                        (cuModuleLoadFatBinary_params*)cbdata->functionParams;

                    if (cbdata->callbackSite == CUPTI_API_ENTER)
                    {
                        cuModuleLoadFatBinary_enter_callback(params);
                    }
                    if (cbdata->callbackSite == CUPTI_API_EXIT)
                    {
                        cuModuleLoadFatBinary_exit_callback(params);
                    }
                }
                break;
                
            case CUPTI_DRIVER_TRACE_CBID_cuModuleUnload :
                {
                    const cuModuleUnload_params* const params =
                        (cuModuleUnload_params*)cbdata->functionParams;
                    
                    if (cbdata->callbackSite == CUPTI_API_ENTER)
                    {
                        cuModuleUnload_enter_callback(params);
                    }
                    if (cbdata->callbackSite == CUPTI_API_EXIT)
                    {
                        cuModuleUnload_exit_callback(params);
                    }
                }
                break;

            }
        }
        break;

    default:
        break;        
    }
}



/**
 * Parse the configuration string that was passed into this collector.
 *
 * @param configuration    Configuration string passed into this collector.
 */
static void parse_configuration(const char* const configuration)
{
#if !defined(NDEBUG)
    if (debug)
    {
        printf("[CBTF/CUDA] parse_configuration(\"%s\")\n", configuration);
    }
#endif
    
    /* ... */            
}



/**
 * Called by the CBTF collector service in order to start data collection.
 */
void cbtf_collector_start(const CBTF_DataHeader* const header)
{
#if !defined(NDEBUG)
    if (debug)
    {
        printf("[CBTF/CUDA] cbtf_collector_start()\n");
    }
#endif
    
    /* Atomically increment the active thread count */

    PTHREAD_CHECK(pthread_mutex_lock(&thread_count.mutex));
    
#if !defined(NDEBUG)
    if (debug)
    {
        printf("[CBTF/CUDA] cbtf_collector_start(): "
               "thread_count.value = %d --> %d\n",
               thread_count.value, thread_count.value + 1);
    }
#endif
    
    if (thread_count.value == 0)
    {
        /* Should debugging be enabled? */
        debug = (getenv("CBTF_DEBUG_COLLECTOR") == NULL);
        
#if !defined(NDEBUG)
        if (debug)
        {
            printf("[CBTF/CUDA] cbtf_collector_start()\n");
        }
#endif

        /** Obtain our configuration string from the environment and parse it */
        const char* const configuration = getenv("CBTF_CUDA_CONFIG");
        if (configuration != NULL)
        {
            parse_configuration(configuration);
        }

        /* Subscribe to the CUPTI callbacks of interest */
        
        CUPTI_CHECK(cuptiSubscribe(&cupti_subscriber_handle,
                                   cupti_callback, NULL));

        CUPTI_CHECK(cuptiEnableCallback(
                        1, cupti_subscriber_handle,
                        CUPTI_CB_DOMAIN_DRIVER_API,
                        CUPTI_DRIVER_TRACE_CBID_cuLaunchKernel
                        ));

        CUPTI_CHECK(cuptiEnableCallback(
                        1, cupti_subscriber_handle,
                        CUPTI_CB_DOMAIN_DRIVER_API,
                        CUPTI_DRIVER_TRACE_CBID_cuModuleGetFunction
                        ));

        CUPTI_CHECK(cuptiEnableCallback(
                        1, cupti_subscriber_handle,
                        CUPTI_CB_DOMAIN_DRIVER_API,
                        CUPTI_DRIVER_TRACE_CBID_cuModuleLoad
                        ));

        CUPTI_CHECK(cuptiEnableCallback(
                        1, cupti_subscriber_handle,
                        CUPTI_CB_DOMAIN_DRIVER_API,
                        CUPTI_DRIVER_TRACE_CBID_cuModuleLoadData
                        ));

        CUPTI_CHECK(cuptiEnableCallback(
                        1, cupti_subscriber_handle,
                        CUPTI_CB_DOMAIN_DRIVER_API,
                        CUPTI_DRIVER_TRACE_CBID_cuModuleLoadDataEx
                        ));

        CUPTI_CHECK(cuptiEnableCallback(
                        1, cupti_subscriber_handle,
                        CUPTI_CB_DOMAIN_DRIVER_API,
                        CUPTI_DRIVER_TRACE_CBID_cuModuleLoadFatBinary
                        ));

        CUPTI_CHECK(cuptiEnableCallback(
                        1, cupti_subscriber_handle,
                        CUPTI_CB_DOMAIN_DRIVER_API,
                        CUPTI_DRIVER_TRACE_CBID_cuModuleUnload
                        ));
    }

    thread_count.value++;

    PTHREAD_CHECK(pthread_mutex_unlock(&thread_count.mutex));
    
    /* Create, zero-initialize, and access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = malloc(sizeof(TLS));
    Assert(tls != NULL);
    OpenSS_SetTLS(TLSKey, tls);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);
    memset(tls, 0, sizeof(TLS));

    /* Copy the header into our thread-local storage for future use */
    memcpy(&tls->data_header, header, sizeof(CBTF_DataHeader));
}



/**
 * Called by the CBTF collector service in order to pause data collection.
 */
void cbtf_collector_pause()
{
#if !defined(NDEBUG)
    if (debug)
    {
        printf("[CBTF/CUDA] cbtf_collector_pause()\n");
    }
#endif
                
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    /* ... */
}



/**
 * Called by the CBTF collector service in order to resume data collection.
 */
void cbtf_collector_resume()
{
#if !defined(NDEBUG)
    if (debug)
    {
        printf("[CBTF/CUDA] cbtf_collector_resume()\n");
    }
#endif

    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    /* ... */
}



/**
 * Called by the CBTF collector service in order to stop data collection.
 */
void cbtf_collector_stop()
{
#if !defined(NDEBUG)
    if (debug)
    {
        printf("[CBTF/CUDA] cbtf_collector_stop()\n");
    }
#endif
    
    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);
    
    /* ... */
    
    /* Destroy our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    free(tls);
    OpenSS_SetTLS(TLSKey, NULL);
#endif
    
    /* Atomically decrement the active thread count */
    
    PTHREAD_CHECK(pthread_mutex_lock(&thread_count.mutex));
    
#if !defined(NDEBUG)
    if (debug)
    {
        printf("[CBTF/CUDA] cbtf_collector_stop(): "
               "thread_count.value = %d --> %d\n",
               thread_count.value, thread_count.value - 1);
    }
#endif
    
    thread_count.value--;

    if (thread_count.value == 0)
    {
        /* Unsubscribe from all CUPTI callbacks */
        CUPTI_CHECK(cuptiUnsubscribe(cupti_subscriber_handle));
    }
    
    PTHREAD_CHECK(pthread_mutex_unlock(&thread_count.mutex));
}
