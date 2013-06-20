////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Krell Institute. All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

/** @file Declaration and definition of the Address class. */

#pragma once

#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <boost/format.hpp>
#include <boost/operators.hpp>
#include <iostream>
#include <KrellInstitute/Messages/Address.h>
#include <limits>

namespace KrellInstitute { namespace SymbolTable {
        
    /**
     * All memory addresses are stored in 64-bit unsigned integers, allowing
     * for a unified representation of both 32-bit and 64-bit addresses, but
     * sacrificing storage efficiency when 32-bit addresses are processed.
     */
    class Address :
        public boost::addable<Address, boost::int64_t>,
        public boost::totally_ordered<Address>,
        public boost::unit_steppable<Address>
    {
        
    public:

        /** Construct the lowest possible address. */
        static Address TheLowest()
        {
            return Address(std::numeric_limits<boost::uint64_t>::min());
        }
        
        /** Construct the highest possible address. */
        static Address TheHighest()
        {
            return Address(std::numeric_limits<boost::uint64_t>::max());
        }
        
        /** Default constructor. */
        Address() :
            dm_value(0x0)
        {
        }

        /** Construct an address from a CBTF_Protocol_Address (uint64_t). */
        Address(const CBTF_Protocol_Address& message) :
            dm_value(message)
        {
        }
        
        /** Type conversion to a CBTF_Protocol_Address (uint64_t). */
        operator CBTF_Protocol_Address() const
        {
            return CBTF_Protocol_Address(dm_value);
        }

        /** Is this address less than another one? */
        bool operator<(const Address& other) const
        {
            return dm_value < other.dm_value;
        }
        
        /** Is this address equal to another one? */
        bool operator==(const Address& other) const
        {
            return dm_value == other.dm_value;
        }

        /** Increment this address. */
        Address& operator++()
        {
            dm_value += 1;
            return *this;
        }

        /** Add a signed offset to this address. */
        Address& operator+=(const boost::int64_t& offset)
        {
            boost::uint64_t result = dm_value + offset;
            BOOST_ASSERT((offset > 0) || (result <= dm_value));
            BOOST_ASSERT((offset < 0) || (result >= dm_value));
            dm_value += offset;
            return *this;
        }

        /** Decrement this address. */
        Address& operator--()
        {
            dm_value -= 1;
            return *this;
        }

        /** Subtract another address from this address. */
        boost::int64_t operator-(const Address& other) const
        {
            boost::int64_t difference = dm_value - other.dm_value;
            BOOST_ASSERT((*this > other) || (difference <= 0));
            BOOST_ASSERT((*this < other) || (difference >= 0));
            return difference;
        }
        
        /** Redirection to an output stream. */
        friend std::ostream& operator<<(std::ostream& stream,
                                        const Address& address)
        {
            stream << boost::str(boost::format("0x%016X") % address.dm_value);
            return stream;
        }
        
    private:
        
        /** Value of this address. */
        boost::uint64_t dm_value;
        
    }; // class Address
        
} } // namespace KrellInstitute::SymbolTable
