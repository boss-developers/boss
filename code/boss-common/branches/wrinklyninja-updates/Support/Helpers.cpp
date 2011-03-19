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

	int Launch(const string& filename)
	{
		const string cmd = launcher_cmd + " " + filename;
		return ::system(cmd.c_str());
	}

	//Changes uppercase to lowercase and removes preceding and trailing spaces.	
	string Tidy(string filename) {
		boost::algorithm::trim(filename);
		boost::algorithm::to_lower(filename);
		return filename;
	}

	//Checks if a given object is an esp or an esm.
	bool IsPlugin(string object) {
		to_lower(object);
		return (object.find(".esp")!=string::npos || object.find(".esm")!=string::npos);
	}

	//Checks if the plugin exists at the given location, even if ghosted.
	bool Exists(const fs::path plugin) {
		return (fs::exists(plugin) || fs::exists(plugin.string() + ".ghost"));
	}

	//Reads the header from mod file and prints a string representation which includes the version text, if found.
	string GetModHeader(const fs::path& filename) {

	//	ostringstream out;
		ModHeader header;

		// Read mod's header now...
		header = ReadHeader(data_path / filename);

		// The current mod's version if found, or empty otherwise.
		string version = header.Version;

		//Return the version if found, otherwise an empty string.
		return version;
	}

	//Calculate the CRC of the given file for comparison purposes.
	unsigned int GetCrc32(const fs::path& filename) {
		boost::crc_32_type result;
		string buffer;
		fileToBuffer(filename, buffer);
		result.process_bytes(buffer.data(), buffer.length());
		return result.checksum();
	}

	//Determines if a given mod is a game's main master file or not.
	bool IsMasterFile(const string plugin) {
		return (Tidy(plugin)=="oblivion.esm") || (Tidy(plugin)=="fallout3.esm") || (Tidy(plugin)=="nehrim.esm") || (Tidy(plugin)=="falloutnv.esm");
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

	//Removes the ".ghost" extension from ghosted filenames.
	string TrimDotGhost(string plugin) {
		fs::path pluginPath(plugin);
		const string ext = to_lower_copy(pluginPath.extension().string());
		if (ext == ".ghost")
			return plugin.substr(0,plugin.length()-6);
		else
			return plugin;
	}

	//Checks if the given plugin is ghosted in the user's install.
	bool IsGhosted(fs::path plugin) {
		const string ext = to_lower_copy(plugin.extension().string());
		if (ext != ".ghost")  //Doesn't have .ghost extension. Add it.
			plugin = fs::path(plugin.string() + ".ghost");
		if (fs::exists(plugin))
			return true;
		else
			return false;
	}
}