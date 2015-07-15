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

#ifndef COMMON_KEYWORDS_H_
#define COMMON_KEYWORDS_H_

#include "common/dll_def.h"

#include <cstdint>

namespace boss {

// MCP Note: Possibly convert these into enums?
BOSS_COMMON extern const std::uint32_t NONE;
// RuleList keywords.
BOSS_COMMON extern const std::uint32_t ADD;
BOSS_COMMON extern const std::uint32_t OVERRIDE;
BOSS_COMMON extern const std::uint32_t FOR;
BOSS_COMMON extern const std::uint32_t BEFORE;
BOSS_COMMON extern const std::uint32_t AFTER;
BOSS_COMMON extern const std::uint32_t TOP;
BOSS_COMMON extern const std::uint32_t BOTTOM;
BOSS_COMMON extern const std::uint32_t APPEND;
BOSS_COMMON extern const std::uint32_t REPLACE;
// Masterlist keywords.
BOSS_COMMON extern const std::uint32_t SAY;
BOSS_COMMON extern const std::uint32_t TAG;
BOSS_COMMON extern const std::uint32_t REQ;
BOSS_COMMON extern const std::uint32_t INC;
BOSS_COMMON extern const std::uint32_t DIRTY;
BOSS_COMMON extern const std::uint32_t WARN;
BOSS_COMMON extern const std::uint32_t ERR;

// Item types.
BOSS_COMMON extern const std::uint32_t MOD;
BOSS_COMMON extern const std::uint32_t BEGINGROUP;
BOSS_COMMON extern const std::uint32_t ENDGROUP;
BOSS_COMMON extern const std::uint32_t REGEX;

}  // namespace boss
#endif  // COMMON_KEYWORDS_H_
