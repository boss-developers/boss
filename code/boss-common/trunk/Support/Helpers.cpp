/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/


#include "Helpers.h"
#include "ModFormat.h"
#include "Logger.h"
#include "Common/Globals.h"
#include "Output/Output.h"

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

	//Changes uppercase to lowercase.	
	BOSS_COMMON_EXP string Tidy(string filename) {
		boost::algorithm::to_lower(filename);
		return filename;
	}

	//Calculate the CRC of the given file for comparison purposes.
	unsigned int GetCrc32(const fs::path& filename) {
		int chksum = 0;
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
		fs::path pStr = fs::current_path() / filepath;
		DWORD dummy = 0;
		DWORD size = GetFileVersionInfoSize(pStr.wstring().c_str(), &dummy);

		if (size > 0) {
			LPBYTE point = new BYTE[size];
			UINT uLen;
			VS_FIXEDFILEINFO *info;
			string ver;

			GetFileVersionInfo(pStr.wstring().c_str(),0,size,point);

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
            static const int BUFSIZE = 32;
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
}