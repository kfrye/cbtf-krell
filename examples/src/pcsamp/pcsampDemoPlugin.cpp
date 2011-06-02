////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2011 Krell Institute. All Rights Reserved.
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

/** @file Plugin used by unit tests for the CBTF MRNet library. */

#include <boost/bind.hpp>
#include <mrnet/MRNet.h>
#include <typeinfo>

#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>

#include "KrellInstitute/Core/Address.hpp"
#include "KrellInstitute/Core/AddressBuffer.hpp"
#include "KrellInstitute/Core/Blob.hpp"
#include "KrellInstitute/Core/PCData.hpp"

#include "KrellInstitute/Messages/Address.h"
#include "KrellInstitute/Messages/DataHeader.h"
#include "KrellInstitute/Messages/EventHeader.h"
#include "KrellInstitute/Messages/File.h"
#include "KrellInstitute/Messages/LinkedObjectEvents.h"
#include "KrellInstitute/Messages/PCSamp_data.h"
#include "KrellInstitute/Messages/Thread.h"
#include "KrellInstitute/Messages/ThreadEvents.h"


using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::Core;

/**
 * Component that aggregates PC address values and their counts.
 */
class __attribute__ ((visibility ("hidden"))) AddressAggregator :
    public Component
{

public:

    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
            reinterpret_cast<Component*>(new AddressAggregator())
            );
    }

private:

    /** Default constructor. */
    AddressAggregator() :
        Component(Type(typeid(AddressAggregator)), Version(0, 0, 1))
    {
        declareInput<CBTF_pcsamp_data>(
            "in1", boost::bind(&AddressAggregator::pcsampHandler, this, _1)
            );
        declareInput<AddressBuffer>(
            "in2", boost::bind(&AddressAggregator::addressBufferHandler, this, _1)
            );
        declareInput<Blob>(
            "in3", boost::bind(&AddressAggregator::blobHandler, this, _1)
            );
        declareInput<CBTF_Protocol_Blob>(
            "in4", boost::bind(&AddressAggregator::cbtf_protocol_blob_Handler, this, _1)
            );
        declareOutput<AddressBuffer>("Aggregatorout");
    }

    /** Handler for the "in1" input.*/
    void pcsampHandler(const CBTF_pcsamp_data& in)
    {
	PCData pcdata;
	pcdata.aggregateAddressCounts(in,abuffer);

        emitOutput<AddressBuffer>("Aggregatorout",  abuffer);
    }

    /** Handler for the "in4" input.*/
    void cbtf_protocol_blob_Handler(const CBTF_Protocol_Blob& in)
    {
	Blob myblob(in.data.data_len, in.data.data_val);

        CBTF_DataHeader header;
        memset(&header, 0, sizeof(header));
        unsigned header_size = myblob.getXDRDecoding(
            reinterpret_cast<xdrproc_t>(xdr_CBTF_DataHeader), &header
            );

        CBTF_pcsamp_data data;
        memset(&data, 0, sizeof(data));
        myblob.getXDRDecoding(reinterpret_cast<xdrproc_t>(xdr_CBTF_pcsamp_data), &data);

	PCData pcdata;
	pcdata.aggregateAddressCounts(data,abuffer);

        emitOutput<AddressBuffer>("Aggregatorout",  abuffer);
    }

    /** Handler for the "in2" input.*/
    void addressBufferHandler(const AddressBuffer& in)
    {
        emitOutput<AddressBuffer>("Aggregatorout",  abuffer);
    }

    /** Handler for the "in3" input.*/
    void blobHandler(const Blob& in)
    {

// this currently is hard coded to pcsamp data.
// the data header has an int for experiment type which in OSS
// is used to match the expId.  Maybe in cbtf this can be an
// enum to define each type of data type and then used to
// choose the appropro data xdrproc....
//
        CBTF_DataHeader header;
        memset(&header, 0, sizeof(header));
        unsigned header_size = in.getXDRDecoding(
            reinterpret_cast<xdrproc_t>(xdr_CBTF_DataHeader), &header
            );

        CBTF_pcsamp_data data;
        memset(&data, 0, sizeof(data));
        in.getXDRDecoding(reinterpret_cast<xdrproc_t>(xdr_CBTF_pcsamp_data), &data);

#if 0
        if (data.pc.pc_len != data.count.count_len) {
            std::cerr << "ASSERT blobHandler pc_len "
            << data.pc.pc_len
            << " != count_len "
            << data.count.count_len << std::endl;
        }

// this is available in the PCData class.
// void PCData::aggregateAddressCounts( const CBTF_pcsamp_data& data,
//                                 AddressBuffer& buffer)
//
	for(unsigned i = 0; i < data.pc.pc_len; ++i) {
	    abuffer.updateAddressCounts(data.pc.pc_val[i], data.count.count_val[i]);
	}
#else
	PCData pcdata;
	pcdata.aggregateAddressCounts(data,abuffer);
#endif
        emitOutput<AddressBuffer>("Aggregatorout",  abuffer);
    }

    AddressBuffer abuffer;
    
}; // class AddressAggregator

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(AddressAggregator)
