/*	Better Oblivion Sorting Software
	2.0 Beta
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    	http://creativecommons.org/licenses/by-nc-nd/3.0/

	 $Id: Helpers.cpp 1200 2010-07-29 22:51:09Z leandor@gmail.com $
	$URL: https://better-oblivion-sorting-software.googlecode.com/svn/BOSS%20source%20code/Support/Helpers.cpp $
*/

#include <strstream>

#include <boost/regex.hpp>

#include <Support/Types.h>
#include <Support/Helpers.h>
#include <Support/VersionRegex.h>

namespace boss {

using namespace std;
using namespace boost;

// Reads a text line from the input stream
bool ReadLine(istream& is, string& s)
{
	s.erase();
	while(is)
	{
		// Get next char in input buffer
		char c = static_cast<char>(is.get());

		// Check for termination conditions
		if (c <= 0 || c == '\r' || c == '\n') {
			if (c > 0){
				// If EOLN then consume the second char in the '\r\n' pair
				c = static_cast<char>(is.peek());
				if (c == '\n' || c == '\r') {
					is.get();
				}
			}

			return true;
		}

		// While termination condition not found -> append chars to result string
		s.append(1, c);
	}

	return !s.empty();
}

// Reads a string until the terminator char is found or the complete buffer is consumed.
string ReadString(char*& bufptr, ushort size){
	string data;
	
	data.reserve(size + 1);
	while (char c = *bufptr++) {
		data.append(1, c);
	}

	return data;
}

// Tries to parse the textual string to find a suitable version indication.
string ParseVersion(const string& text){

	istrstream iss(text.c_str(), text.length());
	string data;

	while (ReadLine(iss, data)) {
		if (data.empty()){
			continue;
		}

		smatch what;

		regex* re;
		for(int i = 0; re = version_checks[i]; i++) {
			if (regex_match(data, what, *re)){
				
				if (what.empty()){
					continue;
				}

				ssub_match match = what[3];
		
				if (!match.matched) {
					continue;
				}

				string result;
				result.assign(match.first, match.second);
				return result;
			}
		}
	}
	return string();
}

};