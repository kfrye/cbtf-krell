////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Argo Navis Technologies. All Rights Reserved.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

/** @file Component for converting CUDA data to pseudo I/O data. */

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <typeinfo>

#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <KrellInstitute/CBTF/XDR.hpp>

#include "Address.h"
#include "Blob.h"
#include "File.h"
#include "Thread.h"
#include "Time.h"

#include "CUDA_data.h"
#include "IO_data.h"
#include "LinkedObjectEvents.h"
#include "Symbol.h"

using namespace KrellInstitute::CBTF;



/**
 * Converter that translates the performance data blobs generated by the CUDA
 * collector into pseudo performance data blobs that appear as if they had been
 * generated by the I/O collector. This allows Open|SpeedShop's existing views
 * for the I/O collector to work directly with the performance data generated
 * by the CUDA collector.
 *
 * @note    Along the way, this converter must create pseudo address space
 *          mappings, linked objects, and functions so that the various CUDA
 *          kernel invocations, memory copies, and memory sets can all appear
 *          as if they were I/O functions.
 */
class __attribute__ ((visibility ("hidden"))) CUDAToIO :
    public Component
{
    
public:

    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
            reinterpret_cast<Component*>(new CUDAToIO())
            );
    }
    
private:

    /** Default constructor. */
    CUDAToIO() :
        Component(Type(typeid(CUDAToIO)), Version(0, 0, 0))
    {
        declareInput<boost::shared_ptr<CBTF_Protocol_LinkedObjectGroup> >(
            "InitialLinkedObjects",
            boost::bind(&CUDAToIO::handleInitialLinkedObjects, this, _1)
            );
        declareInput<boost::shared_ptr<CBTF_Protocol_LoadedLinkedObject> >(
            "LoadedLinkedObject",
            boost::bind(&CUDAToIO::handleLoadedLinkedObject, this, _1)
            );
        declareInput<boost::shared_ptr<CBTF_Protocol_UnloadedLinkedObject> >(
            "UnloadedLinkedObject",
            boost::bind(&CUDAToIO::handleUnloadedLinkedObject, this, _1)
            );
        declareInput<boost::shared_ptr<CBTF_cuda_data> >(
            "Data", boost::bind(&CUDAToIO::handleData, this, _1)
            );

        declareOutput<boost::shared_ptr<CBTF_Protocol_LinkedObjectGroup> >(
            "InitialLinkedObjects"
            );
        declareOutput<boost::shared_ptr<CBTF_Protocol_LoadedLinkedObject> >(
            "LoadedLinkedObject"
            );
        declareOutput<boost::shared_ptr<CBTF_Protocol_UnloadedLinkedObject> >(
            "UnloadedLinkedObject"
            );
        declareOutput<boost::shared_ptr<CBTF_Protocol_SymbolTable> >(
            "SymbolTable"
            );
        declareOutput<boost::shared_ptr<CBTF_io_trace_data> >("Data");
    }

    /** Handler for the "InitialLinkedObjects" input. */
    void handleInitialLinkedObjects(
        const boost::shared_ptr<CBTF_Protocol_LinkedObjectGroup>& message
        )
    {
        // ...
    }

    /** Handler for the "LoadedLinkedObject" input. */
    void handleLoadedLinkedObject(
        const boost::shared_ptr<CBTF_Protocol_LoadedLinkedObject>& message
        )
    {
        // ...
    }
    
    /** Handler for the "UnloadedLinkedObject" input. */
    void handleUnloadedLinkedObject(
        const boost::shared_ptr<CBTF_Protocol_UnloadedLinkedObject>& message
        )
    {
        // ...
    }

    /** Handler for the "Data" input. */
    void handleData(const boost::shared_ptr<CBTF_cuda_data>& message)
    {
        // ...
    }

}; // class CUDAToIO

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(CUDAToIO)

KRELL_INSTITUTE_CBTF_REGISTER_XDR_CONVERTERS(CBTF_Protocol_LinkedObjectGroup)
KRELL_INSTITUTE_CBTF_REGISTER_XDR_CONVERTERS(CBTF_Protocol_LoadedLinkedObject)
KRELL_INSTITUTE_CBTF_REGISTER_XDR_CONVERTERS(CBTF_Protocol_UnloadedLinkedObject)
KRELL_INSTITUTE_CBTF_REGISTER_XDR_CONVERTERS(CBTF_Protocol_SymbolTable)
KRELL_INSTITUTE_CBTF_REGISTER_XDR_CONVERTERS(CBTF_cuda_data)
KRELL_INSTITUTE_CBTF_REGISTER_XDR_CONVERTERS(CBTF_io_trace_data)