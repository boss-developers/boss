/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#include <boost/regex.hpp>

namespace boss {

	using namespace boost;

	/// REGEX expression definition
	///  Each expression is composed of three parts:
	///    1. The marker string "version", "ver", "rev", "v" or "r"
	///    2. The version string itself.

	// Matches "Version: 12.33", "Ver 1.23", "Rev 1.23"
	const char* re1 = 
			"(version[:\\. ]*|ver[:\\. ]*|rev[:\\. ]*) *"
			"([-0-9a-zA-Z\\._ ]+\\+?)"
			"(.*)"
			;

	// Matches "R12.3" and "V1.23"
	const char* re2 = 
			"(\\(?r[:\\. ]*\\)?|\\(?v[:\\. ]*\\)?) *"
			"([-0-9a-zA-Z\\._ ]+\\+?)"
			"(.*)"
			;

	// Matches "Updated: <date>" for the Bashed patch
	const char* re3 = 
			"(Updated:) *"
			"([-0-9a-zA-Z/\\._ ]+\\+?)"
			"(.*)"
			;

	// Matches isolated versions as last resort
	const char* re4 = 
			"([^0-9]+) *"
			"([0-9][-0-9a-zA-Z\\._ ]*\\+?)"
			"(.*)"
			;

	/// Array used to try each of the expressions defined above using 
	/// an iteration for each of them.
	regex* version_checks[] = {
			new regex(re1, regex::icase | regex::extended),
			new regex(re2, regex::icase | regex::extended),
			new regex(re3, regex::icase | regex::extended),
			0
			};

};