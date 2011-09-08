/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/


#include "Types.h"
#include "Helpers.h"
#include "VersionRegex.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sstream>


namespace boss {
	using namespace std;
	using namespace boost;


#if _WIN32 || _WIN64
	const string launcher_cmd = "start";
#else
	const string launcher_cmd = "xdg-open";
#endif

	// Reads a text line from the input stream
	bool ReadLine(istream& is, string& s)
	{
		s.erase();
		while(is)
		{
			// Get next char in input buffer
			char c = static_cast<char>(is.get());

			if (!is) 
				break;

			// Check for termination conditions
			if (c == '\n') 
				break;

			// While termination condition not found -> append chars to result string
			s.append(1, c);
		}

		return (is != 0);
	}

	// Reads a text line skipping all the empty lines along the way
	bool GetLine(istream& is, string& s) 
	{
		while (ReadLine(is, s)) {
			trim_right(s);

			if (!s.empty()) {
				break;
			}
		}

		return is && !s.empty();
	}

	// Reads a text line skipping all the empty lines along the way
	string GetLine(istream& is) 
	{
		string line;
		if (GetLine(is, line)){
			return line;
		}

		return string();
	}


	// Reads a string until the terminator char is found or the complete buffer is consumed.
	wstring ReadString(char*& bufptr, ushort size){
		wstring data;
	
		data.reserve(size + 1);
		while (char c = *bufptr++) {
			data.append(1, c);
		}

		return data;
	}

	// Tries to parse the textual string to find a suitable version indication.
	wstring ParseVersion(const wstring& text){

		wstring::const_iterator begin, end;

		begin = text.begin();
		end = text.end();

		for(int i = 0; wregex* re = version_checks[i]; i++) {

			wsmatch what;
			while (regex_search(begin, end, what, *re)) {

				if (what.empty()){
					continue;
				}

				wssub_match match = what[1];
		
				if (!match.matched) {
					continue;
				}

				return trim_copy(wstring(match.first, match.second));

			}
		}

		return wstring();
	}

	wstring Tidy(wstring filename) {						//Changes uppercase to lowercase and removes trailing spaces to do what Windows filesystem does to filenames.	
		size_t endpos = filename.find_last_not_of(L" \t");
	
		if (filename.npos == endpos) return (L""); 			//sanity check for empty string
		else {
			filename = filename.substr(0, endpos+1);
			for (unsigned int i = 0; i < filename.length(); i++) filename[i] = tolower(filename[i]);
			return (filename);
		}
	}

	int Launch(const string& filename)
	{
		const string cmd = launcher_cmd + " " + filename;
		return ::system(cmd.c_str());
	}
}