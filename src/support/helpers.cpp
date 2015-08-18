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

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#include "support/helpers.h"

#include <sys/types.h>  // MCP Note: Possibly remove this one?

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>

#include <boost/crc.hpp>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <boost/spirit/include/karma.hpp>
//#include <boost/regex.hpp>

#include "alphanum.hpp"

#include "common/error.h"
#include "support/logger.h"

#if _WIN32 || _WIN64
#	include <shlobj.h>
#	include <windows.h>
#endif

namespace boss {

namespace fs = boost::filesystem;
namespace karma = boost::spirit::karma;
namespace bloc = boost::locale;

// Calculate the CRC of the given file for comparison purposes.
std::uint32_t GetCrc32(const fs::path &filename) {
	std::uint32_t chksum = 0;
	static const std::size_t buffer_size = 8192;
	char buffer[buffer_size];
	// MCP Note: changed from filename.c_str() to filename.string(); needs testing as error was about not being able to convert wchar_t to char
	std::ifstream ifile(filename.string(), std::ios::binary);  // MCP Note: I think this is std::ifstream as it's not using a path argument but I'm not sure
	LOG_TRACE("calculating CRC for: '%s'", filename.string().c_str());
	boost::crc_32_type result;
	if (ifile) {
		do {
			ifile.read(buffer, buffer_size);
			result.process_bytes(buffer, ifile.gcount());
		} while (ifile);
		chksum = result.checksum();
	} else {
		LOG_WARN("unable to open file for CRC calculation: '%s'",
		         filename.string().c_str());
	}
	LOG_DEBUG("CRC32('%s'): 0x%x", filename.string().c_str(), chksum);
	return chksum;
}

// Reads an entire file into a string buffer.
void fileToBuffer(const fs::path file, std::string &buffer) {
	// MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
	std::ifstream ifile(file.string());
	if (ifile.fail())
		return;
	ifile.unsetf(std::ios::skipws);  // No white space skipping!
	std::copy(std::istream_iterator<char>(ifile), std::istream_iterator<char>(),
	          std::back_inserter(buffer));
}

// Converts an integer to a string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
BOSS_COMMON std::string IntToString(const std::uint32_t n) {
	std::string out;
	std::back_insert_iterator<std::string> sink(out);
	karma::generate(sink, karma::upper[karma::uint_], n);
	return out;
}

// Converts an integer to a hex string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
std::string IntToHexString(const std::uint32_t n) {
	std::string out;
	std::back_insert_iterator<std::string> sink(out);
	karma::generate(sink, karma::upper[karma::hex], n);
	return out;
}

// Converts a boolean to a string representation (true/false)
std::string BoolToString(bool b) {
	if (b)
		return "true";
	return "false";
	//return b ? "true" : "false";
}

// Turns "true", "false", "1", "0" into booleans.
bool StringToBool(std::string str) {
	return (str == "true" || str == "1");
}

// Convert a Windows-1252 string to UTF-8.
std::string From1252ToUTF8(const std::string &str) {
	try {
		return bloc::conv::to_utf<char>(str, "Windows-1252", bloc::conv::stop);
	}
	catch (bloc::conv::conversion_error &e) {
		throw boss_error(BOSS_ERROR_FILE_NOT_UTF8, "\"" + str + "\" cannot be encoded in Windows-1252.");
	}
}

// Convert a UTF-8 string to Windows-1252.
std::string FromUTF8To1252(const std::string &str) {
	try {
		return bloc::conv::from_utf<char>(str, "Windows-1252", bloc::conv::stop);
	}
	catch (bloc::conv::conversion_error &e) {
		throw boss_error(BOSS_ERROR_FILE_NOT_UTF8, "\"" + str + "\" cannot be encoded in Windows-1252.");
	}
}

// Check if registry subkey exists.
BOSS_COMMON bool RegKeyExists(std::string keyStr, std::string subkey, std::string value) {
	if (RegKeyStringValue(keyStr, subkey, value).empty())
		return false;
	return true;
}

// Get registry subkey value string.
std::string RegKeyStringValue(std::string keyStr, std::string subkey, std::string value) {
#if _WIN32 || _WIN64
	HKEY hKey, key;
	DWORD BufferSize = 4096;
	wchar_t val[4096];

	if (keyStr == "HKEY_CLASSES_ROOT")
		key = HKEY_CLASSES_ROOT;
	else if (keyStr == "HKEY_CURRENT_CONFIG")
		key = HKEY_CURRENT_CONFIG;
	else if (keyStr == "HKEY_CURRENT_USER")
		key = HKEY_CURRENT_USER;
	else if (keyStr == "HKEY_LOCAL_MACHINE")
		key = HKEY_LOCAL_MACHINE;
	else if (keyStr == "HKEY_USERS")
		key = HKEY_USERS;

	// TODO(MCP): This worked before the file-split and namespace qualification but now refuses to compile, complaining of a type mismatch. Figure out why it's now broken.
	// TODO(MCP): Master still doesn't spit this out so something in the changes broke it. Changes to this file were removing using namespace std and using namespace boost.
	// TODO(MCP): Fixed, I think. Had to define _UNICODE. May fix some of the mismatches. Will test.
	// TODO(MCP): Error seems to be back after swapping Boost Regex for STL Regex in all files.
	// TODO(MCP): Fixed, I think. Had to force the Unicode version to be used. Still not sure about the other mismatches that don't deal with the Windows API. May need to use UNICODE for Windows and _UNICODE. That fixes it, need to investigate.
	LONG ret = RegOpenKeyExW(key, fs::path(subkey).wstring().c_str(),
	                        0, KEY_READ|KEY_WOW64_32KEY, &hKey);

	if (ret == ERROR_SUCCESS) {
		// TODO(MCP): This worked before the file-split and namespace qualification but now refuses to compile, complaining of a type mismatch. Figure out why it's now broken.
		// TODO(MCP): Master still doesn't spit this out so something in the changes broke it. Changes to this file were removing using namespace std and using namespace boost.
		// TODO(MCP): Fixed, I think. Had to define _UNICODE. May fix some of the mismatches. Will test.
		ret = RegQueryValueExW(hKey, fs::path(value).wstring().c_str(), NULL,
		                      NULL, (LPBYTE)&val, &BufferSize);
		RegCloseKey(hKey);

		if (ret == ERROR_SUCCESS)
			return fs::path(val).string();  // Easiest way to convert from wide to narrow character strings.
		return "";
	}
#endif
	return "";
}


//////////////////////////////
// Version Class Functions
//////////////////////////////

Version::Version() {}

Version::Version(const char *ver) : verString(ver) {}

Version::Version(const std::string ver) : verString(ver) {}

Version::Version(const fs::path file) {
	LOG_TRACE("extracting version from '%s'", file.string().c_str());
#if _WIN32 || _WIN64
	DWORD dummy = 0;
	// TODO(MCP): This worked before the file-split and namespace qualification but now refuses to compile, complaining of a type mismatch. Figure out why it's now broken.
	// TODO(MCP): Master still doesn't spit this out so something in the changes broke it. Changes to this file were removing using namespace std and using namespace boost.
	// TODO(MCP): Fixed, I think. Had to define _UNICODE. May fix some of the mismatches. Will test.
	DWORD size = GetFileVersionInfoSizeW(file.wstring().c_str(), &dummy);

	if (size > 0) {
		LPBYTE point = new BYTE[size];
		UINT uLen;
		VS_FIXEDFILEINFO *info;
		std::string ver;

		// TODO(MCP): This worked before the file-split and namespace qualification but now refuses to compile, complaining of a type mismatch. Figure out why it's now broken.
		// TODO(MCP): Master still doesn't spit this out so something in the changes broke it. Changes to this file were removing using namespace std and using namespace boost.
		// TODO(MCP): Fixed, I think. Had to define _UNICODE. May fix some of the mismatches. Will test.
		GetFileVersionInfoW(file.wstring().c_str(), 0, size, point);

		// TODO(MCP): This worked before the file-split and namespace qualification but now refuses to compile, complaining of a type mismatch. Figure out why it's now broken.
		// TODO(MCP): Master still doesn't spit this out so something in the changes broke it. Changes to this file were removing using namespace std and using namespace boost.
		// TODO(MCP): Fixed, I think. Had to define _UNICODE. May fix some of the mismatches. Will test.
		VerQueryValueW(point, L"\\", (LPVOID *)&info, &uLen);

		DWORD dwLeftMost     = HIWORD(info->dwFileVersionMS);
		DWORD dwSecondLeft   = LOWORD(info->dwFileVersionMS);
		DWORD dwSecondRight  = HIWORD(info->dwFileVersionLS);
		DWORD dwRightMost    = LOWORD(info->dwFileVersionLS);

		delete [] point;

		verString = IntToString(dwLeftMost) + '.' + IntToString(dwSecondLeft) + '.' + IntToString(dwSecondRight) + '.' + IntToString(dwRightMost);
	}
#else
	// Ensure filename has no quote characters in it to avoid command injection attacks
	if (std::string::npos != file.string().find('"')) {
		LOG_WARN("filename has embedded quotes; skipping to avoid command injection: '%s'",
		         file.string().c_str());
	} else {
		// Command mostly borrowed from the gnome-exe-thumbnailer.sh script
		// wrestool is part of the icoutils package
		std::string cmd = "wrestool --extract --raw --type=version \"" + file.string() + "\" | tr '\\0, ' '\\t.\\0' | sed 's/\\t\\t/_/g' | tr -c -d '[:print:]' | sed -r 's/.*Version[^0-9]*([0-9]+(\\.[0-9]+)+).*/\\1/'";

		FILE *fp = popen(cmd.c_str(), "r");

		// Read out the version string
		static const std::uint32_t BUFSIZE = 32;
		char buf[BUFSIZE];
		if (NULL == std::fgets(buf, BUFSIZE, fp)) {
			LOG_DEBUG("failed to extract version from '%s'",
			          file.string().c_str());
		} else {
			verString = std::string(buf);
			LOG_DEBUG("extracted version from '%s': %s", file.string().c_str(),
			          verString.c_str());
		}
		pclose(fp);
	}
#endif
}

std::string Version::AsString() const {
	return verString;
}

bool Version::operator > (Version ver) {
	return (*this != ver && !(*this < ver));
}

bool Version::operator < (Version ver) {
	// Version string could have a wide variety of formats. Use regex to choose specific comparison types.

	std::regex reg1("(\\d+\\.?)+");  // a.b.c.d.e.f.... where the letters are all integers, and 'a' is the shortest possible match.

	//std::regex reg2("(\\d+\\.?)+([a-zA-Z\\-]+(\\d+\\.?)*)+");  // Matches a mix of letters and numbers - from "0.99.xx", "1.35Alpha2", "0.9.9MB8b1", "10.52EV-D", "1.62EV" to "10.0EV-D1.62EV".

	if (std::regex_match(verString, reg1) &&
	    std::regex_match(ver.AsString(), reg1)) {
		// First type: numbers separated by periods. If two versions have a different number of numbers, then the shorter should be padded
		// with zeros. An arbitrary number of numbers should be supported.
		std::istringstream parser1(verString);
		std::istringstream parser2(ver.AsString());
		while (parser1.good() || parser2.good()) {
			// Check if each stringstream is OK for i/o before doing anything with it. If not, replace its extracted value with a 0.
			std::uint32_t n1, n2;
			if (parser1.good()) {
				parser1 >> n1;
				parser1.get();
			} else {
				n1 = 0;
			}
			if (parser2.good()) {
				parser2 >> n2;
				parser2.get();
			} else {
				n2 = 0;
			}
			if (n1 < n2)
				return true;
			else if (n1 > n2)
				return false;
		}
		return false;
	}
	// Wacky format. Use the Alphanum Algorithm. (what a name!)
	return (doj::alphanum_comp(verString, ver.AsString()) < 0);
}

bool Version::operator >= (Version ver) {
	return (*this == ver || *this > ver);
}

bool Version::operator == (Version ver) {
	return (verString == ver.AsString());
}

bool Version::operator != (Version ver) {
	return !(*this == ver);
}

}  // namespace boss
