////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006-2014 The Krell Institute. All Rights Reserved.
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

/** @file
 *
 * Declaration of the StacktraceData class.
 *
 */

#ifndef _StacktraceData_
#define _StacktraceData_
 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "KrellInstitute/Core/AddressBuffer.hpp"
#include "KrellInstitute/Core/Graph.hpp"


namespace KrellInstitute { namespace Core {

    /**
     * StacktraceData.
     *
     */
    class StacktraceData
    {
	
    public:
	
	StacktraceData();    

	void aggregateAddressCounts(const unsigned &, const uint64_t*,
				    const uint8_t*, AddressBuffer&) const;
	void aggregateAddressCounts(const unsigned &, const uint64_t*,
				    AddressBuffer&) const;
	void aggregateAddressCounts(AddressCounts&,
				    AddressBuffer&) const;
	void graphAddressCounts(const unsigned &, const uint64_t*,
				    const uint8_t*, Graph&) const;
	void graphAddressCounts(const unsigned &, const uint64_t*,
				    Graph&) const;
    };
    
} }



#endif
