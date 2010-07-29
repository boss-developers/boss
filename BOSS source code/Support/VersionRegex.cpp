/*	Better Oblivion Sorting Software
	2.0 Beta
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    	http://creativecommons.org/licenses/by-nc-nd/3.0/

	 $Id$
	$URL$
*/

#include <boost/regex.hpp>

namespace boss {

	using namespace boost;

	/// REGEX expression definition
	///  Each expression is composed of three parts:
	///    1. The marker string "version", "ver", "rev", "v" or "r"
	///    2. Optional punctuation like '.' or ':' and surrounding spaces
	///    3. The version string itself.

	// Matches "Version: 12.33" and "Version 1.23"
	const char* re1 = 
			"(\\s*)"								// what[1]
			"(version\\s*(?:\\:)?\\s*)"				// what[2]
			"(\\S+)"								// what[3]
			;

	// Matches "ver 1.12", "ver. 1.12", "ver: 1.23" and "ver.: 1.23"
	const char* re2 = 
			"(\\s*)"								// what[1]
			"(ver\\s*(?:\\.?|\\:?)+\\s*)"			// what[2]
			"(\\S+)"								// what[3]
			;

	// Matches "rev 1.12", "rev. 1.12", "rev: 1.23" and "rev.: 1.23"
	const char* re3 = 
			"(\\s*)"								// what[1]
			"(rev\\s*(?:\\.?|\\:?)+\\s*)"			// what[2]
			"(\\S+)"								// what[3]
			;

	// Matches "v1.12", "v. 1.12", "v: 1.23" and "v 1.23"
	const char* re4 = 
			"(\\s*)"								// what[1]
			"(v(?:\\.\\s?|\\:\\s?|\\s*))"			// what[2]
			"(\\S+)"								// what[3]
			;

	// Matches "r1.12", "r. 1.12", "r: 1.23" and "r 1.23"
	const char* re5 = 
			"(\\s*)"								// what[1]
			"(r(?:\\.\\s?|\\:\\s?|\\s*))"			// what[2]
			"((?:[0-9.]+)(?:[0-9a-zA-Z.\\-]*))"		// what[3]
			;

	/// Array used to try each of the expressions defined above using 
	/// an iteration for each of them.
	regex* version_checks[] = {
			new regex(re1, regex::icase),
			new regex(re2, regex::icase),
			new regex(re3, regex::icase),
			new regex(re4, regex::icase),
			new regex(re5, regex::icase),
			0
			};

};