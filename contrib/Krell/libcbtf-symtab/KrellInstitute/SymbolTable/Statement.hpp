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

/** @file Declaration of the Statement class. */

#pragma once

#include <boost/filesystem.hpp>
#include <boost/operators.hpp>
#include <KrellInstitute/SymbolTable/AddressRange.hpp>
#include <set>

namespace KrellInstitute { namespace SymbolTable {

    class Function;
    class LinkedObject;

    namespace Impl {
        class StatementImpl;
    }

    /**
     * A source code statement from a symbol table.
     */
    class Statement :
        public boost::totally_ordered<Statement>
    {

    public:

        /**
         * Construct a statement within the given linked object from its source
         * file, line, and column numbers. The constructed statement initially
         * has no address ranges.
         *
         * @param linked_object    Linked object containing this statement.
         * @param path             Full path name of this statement's source
         *                         file.
         * @param line             Line number of this statement.
         * @param column           Column number of this statement.
         */
        Statement(const LinkedObject& linked_object,
                  const boost::filesystem::path& path,
                  const unsigned int& line,
                  const unsigned int& column);

        /**
         * Construct a statement from an existing statement.
         *
         * @param other    Statement to be copied.
         */
        Statement(const Statement& other);
       
        /** Destructor. */
        virtual ~Statement();
        
        /**
         * Replace this statement with a copy of another one.
         *
         * @param other    Statement to be copied.
         * @return         Resulting (this) statement.
         */
        Statement& operator=(const Statement& other);

        /**
         * Is this statement less than another one?
         *
         * @param other    Statement to be compared.
         * @return         Boolean "true" if this statement is less than the
         *                 statement to be compared, or "false" otherwise.
         */
        bool operator<(const Statement& other) const;

        /**
         * Is this statement equal to another one?
         *
         * @param other    Statement to be compared.
         * @return         Boolean "true" if the statements are equal,
         *                 or "false" otherwise.
         */
        bool operator==(const Statement& other) const;

        /**
         * Get the linked object containing this statement.
         *
         * @return    Linked object containing this statement.
         */
        LinkedObject getLinkedObject() const;
        
        /**
         * Get the full path name of this statement's source file.
         *
         * @return    Full path name of this statement's source file.
         */
        boost::filesystem::path getPath() const;

        /**
         * Get the line number of this statement.
         *
         * @return    Line number of this statement.
         */
        unsigned int getLine() const;

        /**
         * Get the column number of this statement.
         *
         * @return    Column number of this statement.
         */
        unsigned int getColumn() const;

        /**
         * Get the address ranges associated with this statement. An empty set
         * is returned if no address ranges are associated with this statement.
         *
         * @return    Address ranges associated with this statement.
         *
         * @note    The addresses specified are relative to the beginning of
         *          the linked object containing this statement rather than
         *          an absolute address from the address space of a specific
         *          process.
         */
        std::set<AddressRange> getAddressRanges() const;

        /**
         * Get the functions containing this statement. An empty set is
         * returned if no function contains this statement.
         *
         * @return    Functions containing this statement.
         */
        std::set<Function> getFunctions() const;

        /**
         * Associate the given address ranges with this statement.
         *
         * @param ranges    Address ranges to associate with this statement.
         *
         * @note    The addresses specified are relative to the beginning of
         *          the linked object containing this statement rather than
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
        Impl::StatementImpl* dm_impl;

    public:

        /**
         * Construct a statement from its implementation details.
         *
         * @param impl    Opaque pointer to this statement's
         *                internal implementation details.
         *
         * @note    This is a public method but not really part of the public
         *          interface. It exists because the implementation sometimes
         *          needs it. There is minimal potential for abuse since only
         *          the implementation has access to the implementation class
         *          and anyone who circumvents this via casting will get what
         *          they deserve.
         */
        Statement(Impl::StatementImpl* impl);
        
    }; // class Statement
    
} } // namespace KrellInstitute::SymbolTable