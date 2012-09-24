/*	libloadorder

	A library for reading and writing the load order of plugin files for
	TES III: Morrowind, TES IV: Oblivion, TES V: Skyrim, Fallout 3 and
	Fallout: New Vegas.

    Copyright (C) 2012    WrinklyNinja

	This file is part of libloadorder.

    libloadorder is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    libloadorder is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libloadorder.  If not, see 
	<http://www.gnu.org/licenses/>.
*/

#include "exception.h"
#include "libloadorder.h"
#include <boost/format.hpp>

namespace liblo {

	error::error() : errCode(0) {
		lastException = *this;
	}

	error::error(uint32_t code) : errCode(code) {
		lastException = *this;
	}

	error::error(uint32_t code, std::string subject) 
		: errCode(code), errSubject(subject) {
		lastException = *this;
	}

	error::error(uint32_t code, std::string subject, std::string detail) 
		: errCode(code), errSubject(subject), errDetail(detail) {
		lastException = *this;
	}

	std::string	error::what()			const {
		if (errCode == LIBLO_WARN_BAD_FILENAME)
			return (boost::format("\"%1%\" cannot be converted from UTF-8 to \"%2%\".") % errSubject % errDetail).str();
		else if (errCode == LIBLO_WARN_LO_MISMATCH)
			return "The order of plugins present in both loadorder.txt and plugins.txt differs between the two files.";
		else if (errCode == LIBLO_ERROR_FILE_READ_FAIL)
			return (boost::format("\"%1%\" cannot be read!") % errSubject).str(); 
		else if (errCode == LIBLO_ERROR_FILE_WRITE_FAIL)
			return (boost::format("\"%1%\" cannot be written to!") % errSubject).str();
		else if (errCode == LIBLO_ERROR_FILE_RENAME_FAIL)
			return (boost::format("\"%1%\" cannot be renamed! Filesystem response: \"%2%\".") % errSubject % errDetail).str();
		else if (errCode == LIBLO_ERROR_FILE_PARSE_FAIL)
			return (boost::format("Parsing of \"%1%\" failed!") % errSubject).str();
		else if (errCode == LIBLO_ERROR_FILE_NOT_UTF8)
			return (boost::format("\"%1%\" is not encoded in valid UTF-8!") % errSubject).str(); 
		else if (errCode == LIBLO_ERROR_FILE_NOT_FOUND)
			return (boost::format("\"%1%\" cannot be found!") % errSubject).str();
		else if (errCode == LIBLO_ERROR_TIMESTAMP_READ_FAIL)
			return (boost::format("The modification date of \"%1%\" cannot be read! Filesystem response: \"%2%\".") % errSubject % errDetail).str();
		else if (errCode == LIBLO_ERROR_TIMESTAMP_WRITE_FAIL)
			return (boost::format("The modification date of \"%1%\" cannot be written! Filesystem response: \"%2%\".") % errSubject % errDetail).str();
		else if (errCode == LIBLO_ERROR_NO_MEM)
			return "Memory allocation failed.";
		else if (errCode == LIBLO_ERROR_INVALID_ARGS)
			return errSubject;
		else
			return "No error.";
	}
	uint32_t	error::code()			const {
		return errCode;
	}

	error lastException = error();
}