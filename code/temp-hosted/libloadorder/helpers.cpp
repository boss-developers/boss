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

#include "helpers.h"
#include "libloadorder.h"
#include "exception.h"
#include <sstream>
#include <source/utf8.h>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/regex.hpp>
#include <alphanum.hpp>

#if _WIN32 || _WIN64
#	include "Windows.h"
#	include "Shlobj.h"
#endif

using namespace std;

namespace liblo {

	// std::string to null-terminated uint8_t string converter.
	uint8_t * ToUint8_tString(string str) {
		size_t length = str.length() + 1;
		uint8_t * p = new uint8_t[length];

		for (size_t j=0; j < str.length(); j++) {  //UTF-8, so this is code-point by code-point rather than char by char, but same result here.
			p[j] = str[j];
		}
		p[length - 1] = '\0';
		return p;
	}

	//UTF-8 file validator.
	bool ValidateUTF8File(const boost::filesystem::path file) {
		ifstream ifs(file.c_str());

		istreambuf_iterator<char> it(ifs.rdbuf());
		istreambuf_iterator<char> eos;

		if (!utf8::is_valid(it, eos))
			return false;
		else
			return true;
	}

	//Reads an entire file into a string buffer.
	void fileToBuffer(const boost::filesystem::path file, string& buffer) {
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

	//Converts an integer to a string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
	string IntToString(const uint32_t n) {
		string out;
		back_insert_iterator<string> sink(out);
		boost::spirit::karma::generate(sink,boost::spirit::karma::upper[boost::spirit::karma::uint_],n);
		return out;
	}


	////////////////////////////////
	// Transcoder Class Functions
	////////////////////////////////

	Transcoder::Transcoder() {
		//Fill common character maps (1251/1252 -> UTF-8).
		commonMap.emplace('\x00', 0x0000);
		commonMap.emplace('\x01', 0x0001);
		commonMap.emplace('\x02', 0x0002);
		commonMap.emplace('\x03', 0x0003);
		commonMap.emplace('\x04', 0x0004);
		commonMap.emplace('\x05', 0x0005);
		commonMap.emplace('\x06', 0x0006);
		commonMap.emplace('\x07', 0x0007);
		commonMap.emplace('\x08', 0x0008);
		commonMap.emplace('\x09', 0x0009);
		commonMap.emplace('\x0A', 0x000A);
		commonMap.emplace('\x0B', 0x000B);
		commonMap.emplace('\x0C', 0x000C);
		commonMap.emplace('\x0D', 0x000D);
		commonMap.emplace('\x0E', 0x000E);
		commonMap.emplace('\x0F', 0x000F);
		commonMap.emplace('\x10', 0x0010);
		commonMap.emplace('\x11', 0x0011);
		commonMap.emplace('\x12', 0x0012);
		commonMap.emplace('\x13', 0x0013);
		commonMap.emplace('\x14', 0x0014);
		commonMap.emplace('\x15', 0x0015);
		commonMap.emplace('\x16', 0x0016);
		commonMap.emplace('\x17', 0x0017);
		commonMap.emplace('\x18', 0x0018);
		commonMap.emplace('\x19', 0x0019);
		commonMap.emplace('\x1A', 0x001A);
		commonMap.emplace('\x1B', 0x001B);
		commonMap.emplace('\x1C', 0x001C);
		commonMap.emplace('\x1D', 0x001D);
		commonMap.emplace('\x1E', 0x001E);
		commonMap.emplace('\x1F', 0x001F);
		commonMap.emplace('\x20', 0x0020);
		commonMap.emplace('\x21', 0x0021);
		commonMap.emplace('\x22', 0x0022);
		commonMap.emplace('\x23', 0x0023);
		commonMap.emplace('\x24', 0x0024);
		commonMap.emplace('\x25', 0x0025);
		commonMap.emplace('\x26', 0x0026);
		commonMap.emplace('\x27', 0x0027);
		commonMap.emplace('\x28', 0x0028);
		commonMap.emplace('\x29', 0x0029);
		commonMap.emplace('\x2A', 0x002A);
		commonMap.emplace('\x2B', 0x002B);
		commonMap.emplace('\x2C', 0x002C);
		commonMap.emplace('\x2D', 0x002D);
		commonMap.emplace('\x2E', 0x002E);
		commonMap.emplace('\x2F', 0x002F);
		commonMap.emplace('\x30', 0x0030);
		commonMap.emplace('\x31', 0x0031);
		commonMap.emplace('\x32', 0x0032);
		commonMap.emplace('\x33', 0x0033);
		commonMap.emplace('\x34', 0x0034);
		commonMap.emplace('\x35', 0x0035);
		commonMap.emplace('\x36', 0x0036);
		commonMap.emplace('\x37', 0x0037);
		commonMap.emplace('\x38', 0x0038);
		commonMap.emplace('\x39', 0x0039);
		commonMap.emplace('\x3A', 0x003A);
		commonMap.emplace('\x3B', 0x003B);
		commonMap.emplace('\x3C', 0x003C);
		commonMap.emplace('\x3D', 0x003D);
		commonMap.emplace('\x3E', 0x003E);
		commonMap.emplace('\x3F', 0x003F);
		commonMap.emplace('\x40', 0x0040);
		commonMap.emplace('\x41', 0x0041);
		commonMap.emplace('\x42', 0x0042);
		commonMap.emplace('\x43', 0x0043);
		commonMap.emplace('\x44', 0x0044);
		commonMap.emplace('\x45', 0x0045);
		commonMap.emplace('\x46', 0x0046);
		commonMap.emplace('\x47', 0x0047);
		commonMap.emplace('\x48', 0x0048);
		commonMap.emplace('\x49', 0x0049);
		commonMap.emplace('\x4A', 0x004A);
		commonMap.emplace('\x4B', 0x004B);
		commonMap.emplace('\x4C', 0x004C);
		commonMap.emplace('\x4D', 0x004D);
		commonMap.emplace('\x4E', 0x004E);
		commonMap.emplace('\x4F', 0x004F);
		commonMap.emplace('\x50', 0x0050);
		commonMap.emplace('\x51', 0x0051);
		commonMap.emplace('\x52', 0x0052);
		commonMap.emplace('\x53', 0x0053);
		commonMap.emplace('\x54', 0x0054);
		commonMap.emplace('\x55', 0x0055);
		commonMap.emplace('\x56', 0x0056);
		commonMap.emplace('\x57', 0x0057);
		commonMap.emplace('\x58', 0x0058);
		commonMap.emplace('\x59', 0x0059);
		commonMap.emplace('\x5A', 0x005A);
		commonMap.emplace('\x5B', 0x005B);
		commonMap.emplace('\x5C', 0x005C);
		commonMap.emplace('\x5D', 0x005D);
		commonMap.emplace('\x5E', 0x005E);
		commonMap.emplace('\x5F', 0x005F);
		commonMap.emplace('\x60', 0x0060);
		commonMap.emplace('\x61', 0x0061);
		commonMap.emplace('\x62', 0x0062);
		commonMap.emplace('\x63', 0x0063);
		commonMap.emplace('\x64', 0x0064);
		commonMap.emplace('\x65', 0x0065);
		commonMap.emplace('\x66', 0x0066);
		commonMap.emplace('\x67', 0x0067);
		commonMap.emplace('\x68', 0x0068);
		commonMap.emplace('\x69', 0x0069);
		commonMap.emplace('\x6A', 0x006A);
		commonMap.emplace('\x6B', 0x006B);
		commonMap.emplace('\x6C', 0x006C);
		commonMap.emplace('\x6D', 0x006D);
		commonMap.emplace('\x6E', 0x006E);
		commonMap.emplace('\x6F', 0x006F);
		commonMap.emplace('\x70', 0x0070);
		commonMap.emplace('\x71', 0x0071);
		commonMap.emplace('\x72', 0x0072);
		commonMap.emplace('\x73', 0x0073);
		commonMap.emplace('\x74', 0x0074);
		commonMap.emplace('\x75', 0x0075);
		commonMap.emplace('\x76', 0x0076);
		commonMap.emplace('\x77', 0x0077);
		commonMap.emplace('\x78', 0x0078);
		commonMap.emplace('\x79', 0x0079);
		commonMap.emplace('\x7A', 0x007A);
		commonMap.emplace('\x7B', 0x007B);
		commonMap.emplace('\x7C', 0x007C);
		commonMap.emplace('\x7D', 0x007D);
		commonMap.emplace('\x7E', 0x007E);
		commonMap.emplace('\x7F', 0x007F);
		commonMap.emplace('\x82', 0x201A);
		commonMap.emplace('\x84', 0x201E);
		commonMap.emplace('\x85', 0x2026);
		commonMap.emplace('\x86', 0x2026);
		commonMap.emplace('\x87', 0x2021);
		commonMap.emplace('\x89', 0x2030);
		commonMap.emplace('\x8B', 0x2039);
		commonMap.emplace('\x91', 0x2018);
		commonMap.emplace('\x92', 0x2019);
		commonMap.emplace('\x93', 0x201C);
		commonMap.emplace('\x94', 0x201D);
		commonMap.emplace('\x95', 0x2022);
		commonMap.emplace('\x96', 0x2013);
		commonMap.emplace('\x97', 0x2014);
		commonMap.emplace('\x99', 0x2122);
		commonMap.emplace('\x9B', 0x203A);
		commonMap.emplace('\xA0', 0x00A0);
		commonMap.emplace('\xA4', 0x00A4);
		commonMap.emplace('\xA6', 0x00A6);
		commonMap.emplace('\xA7', 0x00A7);
		commonMap.emplace('\xA9', 0x00A9);
		commonMap.emplace('\xAB', 0x00AB);
		commonMap.emplace('\xAC', 0x00AC);
		commonMap.emplace('\xAD', 0x00AD);
		commonMap.emplace('\xAE', 0x00AE);
		commonMap.emplace('\xB0', 0x00B0);
		commonMap.emplace('\xB1', 0x00B1);
		commonMap.emplace('\xB5', 0x00B5);
		commonMap.emplace('\xB6', 0x00B6);
		commonMap.emplace('\xB7', 0x00B7);
		commonMap.emplace('\xBB', 0x00BB);

		//Now fill 1252 -> UTF-8 map.
		map1252toUtf8 = commonMap;  //Fill with common mapped characters.
		map1252toUtf8.emplace('\x80', 0x20AC);
		map1252toUtf8.emplace('\x83', 0x0192);
		map1252toUtf8.emplace('\x88', 0x02C6);
		map1252toUtf8.emplace('\x8A', 0x0160);
		map1252toUtf8.emplace('\x8C', 0x0152);
		map1252toUtf8.emplace('\x8E', 0x017D);
		map1252toUtf8.emplace('\x98', 0x02DC);
		map1252toUtf8.emplace('\x9A', 0x0161);
		map1252toUtf8.emplace('\x9C', 0x0153);
		map1252toUtf8.emplace('\x9E', 0x017E);
		map1252toUtf8.emplace('\x9F', 0x0178);
		map1252toUtf8.emplace('\xA1', 0x00A1);
		map1252toUtf8.emplace('\xA2', 0x00A2);
		map1252toUtf8.emplace('\xA3', 0x00A3);
		map1252toUtf8.emplace('\xA5', 0x00A5);
		map1252toUtf8.emplace('\xA8', 0x00A8);
		map1252toUtf8.emplace('\xAA', 0x00AA);
		map1252toUtf8.emplace('\xAF', 0x00AF);
		map1252toUtf8.emplace('\xB2', 0x00B2);
		map1252toUtf8.emplace('\xB3', 0x00B3);
		map1252toUtf8.emplace('\xB4', 0x00B4);
		map1252toUtf8.emplace('\xB8', 0x00B8);
		map1252toUtf8.emplace('\xB9', 0x00B9);
		map1252toUtf8.emplace('\xBA', 0x00BA);
		map1252toUtf8.emplace('\xBC', 0x00BC);
		map1252toUtf8.emplace('\xBD', 0x00BD);
		map1252toUtf8.emplace('\xBE', 0x00BE);
		map1252toUtf8.emplace('\xBF', 0x00BF);
		map1252toUtf8.emplace('\xC0', 0x00C0);
		map1252toUtf8.emplace('\xC1', 0x00C1);
		map1252toUtf8.emplace('\xC2', 0x00C2);
		map1252toUtf8.emplace('\xC3', 0x00C3);
		map1252toUtf8.emplace('\xC4', 0x00C4);
		map1252toUtf8.emplace('\xC5', 0x00C5);
		map1252toUtf8.emplace('\xC6', 0x00C6);
		map1252toUtf8.emplace('\xC7', 0x00C7);
		map1252toUtf8.emplace('\xC8', 0x00C8);
		map1252toUtf8.emplace('\xC9', 0x00C9);
		map1252toUtf8.emplace('\xCA', 0x00CA);
		map1252toUtf8.emplace('\xCB', 0x00CB);
		map1252toUtf8.emplace('\xCC', 0x00CC);
		map1252toUtf8.emplace('\xCD', 0x00CD);
		map1252toUtf8.emplace('\xCE', 0x00CE);
		map1252toUtf8.emplace('\xCF', 0x00CF);
		map1252toUtf8.emplace('\xD0', 0x00D0);
		map1252toUtf8.emplace('\xD1', 0x00D1);
		map1252toUtf8.emplace('\xD2', 0x00D2);
		map1252toUtf8.emplace('\xD3', 0x00D3);
		map1252toUtf8.emplace('\xD4', 0x00D4);
		map1252toUtf8.emplace('\xD5', 0x00D5);
		map1252toUtf8.emplace('\xD6', 0x00D6);
		map1252toUtf8.emplace('\xD7', 0x00D7);
		map1252toUtf8.emplace('\xD8', 0x00D8);
		map1252toUtf8.emplace('\xD9', 0x00D9);
		map1252toUtf8.emplace('\xDA', 0x00DA);
		map1252toUtf8.emplace('\xDB', 0x00DB);
		map1252toUtf8.emplace('\xDC', 0x00DC);
		map1252toUtf8.emplace('\xDD', 0x00DD);
		map1252toUtf8.emplace('\xDE', 0x00DE);
		map1252toUtf8.emplace('\xDF', 0x00DF);
		map1252toUtf8.emplace('\xE0', 0x00E0);
		map1252toUtf8.emplace('\xE1', 0x00E1);
		map1252toUtf8.emplace('\xE2', 0x00E2);
		map1252toUtf8.emplace('\xE3', 0x00E3);
		map1252toUtf8.emplace('\xE4', 0x00E4);
		map1252toUtf8.emplace('\xE5', 0x00E5);
		map1252toUtf8.emplace('\xE6', 0x00E6);
		map1252toUtf8.emplace('\xE7', 0x00E7);
		map1252toUtf8.emplace('\xE8', 0x00E8);
		map1252toUtf8.emplace('\xE9', 0x00E9);
		map1252toUtf8.emplace('\xEA', 0x00EA);
		map1252toUtf8.emplace('\xEB', 0x00EB);
		map1252toUtf8.emplace('\xEC', 0x00EC);
		map1252toUtf8.emplace('\xED', 0x00ED);
		map1252toUtf8.emplace('\xEE', 0x00EE);
		map1252toUtf8.emplace('\xEF', 0x00EF);
		map1252toUtf8.emplace('\xF0', 0x00F0);
		map1252toUtf8.emplace('\xF1', 0x00F1);
		map1252toUtf8.emplace('\xF2', 0x00F2);
		map1252toUtf8.emplace('\xF3', 0x00F3);
		map1252toUtf8.emplace('\xF4', 0x00F4);
		map1252toUtf8.emplace('\xF5', 0x00F5);
		map1252toUtf8.emplace('\xF6', 0x00F6);
		map1252toUtf8.emplace('\xF7', 0x00F7);
		map1252toUtf8.emplace('\xF8', 0x00F8);
		map1252toUtf8.emplace('\xF9', 0x00F9);
		map1252toUtf8.emplace('\xFA', 0x00FA);
		map1252toUtf8.emplace('\xFB', 0x00FB);
		map1252toUtf8.emplace('\xFC', 0x00FC);
		map1252toUtf8.emplace('\xFD', 0x00FD);
		map1252toUtf8.emplace('\xFE', 0x00FE);
		map1252toUtf8.emplace('\xFF', 0x00FF);
		
		currentEncoding = 0;
	}

	void Transcoder::SetEncoding(const uint32_t inEncoding) {
		if (inEncoding != 1252)
			return;

		//Set the enc -> UTF-8 map.
		encToUtf8 = map1252toUtf8;
		currentEncoding = inEncoding;
		
		//Now create the UTF-8 -> enc map.
		for (boost::unordered_map<char, uint32_t>::iterator iter = encToUtf8.begin(); iter != encToUtf8.end(); ++iter) {
			utf8toEnc.emplace(iter->second, iter->first);  //Swap mapping. There *should* be unique values for each character either way.
		}
	}

	uint32_t Transcoder::GetEncoding() {
		return currentEncoding;
	}

	string Transcoder::Utf8ToEnc(const string inString) {
		stringstream outString;
		string::const_iterator strIter = inString.begin();
		//I need to use a UTF-8 string iterator. See UTF-CPP for usage.
		try {
			utf8::iterator<string::const_iterator> iter(strIter, strIter, inString.end());
			for (iter; iter.base() != inString.end(); ++iter) {
				boost::unordered_map<uint32_t, char>::iterator mapIter = utf8toEnc.find(*iter);
				if (mapIter != utf8toEnc.end())
					outString << mapIter->second;
				else
					throw error(LIBLO_WARN_BAD_FILENAME, inString);
			}
		} catch (...) {
			throw error(LIBLO_WARN_BAD_FILENAME, inString);
		}
		return outString.str();
	}

	string Transcoder::EncToUtf8(const string inString) {
		string outString;
		for (string::const_iterator iter = inString.begin(); iter != inString.end(); ++iter) {
			boost::unordered_map<char, uint32_t>::iterator mapIter = encToUtf8.find(*iter);
			if (mapIter != encToUtf8.end()) {
				try {
					utf8::append(mapIter->second, back_inserter(outString));
				} catch (...) {
					throw error(LIBLO_WARN_BAD_FILENAME, inString);
				}
			} else
				throw error(LIBLO_WARN_BAD_FILENAME, inString);
		}
		//Let's check that it's valid UTF-8.
		if (!utf8::is_valid(outString.begin(), outString.end()))
			throw error(LIBLO_WARN_BAD_FILENAME, outString);
		return outString;
	}


	//////////////////////////////
	// Version Class Functions
	//////////////////////////////

	Version::Version() {}

	Version::Version(const char * ver) 
		: verString(ver) {}

	Version::Version(const boost::filesystem::path file) {
#if _WIN32 || _WIN64
		DWORD dummy = 0;
		DWORD size = GetFileVersionInfoSize(file.wstring().c_str(), &dummy);

		if (size > 0) {
			LPBYTE point = new BYTE[size];
			UINT uLen;
			VS_FIXEDFILEINFO *info;
			string ver;

			GetFileVersionInfo(file.wstring().c_str(),0,size,point);

			VerQueryValue(point,L"\\",(LPVOID *)&info,&uLen);

			DWORD dwLeftMost     = HIWORD(info->dwFileVersionMS);
			DWORD dwSecondLeft   = LOWORD(info->dwFileVersionMS);
			DWORD dwSecondRight  = HIWORD(info->dwFileVersionLS);
			DWORD dwRightMost    = LOWORD(info->dwFileVersionLS);
			
			delete [] point;

			verString = IntToString(dwLeftMost) + '.' + IntToString(dwSecondLeft) + '.' + IntToString(dwSecondRight) + '.' + IntToString(dwRightMost);
		}
#else
        // ensure filename has no quote characters in it to avoid command injection attacks
        if (string::npos != file.string().find('"')) {
    	    LOG_WARN("filename has embedded quotes; skipping to avoid command injection: '%s'", file.string().c_str());
        } else {
            // command mostly borrowed from the gnome-exe-thumbnailer.sh script
            // wrestool is part of the icoutils package
            string cmd = "wrestool --extract --raw --type=version \"" + file.string() + "\" | tr '\\0, ' '\\t.\\0' | sed 's/\\t\\t/_/g' | tr -c -d '[:print:]' | sed -r 's/.*Version[^0-9]*([0-9]+(\\.[0-9]+)+).*/\\1/'";

            FILE *fp = popen(cmd.c_str(), "r");

            // read out the version string
            static const uint32_t BUFSIZE = 32;
            char buf[BUFSIZE];
            if (NULL == fgets(buf, BUFSIZE, fp)) {
    	        LOG_DEBUG("failed to extract version from '%s'", file.string().c_str());
            }
            else {
                verString = string(buf);
	   	        LOG_DEBUG("extracted version from '%s': %s", file.string().c_str(), retVal.c_str());
            }
            pclose(fp);
        }
#endif
	}
	
	string Version::AsString() const {
		return verString;
	}

	bool Version::operator < (Version ver) {
		//Version string could have a wide variety of formats. Use regex to choose specific comparison types.

		boost::regex reg1("(\\d+\\.?)+");  //a.b.c.d.e.f.... where the letters are all integers, and 'a' is the shortest possible match.

		//boost::regex reg2("(\\d+\\.?)+([a-zA-Z\\-]+(\\d+\\.?)*)+");  //Matches a mix of letters and numbers - from "0.99.xx", "1.35Alpha2", "0.9.9MB8b1", "10.52EV-D", "1.62EV" to "10.0EV-D1.62EV".

		if (boost::regex_match(verString, reg1) && boost::regex_match(ver.AsString(), reg1)) {
			//First type: numbers separated by periods. If two versions have a different number of numbers, then the shorter should be padded 
			//with zeros. An arbitrary number of numbers should be supported.
			istringstream parser1(verString);
			istringstream parser2(ver.AsString());
			while (parser1.good() || parser2.good()) {
				//Check if each stringstream is OK for i/o before doing anything with it. If not, replace its extracted value with a 0.
				uint32_t n1, n2;
				if (parser1.good()) {
					parser1 >> n1;
					parser1.get();
				} else
					n1 = 0;
				if (parser2.good()) {
					parser2 >> n2;
					parser2.get();
				} else
					n2 = 0;
				if (n1 < n2)
					return true;
				else if (n1 > n2)
					return false;
			}
			return false;
		} else {
			//Wacky format. Use the Alphanum Algorithm. (what a name!)
			return (doj::alphanum_comp(verString, ver.AsString()) < 0);
		}
	}

	bool Version::operator > (Version ver) {
		return (*this != ver && !(*this < ver));
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
}