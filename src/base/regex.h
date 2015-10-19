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

	$Revision: 3135 $, $Date: 2011-08-17 22:01:17 +0100 (Wed, 17 Aug 2011) $
*/

#ifndef BASE_REGEX_H_
#define BASE_REGEX_H_

#if _MSC_VER == 1900
#include <boost/regex.hpp>
#else
#include <regex>
#endif

namespace boss {
namespace boss_regex {  // Internal to boss::

#if _MSC_VER == 1900
using boost::regex;
using boost::regex_match;
using boost::smatch;
using boost::regex_search;
using boost::ssub_match;
using boost::regex_error;
#else
using std::regex;
using std::regex_match;
using std::smatch;
using std::regex_search;
using std::ssub_match;
using std::regex_error;
#endif

}  // namespace boss_regex
}  // namespace boss
#endif  // BASE_REGEX_H_
