/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/


#include "Helpers.h"
#include "VersionRegex.h"
#include "ModFormat.h"
#include "Logger.h"
#include "Common/Globals.h"
#include "Output/Output.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>

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

	BOSS_COMMON int Launch(const string& filename)
	{
		const string cmd = launcher_cmd + " " + filename;
		return ::system(cmd.c_str());
	}

	//Changes uppercase to lowercase and removes preceding and trailing spaces.	
	BOSS_COMMON string Tidy(string filename) {
		boost::algorithm::trim(filename);
		boost::algorithm::to_lower(filename);
		return filename;
	}

	//Checks if a given object is an esp or an esm.
	bool IsPlugin(string object) {
		object = fs::path(object).extension().string();
		boost::algorithm::trim(object);
		to_lower(object);
		return (object == ".esp" || object == ".esm");
	}

	//Checks if the plugin exists at the given location, even if ghosted.
	bool Exists(const fs::path plugin) {
		return (fs::exists(plugin) || fs::exists(plugin.string() + ".ghost"));
	}

	//Determines if a given mod is a game's main master file or not.
	bool IsMasterFile(const string plugin) {
		return ((Tidy(plugin) == "oblivion.esm") || (Tidy(plugin) == "fallout3.esm") || (Tidy(plugin) == "nehrim.esm") || (Tidy(plugin) == "falloutnv.esm") || (Tidy(plugin) == "skyrim.esm"));
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

	//Searches a hashset for the first matching string of a regex and returns its iterator position.
	boost::unordered_set<string>::iterator FindRegexMatch(const boost::unordered_set<string> set, const boost::regex reg, boost::unordered_set<string>::iterator startPos) {
		while(startPos != set.end()) {
			string mod = *startPos;
			if (boost::regex_match(mod,reg))
				return startPos;
			++startPos;
		}
		return set.end();
	}
}