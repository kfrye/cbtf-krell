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

/**
 * @file
 *
 * Callback functions invoked by cupti_callback() every time the CUDA function:
 *
 *     cuModuleGetFunction
 *
 * is entered or exited. This is not a traditional header file in that it is not
 * intended for use <em>anywhere</em> except in "collector.h". It exists purely
 * to help manage the size and repetiveness of these callback functions.
 */



/**
 * Entry callback for cuModuleGetFunction().
 */
inline void cuModuleGetFunction_enter_callback(
    const cuModuleGetFunction_params* const params
    )
{
#if !defined(NDEBUG)
    if (debug)
    {
        printf("[CBTF/CUDA] cuModuleGetFunction_enter_callback()\n");
    }
#endif

    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    /* ... disable any sampling and send data ... */
}



/**
 * Exit callback for cuModuleGetFunction().
 */
inline void cuModuleGetFunction_exit_callback(
    const cuModuleGetFunction_params* const params
    )
{
#if !defined(NDEBUG)
    if (debug)
    {
        printf("[CBTF/CUDA] cuModuleGetFunction_exit_callback()\n");
    }
#endif

    /* Access our thread-local storage */
#ifdef USE_EXPLICIT_TLS
    TLS* tls = CBTF_GetTLS(TLSKey);
#else
    TLS* tls = &the_tls;
#endif
    Assert(tls != NULL);

    /* Send a message for this event */
    
    CBTF_cuda_data data;
    data.type = ResolvedFunction;
                        
    CUDA_ResolvedFunction* message = &data.CBTF_cuda_data_u.resolved_function;
                        
    message->time = CBTF_GetTime();
    message->module_handle = (CBTF_Protocol_Address)params->hmod;
    message->function = (char*)params->name;
    message->handle = (CBTF_Protocol_Address)*(params->hfunc);

    tls->data_header.time_begin = 0;
    tls->data_header.time_end = 0;
    tls->data_header.addr_begin = 0;
    tls->data_header.addr_end = 0;
    
    cbtf_collector_send(
        &tls->data_header, (xdrproc_t)xdr_CBTF_cuda_data, &data
        );
                        
    /* ... re-enable any sampling ... */
}
