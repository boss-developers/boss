/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of Better Oblivion Sorting Software.

    Better Oblivion Sorting Software is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    Better Oblivion Sorting Software is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Better Oblivion Sorting Software.  If not, see 
	<http://www.gnu.org/licenses/>.

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/


#include "Support/Helpers.h"
#include "Support/ModFormat.h"
#include "Support/Logger.h"
#include "Common/Globals.h"

#include <boost/spirit/include/karma.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>

#include "source/utf8.h"

#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sstream>

#if _WIN32 || _WIN64
#  include "Windows.h"
#endif

namespace boss {
	using namespace std;
	using namespace boost;
	using boost::algorithm::replace_all;
	using boost::algorithm::replace_first;
	namespace karma = boost::spirit::karma;

	//Changes uppercase to lowercase.	
	BOSS_COMMON_EXP string Tidy(string filename) {
		boost::algorithm::to_lower(filename);
		return filename;
	}

	//Calculate the CRC of the given file for comparison purposes.
	uint32_t GetCrc32(const fs::path& filename) {
		uint32_t chksum = 0;
		static const size_t buffer_size = 8192;
		char buffer[buffer_size];
		ifstream ifile(filename.c_str(), ios::binary);
		LOG_TRACE("calculating CRC for: '%s'", filename.string().c_str());
		boost::crc_32_type result;
		if (ifile) {
			do {
				ifile.read(buffer, buffer_size);
				result.process_bytes(buffer, ifile.gcount());
			} while (ifile);
			chksum = result.checksum();
		} else {
			LOG_WARN("unable to open file for CRC calculation: '%s'", filename.string().c_str());
		}
		LOG_DEBUG("CRC32('%s'): 0x%x", filename.string().c_str(), chksum);
        return chksum;
	}

	//Gets the given .exe or .dll file's version number.
	string GetExeDllVersion(const fs::path& filepath) {
		string filename = filepath.string();
		LOG_TRACE("extracting version from '%s'", filename.c_str());
		string retVal = "";
#if _WIN32 || _WIN64
		// WARNING - NOT VERY SAFE, SEE http://www.boost.org/doc/libs/1_46_1/libs/filesystem/v3/doc/reference.html#current_path
		DWORD dummy = 0;
		DWORD size = GetFileVersionInfoSize(filepath.wstring().c_str(), &dummy);

		if (size > 0) {
			LPBYTE point = new BYTE[size];
			UINT uLen;
			VS_FIXEDFILEINFO *info;
			string ver;

			GetFileVersionInfo(filepath.wstring().c_str(),0,size,point);

			VerQueryValue(point,L"\\",(LPVOID *)&info,&uLen);

			DWORD dwLeftMost     = HIWORD(info->dwFileVersionMS);
			DWORD dwSecondLeft   = LOWORD(info->dwFileVersionMS);
			DWORD dwSecondRight  = HIWORD(info->dwFileVersionLS);
			DWORD dwRightMost    = LOWORD(info->dwFileVersionLS);
			
			delete [] point;

			retVal = IntToString(dwLeftMost) + '.' + IntToString(dwSecondLeft) + '.' + IntToString(dwSecondRight) + '.' + IntToString(dwRightMost);
		}
#else
        // ensure filename has no quote characters in it to avoid command injection attacks
        if (string::npos != filename.find('"')) {
    	    LOG_WARN("filename has embedded quotes; skipping to avoid command injection: '%s'", filename.c_str());
        } else {
            // command mostly borrowed from the gnome-exe-thumbnailer.sh script
            // wrestool is part of the icoutils package
            string cmd = "wrestool --extract --raw --type=version \"" + filename + "\" | tr '\\0, ' '\\t.\\0' | sed 's/\\t\\t/_/g' | tr -c -d '[:print:]' | sed -r 's/.*Version[^0-9]*([0-9]+(\\.[0-9]+)+).*/\\1/'";

            FILE *fp = popen(cmd.c_str(), "r");

            // read out the version string
            static const uint32_t BUFSIZE = 32;
            char buf[BUFSIZE];
            if (NULL == fgets(buf, BUFSIZE, fp)) {
    	        LOG_DEBUG("failed to extract version from '%s'", filename.c_str());
            }
            else {
                retVal = string(buf);
	   	        LOG_DEBUG("extracted version from '%s': %s", filename.c_str(), retVal.c_str());
            }
            pclose(fp);
        }
#endif
		return retVal;
	}

	//Reads an entire file into a string buffer.
	void fileToBuffer(const fs::path file, string& buffer) {
		ifstream ifile(file.c_str());
		if (ifile.fail())
			return;
		ifile.unsetf(ios::skipws); // No white space skipping!
		copy(
			istream_iterator<char>(ifile),
			istream_iterator<char>(),
			back_inserter(buffer)
		);
	}

	//UTF-8 file validator.
	bool ValidateUTF8File(const fs::path file) {
		ifstream ifs(file.c_str());

		istreambuf_iterator<char> it(ifs.rdbuf());
		istreambuf_iterator<char> eos;

		if (!utf8::is_valid(it, eos))
			return false;
		else
			return true;
	}

	//Converts an integer to a string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
	BOSS_COMMON_EXP string IntToString(const uint32_t n) {
		string out;
		back_insert_iterator<string> sink(out);
		karma::generate(sink,karma::upper[karma::uint_],n);
		return out;
	}

	//Converts an integer to a hex string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
	string IntToHexString(const uint32_t n) {
		string out;
		back_insert_iterator<string> sink(out);
		karma::generate(sink,karma::upper[karma::hex],n);
		return out;
	}

	//Converts a boolean to a string representation (true/false)
	string BoolToString(bool b) {
		if (b)
			return "true";
		else
			return "false";
	}

	//Check if registry subkey exists.
	bool RegKeyExists(string keyStr, string subkey) {
#if _WIN32 || _WIN64
		HKEY hKey, key;

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

		LONG lRes = RegOpenKeyEx(key, fs::path(subkey).wstring().c_str(), 0, KEY_READ, &hKey);
		RegCloseKey(hKey);

		if (lRes == ERROR_SUCCESS)
			return true;
		else
			return false;
#else
		return false;
#endif
	}

	//Get registry subkey value string.
	string RegKeyStringValue(string keyStr, string subkey, string value) {
#if _WIN32 || _WIN64
		HKEY key;
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

		LONG lRes = RegGetValue(key, fs::path(subkey).wstring().c_str(), fs::path(value).wstring().c_str(), RRF_RT_REG_SZ, NULL, (PVOID)&val, &BufferSize);

		if (lRes == ERROR_SUCCESS) {
			return fs::path(val).string();  //Easiest way to convert from wide to narrow character strings.
		} else
			return "";
#else
		return "";
#endif
	}
}