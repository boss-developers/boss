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

	const wchar_t* regex1 = 
		L"^(?:\\bversion\\b[ ]*(?:[:.\\-]?)|\\brevision\\b(?:[:.\\-]?))[ ]*"
		L"((?:alpha|beta|test|debug)?\\s*[-0-9a-zA-Z._+]+\\s*(?:alpha|beta|test|debug)?\\s*(?:[0-9]*))$"
		;

	const wchar_t* regex2 = 
		L"(?:\\bversion\\b(?:[ :]?)|\\brevision\\b(?:[:.\\-]?))[ ]*"
		L"([0-9][-0-9a-zA-Z._]+\\+?)"
		;

	const wchar_t* regex3 = 
		L"(?:\\bver(?:[:.]?)|\\brev(?:[:.]?))\\s*"
		L"([0-9][-0-9a-zA-Z._]*\\+?)"
		;

	// Matches "Updated: <date>" for the Bashed patch
	const wchar_t* regex4 = 
		L"(?:Updated:)\\s*"
		L"([-0-9aAmMpP/ :]+)$"
		;

	// Matches isolated versions as last resort
	const wchar_t* regex5 = 
		L"(?:(?:\\bv|\\br)(?:\\s?)(?:[-.:])?(?:\\s*))"
		L"((?:(?:\\balpha\\b)?|(?:\\bbeta\\b)?)\\s*[0-9][-0-9a-zA-Z._]*\\+?)"
		;

	// Matches isolated versions as last resort
	const wchar_t* regex6 = 
		L"((?:(?:\\balpha\\b)?|(?:\\bbeta\\b)?)\\s*\\b[0-9][-0-9a-zA-Z._]*\\+?)$"
		;

	const wchar_t* regex7 = 
		L"(^\\bmark\\b\\s*\\b[IVX0-9][-0-9a-zA-Z._+]*\\s*(?:alpha|beta|test|debug)?\\s*(?:[0-9]*)?)$"
		;

	/// Array used to try each of the expressions defined above using 
	/// an iteration for each of them.
	wregex* version_checks[] = {
			new wregex(regex1, regex::icase),
			new wregex(regex2, regex::icase),
			new wregex(regex3, regex::icase),
			new wregex(regex4, regex::icase),
			new wregex(regex5, regex::icase),
			new wregex(regex6, regex::icase),
			new wregex(regex7, regex::icase),
			0
			};

}