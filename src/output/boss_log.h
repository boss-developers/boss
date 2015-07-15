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

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef OUTPUT_BOSS_LOG_H
#define OUTPUT_BOSS_LOG_H

#include <cstdint>

#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include "common/conditional_data.h"
#include "common/dll_def.h"
#include "common/error.h"

namespace boss {

namespace fs = boost::filesystem;

class BOSS_COMMON Outputter;

class BossLog {
 public:
	BossLog();
	BossLog(const std::uint32_t format);

	void SetFormat(const std::uint32_t format);
	void Save(const fs::path file, const bool overwrite);  // Saves contents to file. Throws boss_error exception on fail.
	void Clear();

	std::uint32_t recognised;
	std::uint32_t unrecognised;
	std::uint32_t inactive;
	std::uint32_t messages;
	std::uint32_t warnings;
	std::uint32_t errors;

	std::string scriptExtender;
	std::string gameName;

	Outputter updaterOutput;
	Outputter criticalError;
	Outputter userRules;
	Outputter sePlugins;
	Outputter recognisedPlugins;
	Outputter unrecognisedPlugins;

	std::vector<ParsingError> parsingErrors;
	std::vector<Message> globalMessages;

 private:
	std::uint32_t logFormat;
	bool recognisedHasChanged;

	std::string PrintLog();
	std::string PrintHeaderTop();
	std::string PrintHeaderBottom();
	std::string PrintFooter();

	bool HasRecognisedListChanged(const fs::path file);
};

}  // namespace boss;
#endif  // OUTPUT_BOSS_LOG_H
