////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 William Hachfeld. All Rights Reserved.
// Copyright (c) 2006-2014 The Krell Institute All Rights Reserved.
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
 * Declaration of the ThreadName class.
 *
 */

#ifndef _KrellInstitute_Core_ThreadName_
#define _KrellInstitute_Core_ThreadName_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <stdint.h>

#include "KrellInstitute/Core/Path.hpp"
#include "KrellInstitute/Messages/Thread.h"


namespace KrellInstitute { namespace Core {

    class Path;

    /**
     * Repeatable thread name.
     *
     * Container holding those attributes of a thread that are repeatable across
     * multiple instantiations of the thread. Also defines when two such names
     * are to be considered equal. When reruning an experiment this allows the
     * data collectors applied to the new threads to mimic those applied to the
     * original threads.
     *
     * @ingroup Implementation
     */
    class ThreadName
    {

    public:

	ThreadName();
	ThreadName(const CBTF_Protocol_ThreadName&);

	ThreadName(const std::string&, const std::string&,
		   const int64_t&, const int64_t&,
		   const int32_t&, const Path&);
	ThreadName(const std::string&, const std::string&,
		   const int64_t&, const int64_t&,
		   const int32_t&, const int32_t&, const Path&);
	ThreadName(const std::string&,
		   const int64_t&, const int64_t&,
		   const int32_t&, const int32_t&);
	ThreadName(const std::string&,
		   const int64_t&, const int64_t&,
		   const int32_t&);

	bool operator==(const ThreadName&) const;

        /** Type conversion to a CBTF_Protocol_ThreadName object. */
        operator CBTF_Protocol_ThreadName() const
        {
            CBTF_Protocol_ThreadName object;
            //convert(dm_host, object.host);
            object.host = strdup(dm_host.c_str());
            object.pid = dm_pid;
            object.has_posix_tid = dm_posixtid.first;
            object.posix_tid = dm_posixtid.second;
            object.rank = dm_rank;
            object.omp_tid = dm_omptid;
            return object;
        }


	/** Operator "<" defined for two ThreadName objects. */
	bool operator<(const ThreadName& other) const
	{
	    if(dm_host < other.dm_host)
		return true;
	    if(dm_host > other.dm_host)
		return false;
	    if(!dm_posixtid.first && !other.dm_posixtid.first) {
		return dm_pid < other.dm_pid;
	    }
	    else {
		if(dm_pid < other.dm_pid)
		    return true;
		if(dm_pid > other.dm_pid)
		    return false;
		return (dm_posixtid.first && other.dm_posixtid.first) ?
		    (dm_posixtid.second < other.dm_posixtid.second) : !dm_posixtid.first;
	    }
	}

	/** Read-only data member accessor function. */
	const std::pair<bool, std::string>& getCommand() const
	{
	    return dm_command;
	}

	/** Read-only data member accessor function. */
	const std::string& getHost() const
	{
	    return dm_host;
	}

	/** Read-only data member accessor function. */
	const pid_t& getPid() const
	{
	    return dm_pid;
	}

	/** Read-only data member accessor function. */
	const std::pair<bool, int64_t>& getPosixThreadId() const
	{
	    return dm_posixtid;
	}

	/** Read-only data member accessor function. */
	const int32_t& getMPIRank() const
	{
	    return dm_rank;
	}

	/** Read-only data member accessor function. */
	const int32_t& getOmpTid() const
	{
	    return dm_omptid;
	}

	/** Read-only data member accessor function. */
	const std::pair<bool, Path>& getExecutable() const
	{
	    return dm_executable;
	}

        /** Operator "<<" defined for std::ostream. */
        friend std::ostream& operator<<(std::ostream& stream,
                                        const ThreadName& object)
        {
            stream << object.dm_host << ":" << object.dm_pid;
	    stream << ":" << object.dm_rank;
	    stream << ":" << object.dm_posixtid.second;
	    stream << ":" << object.dm_omptid;
            return stream;
        }

    private:

	/** Command used to create this thread. */
	std::pair<bool, std::string> dm_command;

	/** Name of the host on which this thread is located. */
	std::string dm_host;

	/** Process Id identifier of this thread. */
	pid_t dm_pid;

	/** Posix thread Id identifier of this thread. */
	std::pair<bool, int64_t> dm_posixtid;

	/** MPI rank of this thread. */
	int32_t dm_rank;

	/** OpenMP id of this thread. */
	int32_t dm_omptid;

	/** Path of this thread's executable. */
	std::pair<bool, Path> dm_executable;

    };
    
    typedef std::vector<ThreadName> ThreadNameVec;
} }



#endif
