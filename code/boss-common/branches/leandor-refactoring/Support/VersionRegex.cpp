/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#include <string>
#include <boost/regex.hpp>

namespace boss {

	using namespace std;
	using namespace boost;

	/// REGEX expression definition
	///  Each expression is composed of three parts:
	///    1. The marker string "version", "ver", "rev", "v" or "r"
	///    2. The version string itself.

	const char* regex1 = 
		"(?:\s*)"
		"(?:version(?:[:.\\- \\t]*)|revision(?:[:.\\- \\t]*))[ ]*"
		"([-0-9a-zA-Z._ ]+\\+?)$"
		;

	const char* regex2 = 
		"(?:\s*)"
		"(?:ver(?:[:.\\- \\t]*)|rev(?:[:.\\- \\t]*))[ ]*"
		"([-0-9a-zA-Z._ ]+\\+?)$"
		;

	// Matches "Updated: <date>" for the Bashed patch
	const char* regex3 = 
		"(?:Updated:)\\s*"
		"([-0-9aAmMpP/ :]+)$"
		;

	// Matches isolated versions as last resort
	const char* regex4 = 
		"(?:(?:v|r)(?:\s?)(?:[-.:])?(?:\s*))"
		"((?:(?:alpha)?|(?:beta)?|(?:mark)?)\s*[0-9][-0-9a-zA-Z._]*\\+?)"
		;

	/// Array used to try each of the expressions defined above using 
	/// an iteration for each of them.
	regex* version_checks[] = {
			new regex(regex1, regex::icase),
			new regex(regex2, regex::icase),
			new regex(regex3, regex::icase),
			new regex(regex4, regex::icase),
			//new regex(re2, regex::icase | regex::extended | regex::nosubs),
			//new regex(re3, regex::icase | regex::extended | regex::nosubs),
			0
			};

};