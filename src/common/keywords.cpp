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

#include "common/keywords.h"

#include <cstdint>

namespace boss {

// DO NOT CHANGE THESE VALUES. THEY MUST BE CONSTANT FOR API USERS.
BOSS_COMMON const std::uint32_t NONE         = 0;
// RuleList keywords.
BOSS_COMMON const std::uint32_t ADD          = 1;
BOSS_COMMON const std::uint32_t OVERRIDE     = 2;
BOSS_COMMON const std::uint32_t FOR          = 3;
BOSS_COMMON const std::uint32_t BEFORE       = 4;
BOSS_COMMON const std::uint32_t AFTER        = 5;
BOSS_COMMON const std::uint32_t TOP          = 6;
BOSS_COMMON const std::uint32_t BOTTOM       = 7;
BOSS_COMMON const std::uint32_t APPEND       = 8;
BOSS_COMMON const std::uint32_t REPLACE      = 9;
// Masterlist keywords.
BOSS_COMMON const std::uint32_t SAY          = 10;
BOSS_COMMON const std::uint32_t TAG          = 11;
BOSS_COMMON const std::uint32_t REQ          = 12;
BOSS_COMMON const std::uint32_t INC          = 13;
BOSS_COMMON const std::uint32_t DIRTY        = 14;
BOSS_COMMON const std::uint32_t WARN         = 15;
BOSS_COMMON const std::uint32_t ERR          = 16;

// Item types.
BOSS_COMMON const std::uint32_t MOD          = 0;
BOSS_COMMON const std::uint32_t BEGINGROUP   = 1;
BOSS_COMMON const std::uint32_t ENDGROUP     = 2;
BOSS_COMMON const std::uint32_t REGEX        = 3;

}  // namespace boss
