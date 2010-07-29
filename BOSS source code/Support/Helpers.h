/*	Better Oblivion Sorting Software
	2.0 Beta
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    	http://creativecommons.org/licenses/by-nc-nd/3.0/

	 $Id$
	$URL$
*/

#ifndef __SUPPORT_HELPERS__HPP__
#define __SUPPORT_HELPERS__HPP__

#include <cstring>
#include <iostream>

#include <Support/Types.h>

namespace boss {

using namespace std;


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

//
// string ReadString(pointer&, maxsize):
//	- Reads a consecutive array of charactes up to maxsize length and 
//	returns them as a new string.
//
string ReadString(char*& bufptr, ushort size = 512);

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

//
// T Peek<T>(pointer&):
//	- Peeks into the received buffer and returns the value pointed 
//	converting it to the type T.
//
template <typename T> 
T Peek(char* buffer) {
	return *reinterpret_cast<T*>(buffer);
}

};

#endif __SUPPORT_HELPERS__HPP__