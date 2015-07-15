/*	BOSS

	A "one-click" program for users that quickly optimises and avoids
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge,
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

	Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of BOSS.

	BOSS is free software: you can redistribute
	it and/or modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	BOSS is distributed in the hope that it will
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with BOSS.  If not, see
	<http://www.gnu.org/licenses/>.

	$Revision: 2488 $, $Date: 2011-03-27 14:31:33 +0100 (Sun, 27 Mar 2011) $
*/

#include "support/mod_format.h"

#include <fstream>
#include <string>

#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

//#include "support/helpers.h"  // MCP Note: Don't think this one is needed. Will delete later after confirmation
#include "support/types.h"
#include "support/version_regex.h"

namespace boss {

namespace fs = boost::filesystem;

// MCP Note: Make function prototypes for these?

/*
 * string ParseVersion(string&):
 * - Tries to extract the version string value from the given text,
 * using the above defined regexes to do the dirty work.
 */
std::string ParseVersion(const std::string& text) {
	std::string::const_iterator begin, end;

	begin = text.begin();
	end = text.end();

	for(int i = 0; boost::regex* re = version_checks[i]; i++) {
		boost::smatch what;
		while (boost::regex_search(begin, end, what, *re)) {
			if (what.empty()) {
				continue;
			}

			boost::ssub_match match = what[1];

			if (!match.matched) {
				continue;
			}

			return boost::algorithm::trim_copy(std::string(match.first, match.second));
		}
	}

	return std::string();
}

/*
 * string ReadString(pointer&, maxsize):
 * - Reads a consecutive array of charactes up to maxsize length and
 * returns them as a new string.
 */
std::string ReadString(char*& bufptr, unsigned short size) {
	std::string data;

	data.reserve(size + 1);
	while (char c = *bufptr++) {
		data.append(1, c);
	}

	return data;
}

/*
 * T Peek<T>(pointer&):
 * - Peeks into the received buffer and returns the value pointed
 * converting it to the type T.
 */
template <typename T>
T Peek(char* buffer) {
	return *reinterpret_cast<T*>(buffer);
}

/*
 * T Read<T>(pointer&):
 * - Tries to extract a value of the specified type T from the
 * received buffer, incrementing the pointer to point past the readen
 * value.
 */
template <typename T>
inline T Read(char*& buffer) {
	T value = Peek<T>(buffer);
	buffer += sizeof(T);
	return value;
}

/*
 * ModHeader ReadHeader(string):
 * - Parses the mod file contents and extract the header information
 * returning the most important data using a ModHeader struct.
 * --> see: http://www.uesp.net/wiki/Tes4Mod:Mod_File_Format,
 *
 * and in particular this link: http://www.uesp.net/wiki/Tes4Mod:Mod_File_Format/TES4
 */

bool IsPluginMaster(fs::path filename) {
	char buffer[MAXLENGTH];
	char* bufptr = buffer;
	ModHeader modHeader;

	if (filename.empty())
		return false;

	// MCP Note: changed from filename.native().c_str() to filename.string(); needs testing as error was about not being able to convert wchar_t to char
	// Note 2: According to Boost docs, c_str() is the same as specifying native().c_str()?
	std::ifstream file(filename.string(), std::ios_base::binary | std::ios_base::in);

	if (file.bad())
		//throw boss_error(BOSS_ERROR_FILE_READ_FAIL, filename.string());
		return false;

	// Reads the first MAXLENGTH bytes into the buffer
	file.read(&buffer[0], sizeof(buffer));

	// Check for the 'magic' marker at start
	if (Read<unsigned int>(bufptr) != Record::TES4) {
		return false;
	}

	// Next field is the total header size
	/*uint headerSize = */Read<unsigned int>(bufptr);

	// Next comes the header record Flags
	unsigned int flags = Read<unsigned int>(bufptr);

	// LSb of this record's flags is used to indicate if the
	// mod is a master or a plugin
	return ((flags & 0x1) != 0);
}

ModHeader ReadHeader(fs::path filename) {
	char buffer[MAXLENGTH];
	char* bufptr = buffer;
	ModHeader modHeader;
	// MCP Note: changed from filename.native().c_str() to filename.string(); needs testing as error was about not being able to convert wchar_t to char
	std::ifstream file(filename.string(), std::ios_base::binary | std::ios_base::in);

	modHeader.Name = filename.string();

	// Reads the first MAXLENGTH bytes into the buffer
	file.read(&buffer[0], sizeof(buffer));

	// Check for the 'magic' marker at start
	if (Read<unsigned int>(bufptr) != Record::TES4) {
		return modHeader;
	}

	// Next field is the total header size
	/*uint headerSize = */Read<unsigned int>(bufptr);

	// Next comes the header record Flags
	unsigned int flags = Read<unsigned int>(bufptr);

	// LSb of this record's flags is used to indicate if the
	// mod is a master or a plugin
	modHeader.IsMaster = (flags & 0x1) != 0;

	// Next comes the FormID...
	/*uint formId = */Read<unsigned int>(bufptr);  // Skip formID

	// ...and extra flags
	/*uint flags2 = */Read<unsigned int>(bufptr);  // Skip flags2

	// For Oblivion plugins, the Header record starts here, check for its signature 'HEDR'.
	if (Read<unsigned int>(bufptr) != Record::HEDR) {
		// Check if it's a FO3, FNV or TES5 plugin.
		if (Read<unsigned int>(bufptr) != Record::HEDR) {  // Nope, exit.
			return modHeader;
		}
	}

	// HEDR record has fields: DataSize, Version (0.8 o 1.0), Record Count
	// and Next Object Id
	/*ushort dataSize = */Read<unsigned short>(bufptr);
	/*float version = */Read<float>(bufptr);
	/*int numRecords = */Read<int>(bufptr);
	/*uint nextObjId = */Read<unsigned int>(bufptr);

	// Then comes the sub-records
	unsigned int signature = Read<unsigned int>(bufptr);

	// Skip optional records
	bool loop = true;
	while (loop) {
		switch (signature) {
			case Record::OFST:
			case Record::DELE:
				bufptr += Read<unsigned short>(bufptr);  // Skip
				signature = Read<unsigned int>(bufptr);
				break;

			// Extract author name, if present
			case Record::CNAM:
				modHeader.Author = ReadString(bufptr, Read<unsigned short>(bufptr));
				signature = Read<uint>(bufptr);
				break;

			// Extract description and version, if present
			case Record::SNAM:
				modHeader.Description = ReadString(bufptr, Read<unsigned short>(bufptr));
				modHeader.Version     = ParseVersion(modHeader.Description);
				signature = Read<unsigned int>(bufptr);
				break;

			default:
				loop = false;
		}
	}

	// We should have all the required information.
	return modHeader;
}

}  // namespace boss
