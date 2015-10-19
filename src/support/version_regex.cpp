/*	BOSS

	A "one-click" program for users that quickly optimises and avoids
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge,
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

	Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of BOSS.

	BOSS is free software: you can redistribute
	it and/or modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	BOSS is distributed in the hope that it will
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with BOSS.  If not, see
	<http://www.gnu.org/licenses/>.

	$Revision: 1681 $, $Date: 2010-10-17 21:01:25 +0100 (Sun, 17 Oct 2010) $
*/

#include "support/version_regex.h"

//#include <regex>

//#include <boost/regex.hpp>

#include "base/regex.h"

namespace boss {

/// REGEX expression definition
///  Each expression is composed of three parts:
///    1. The marker string "version", "ver", "rev", "v" or "r"
///    2. The version string itself.

const char *regex1 =
    "^(?:\\bversion\\b[ ]*(?:[:.\\-]?)|\\brevision\\b(?:[:.\\-]?))[ ]*"
    "((?:alpha|beta|test|debug)?\\s*[-0-9a-zA-Z._+]+\\s*(?:alpha|beta|test|debug)?\\s*(?:[0-9]*))$";

const char *regex2 =
    "(?:\\bversion\\b(?:[ :]?)|\\brevision\\b(?:[:.\\-]?))[ ]*"
    "([0-9][-0-9a-zA-Z._]+\\+?)";

const char *regex3 =
    "(?:\\bver(?:[:.]?)|\\brev(?:[:.]?))\\s*"
    "([0-9][-0-9a-zA-Z._]*\\+?)";

// Matches "Updated: <date>" for the Bashed patch
const char *regex4 =
    "(?:Updated:)\\s*"
    "([-0-9aAmMpP/ :]+)$";

// Matches isolated versions as last resort
const char *regex5 =
    "(?:(?:\\bv|\\br)(?:\\s?)(?:[-.:])?(?:\\s*))"
    "((?:(?:\\balpha\\b)?|(?:\\bbeta\\b)?)\\s*[0-9][-0-9a-zA-Z._]*\\+?)";

// Matches isolated versions as last resort
const char *regex6 =
    "((?:(?:\\balpha\\b)?|(?:\\bbeta\\b)?)\\s*\\b[0-9][-0-9a-zA-Z._]*\\+?)$";

const char *regex7 =
    "(^\\bmark\\b\\s*\\b[IVX0-9][-0-9a-zA-Z._+]*\\s*(?:alpha|beta|test|debug)?\\s*(?:[0-9]*)?)$";

/// Array used to try each of the expressions defined above using
/// an iteration for each of them.
// TODO(MCP): Swap out Boost Regex for STL Regex once the infinite loop that occurs with VS is sorted out
//std::regex *version_checks[] = {new std::regex(regex1, std::regex::icase),
//                                new std::regex(regex2, std::regex::icase),
//                                new std::regex(regex3, std::regex::icase),
//                                new std::regex(regex4, std::regex::icase),
//                                new std::regex(regex5, std::regex::icase),  // This incorrectly identifies "OBSE v19" where 19 is any integer.
//                                new std::regex(regex6, std::regex::icase),  // This is responsible for metallicow's false positive.
//                                new std::regex(regex7, std::regex::icase), 0};
boss_regex::regex *version_checks[] = {new boss_regex::regex(regex1, boss_regex::regex::icase),
                                       new boss_regex::regex(regex2, boss_regex::regex::icase),
                                       new boss_regex::regex(regex3, boss_regex::regex::icase),
                                       new boss_regex::regex(regex4, boss_regex::regex::icase),
                                       new boss_regex::regex(regex5, boss_regex::regex::icase),  // This incorrectly identifies "OBSE v19" where 19 is any integer.
                                       new boss_regex::regex(regex6, boss_regex::regex::icase),  // This is responsible for metallicow's false positive.
                                       new boss_regex::regex(regex7, boss_regex::regex::icase), 0};

}  // namespace boss
