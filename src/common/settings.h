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

#ifndef COMMON_SETTINGS_H_
#define COMMON_SETTINGS_H_

#include <cstdint>

#include <string>

#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>

#include "common/dll_def.h"

namespace boss {

namespace fs = boost::filesystem;

class BOSS_COMMON ParsingError;

///////////////////////////////
// Settings Class
///////////////////////////////

class BOSS_COMMON Settings {
 public:
	void Load(const fs::path file);                                     // Throws exception on fail.
	void Save(const fs::path file, const std::uint32_t currentGameId);  // Throws exception on fail.

	ParsingError ErrorBuffer() const;
	void ErrorBuffer(const ParsingError buffer);

	std::string GetValue(const std::string setting) const;
 private:
	ParsingError errorBuffer;
	boost::unordered_map<std::string, std::string> iniSettings;

	std::string GetIniGameString(const std::uint32_t game) const;
	std::string GetLogFormatString() const;
	std::string GetLanguageString() const;
	void ApplyIniSettings();
};

}  // namespace boss
#endif  // COMMON_SETTINGS_H_
