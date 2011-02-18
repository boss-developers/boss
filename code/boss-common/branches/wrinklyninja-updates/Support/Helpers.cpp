/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/


#include "Helpers.h"
#include "VersionRegex.h"
#include "ModFormat.h"
#include "../Common/Globals.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>

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

		string::const_iterator begin, end;

		begin = text.begin();
		end = text.end();

		for(int i = 0; regex* re = version_checks[i]; i++) {

			smatch what;
			while (regex_search(begin, end, what, *re)) {

				if (what.empty()){
					continue;
				}

				ssub_match match = what[1];
		
				if (!match.matched) {
					continue;
				}

				return trim_copy(string(match.first, match.second));

			}
		}

		return string();
	}

/*	string Tidy(string filename) {						//Changes uppercase to lowercase and removes trailing spaces to do what Windows filesystem does to filenames.	
		
		size_t endpos = filename.find_last_not_of(" \t");
	
		if (filename.npos == endpos) return (""); 			//sanity check for empty string
		else {
			filename = filename.substr(0, endpos+1);
			for (unsigned int i = 0; i < filename.length(); i++) filename[i] = tolower(filename[i]);
			return (filename);
		}
	}
*/
	string Tidy(string filename) {
		boost::algorithm::trim(filename);
		boost::algorithm::to_lower(filename);
		return filename;
	}

	int Launch(const string& filename)
	{
		const string cmd = launcher_cmd + " " + filename;
		return ::system(cmd.c_str());
	}

	//Checks if a given object is an esp, an esm or a ghosted mod.
	bool IsPlugin(string object) {
		to_lower(object);
		return (object.find(".esp")!=string::npos || object.find(".esm")!=string::npos);
	}

	//Checks if a plugin exists, even if ghosted.
	bool PluginExists(fs::path plugin) {
		return (fs::exists(plugin) || fs::exists(plugin.native()+fs::path(".ghost").native()));
	}

	/// GetModHeader(string textbuf):
	///  - Reads the header from mod file and prints a string representation which includes the version text, if found.
	///
	string GetModHeader(const fs::path& filename, bool ghosted) {

	//	ostringstream out;
		ModHeader header;

		// Read mod's header now...
		if (ghosted) header = ReadHeader(data_path / fs::path(filename.string()+".ghost"));
		else header = ReadHeader(data_path / filename);

		// The current mod's version if found, or empty otherwise.
		string version = header.Version;

		//Return the version if found, otherwise an empty string.
		return version;
	}

	//Calculate the CRC of the given file for comparison purposes.
	int GetCrc32(const fs::path& filename) {
		boost::crc_32_type result;
		result.process_bytes(filename.string().data(), filename.string().length());
		return result.checksum();
	}

	//Determines if a given mod is a game's main master file or not.
	bool IsMasterFile(string plugin) {
		return (Tidy(plugin)=="oblivion.esm") || (Tidy(plugin)=="fallout3.esm") || (Tidy(plugin)=="nehrim.esm") || (Tidy(plugin)=="falloutnv.esm");
	}
}