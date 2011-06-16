/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#ifndef __SUPPORT_HELPERS__HPP__
#define __SUPPORT_HELPERS__HPP__

#include "Types.h"

#include <cstring>
#include <iostream>
#include "boost/filesystem.hpp"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	//////////////////////////////////////////////////////////////////////////
	// Helper functions
	//////////////////////////////////////////////////////////////////////////


	//
	// string ParseVersion(string&):
	//	- Tries to extract the version string value from the given text,
	//	using the above defined regexes to do the dirty work.
	//
	string ParseVersion(const string& text);

	//
	// bool ReadLine(InputStream, OutputString&):
	//	- Reads a text line from the received input stream.
	//
	bool ReadLine(istream& is, string& s);

	/// bool GetLine(istream& is, string& s)
	///  - Reads a text line skipping all the empty lines along the way
	bool GetLine(istream& is, string& s);

	/// bool GetLine(istream& is, string& s)
	///  - Reads a text line skipping all the empty lines along the way
	string GetLine(istream& is);

	//
	// string ReadString(pointer&, maxsize):
	//	- Reads a consecutive array of charactes up to maxsize length and 
	//	returns them as a new string.
	//
	string ReadString(char*& bufptr, ushort size = 512);

	//
	// T Peek<T>(pointer&):
	//	- Peeks into the received buffer and returns the value pointed 
	//	converting it to the type T.
	//
	template <typename T> 
	T Peek(char* buffer) {
		return *reinterpret_cast<T*>(buffer);
	}

	//
	// T Read<T>(pointer&):
	//	- Tries to extract a value of the specified type T from the 
	//	received buffer, incrementing the pointer to point past the readen 
	//	value.
	//
	template <typename T> 
	inline T Read(char*& buffer) {
		T value = Peek<T>(buffer);
		buffer += sizeof(T);
		return value;
	}

	// Launches the specified file using the most appropriate program for viewing it.
	int Launch(const string& filename);

	//Changes uppercase to lowercase and removes preceding and trailing spaces.	
	string Tidy(string text);

	//Checks if a given object is an esp or an esm.
	bool IsPlugin(string object);
	
	//Checks if the plugin exists at the given location, even if ghosted.
	bool Exists(const fs::path plugin);

	//Determines if a given mod is a game's main master file or not.
	bool IsMasterFile(const string plugin);

	//Reads the header from mod file and prints a string representation which includes the version text, if found.
	string GetModHeader(const fs::path& filename);

	//Calculate the CRC of the given file for comparison purposes.
	unsigned int GetCrc32(const fs::path& filename);

	//Reads an entire file into a string buffer.
	void fileToBuffer(const fs::path file, string& buffer);

	//Removes the ".ghost" extension from ghosted filenames. 
	string TrimDotGhost(string plugin);

	//Checks if the given plugin is ghosted in the user's install.
	//NOT if the plugin given has a '.ghost' extension.
	bool IsGhosted(fs::path plugin);

	//Gets the given OBSE dll or OBSE plugin dll's version number.
	//Also works for FOSE and NVSE.
	//NOT CROSS-PLATFORM. Requires Windows.h.
	string GetExeDllVersion(const fs::path& filename);
}

#endif
