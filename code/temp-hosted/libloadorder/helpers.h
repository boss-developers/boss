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

#ifndef LIBLO_HELPERS_H
#define LIBLO_HELPERS_H

#include <string>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>

namespace liblo {
	// std::string to null-terminated uint8_t string converter.
	uint8_t * ToUint8_tString(std::string str);

	//UTF-8 file validator.
	bool ValidateUTF8File(const boost::filesystem::path file);

	//Reads an entire file into a string buffer.
	void fileToBuffer(const boost::filesystem::path file, std::string& buffer);

	//Converts an integer to a string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
	std::string IntToString(const uint32_t n);

	// converts between encodings.
	class Transcoder {
	private:
		//0x81, 0x8D, 0x8F, 0x90, 0x9D in 1252 are undefined in UTF-8.
		boost::unordered_map<char, uint32_t> commonMap;  //1251/1252, UTF-8. 0-127, plus some more.
		boost::unordered_map<char, uint32_t> map1252toUtf8; //1252, UTF-8. 128-255, minus a few common characters.
		boost::unordered_map<uint32_t, char> utf8toEnc;
		boost::unordered_map<char, uint32_t> encToUtf8;
		uint32_t currentEncoding;
	public:
		Transcoder();
		void SetEncoding(const uint32_t inEncoding);
		uint32_t GetEncoding();

		std::string Utf8ToEnc(const std::string inString);
		std::string EncToUtf8(const std::string inString);
	};

	//Version class for more robust version comparisons.
	class Version {
	private:
		std::string verString;
	public:
		Version();
		Version(const char * ver);
		Version(const boost::filesystem::path file);

		std::string AsString() const;

		bool operator > (Version);
		bool operator < (Version);
		bool operator >= (Version);
		bool operator == (Version);
		bool operator != (Version);
	};
}

#endif