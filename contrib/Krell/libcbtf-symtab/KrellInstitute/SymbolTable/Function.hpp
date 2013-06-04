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

/** @file Declaration of the Function class. */

#pragma once

#include <boost/operators.hpp>
#include <KrellInstitute/SymbolTable/AddressRange.hpp>
#include <set>
#include <string>

namespace KrellInstitute { namespace SymbolTable {

    class LinkedObject;
    class Statement;

    namespace Impl {
        class FunctionImpl;
    }

    /**
     * A source code function within a linked object.
     */
    class Function :
        public boost::totally_ordered<Function>
    {

    public:

        /**
         * Construct a function within the given linked object from its mangled
         * name. The constructed function initially has no address ranges.
         *
         * @param linked_object    Linked object containing this function.
         * @param name             Mangled name of this function.
         */
        Function(const LinkedObject& linked_object, const std::string& name);
        
        /**
         * Construct a function from an existing function.
         *
         * @param other    Function to be copied.
         */
        Function(const Function& other);
        
        /** Destructor. */
        virtual ~Function();
        
        /**
         * Replace this function with a copy of another one.
         *
         * @param other    Function to be copied.
         * @return         Resulting (this) function.
         */
        Function& operator=(const Function& other);

        /**
         * Is this function less than another one?
         *
         * @param other    Function to be compared.
         * @return         Boolean "true" if this function is less than the
         *                 function to be compared, or "false" otherwise.
         */
        bool operator<(const Function& other) const;

        /**
         * Is this function equal to another one?
         *
         * @param other    Function to be compared.
         * @return         Boolean "true" if the functions are equal,
         *                 or "false" otherwise.
         */
        bool operator==(const Function& other) const;

        /**
         * Get the linked object containing this function.
         *
         * @return    Linked object containing this function.
         */
        LinkedObject getLinkedObject() const;
        
        /**
         * Get the mangled name of this function.
         *
         * @return    Mangled name of this function.
         */
        std::string getMangledName() const;
        
        /**
         * Get the demangled name of this function. An optional boolean flag
         * is used to specify if all available information (including const,
         * volatile, function arguments, etc.) should be included in the
         * demangled name or not.
         *
         * @param all    Boolean "true" if all available information should be
         *               included in the demangled name, or "false" otherwise.
         * @return       Demangled name of this function.
         */
        std::string getDemangledName(const bool& all = true) const;

        /**
         * Get the address ranges associated with this function. An empty set
         * is returned if no address ranges are associated with this function.
         *
         * @return    Address ranges associated with this function.
         *
         * @note    The addresses specified are relative to the beginning of
         *          the linked object containing this function rather than
         *          an absolute address from the address space of a specific
         *          process.
         */
        std::set<AddressRange> getAddressRanges() const;

        /**
         * Get the definitions of this function. An empty set is returned if
         * no definitions of this function are found.
         *
         * @return    Definitions of this function.
         */
        std::set<Statement> getDefinitions() const;

        /**
         * Get the statements associated with this function. An empty set
         * is returned if no statements are associated with this function.
         *
         * @return    Statements associated with this function.
         */
        std::set<Statement> getStatements() const;

        /**
         * Associate the given address ranges with this function. 
         *
         * @param ranges    Address ranges to associate with this function.
         *
         * @note    The addresses specified are relative to the beginning of
         *          the linked object containing this function rather than
         *          an absolute address from the address space of a specific
         *          process.
         */
        void addAddressRanges(const std::set<AddressRange>& ranges);

    private:

        /**
         * Opaque pointer to this object's internal implementation details.
         * Provides information hiding, improves binary compatibility, and
         * reduces compile times.
         *
         * @sa http://en.wikipedia.org/wiki/Opaque_pointer
         */
        Impl::FunctionImpl* dm_impl;

    public:

        /**
         * Construct a function from its implementation details.
         *
         * @param impl    Opaque pointer to this function's
         *                internal implementation details.
         *
         * @note    This is a public method but not really part of the public
         *          interface. It exists because the implementation sometimes
         *          needs it. There is minimal potential for abuse since only
         *          the implementation has access to the implementation class
         *          and anyone who circumvents this via casting will get what
         *          they deserve.
         */
        Function(Impl::FunctionImpl* impl);
        
    }; // class Function
        
} } // namespace KrellInstitute::SymbolTable