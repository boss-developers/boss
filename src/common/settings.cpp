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

#include "common/settings.h"

#include <cstdint>
#include <cstdlib>

#include <fstream>
#include <ostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <boost/unordered_map.hpp>

#include "common/error.h"
#include "common/globals.h"
#include "parsing/grammar.h"
#include "support/helpers.h"

namespace boss {

namespace fs = boost::filesystem;
namespace bloc = boost::locale;

///////////////////////////////
// Settings Class
///////////////////////////////

void Settings::Load(const fs::path file) {
	Skipper skipper;
	ini_grammar grammar;
	std::string::const_iterator begin, end;
	std::string contents;

	skipper.SkipIniComments(true);
	grammar.SetErrorBuffer(&errorBuffer);

	fileToBuffer(file, contents);

	begin = contents.begin();
	end = contents.end();

	//iterator_type u32b(begin);
	//iterator_type u32e(end);

	//bool r = phrase_parse(u32b, u32e, grammar, skipper, iniSettings);
	bool r = phrase_parse(begin, end, grammar, skipper, iniSettings);

	if (!r || begin != end || !errorBuffer.Empty())  // This might not work correctly.
		throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, file.string());

	ApplyIniSettings();
}

void Settings::Save(const fs::path file, const std::uint32_t currentGameId) {
	// MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
	std::ofstream ini(file.string(), std::ios_base::trunc);
	if (ini.fail())
		throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
	ini << '\xEF' << '\xBB' << '\xBF'  // Write UTF-8 BOM to ensure the file is recognised as having the UTF-8 encoding.
	    << "#---------------" << std::endl
	    << "# BOSS Ini File" << std::endl
	    << "#---------------" << std::endl
	    << bloc::translate("# Settings with names starting with 'b' are boolean and accept values of 'true' or 'false'.") << std::endl
	    << bloc::translate("# Settings with names starting with 'i' are unsigned integers and accept varying ranges of whole numbers.") << std::endl
	    << bloc::translate("# Settings with names starting with 's' are strings and their accepted values vary.") << std::endl
	    << bloc::translate("# See the BOSS ReadMe for details on what each setting does and the accepted values for integer and string settings.") << std::endl << std::endl

	    << "[General Settings]" << std::endl
	    << "bUseUserRulesManager    = " << BoolToString(gl_use_user_rules_manager) << std::endl
	    << "bCloseGUIAfterSorting   = " << BoolToString(gl_close_gui_after_sorting) << std::endl
	    << "sLanguage               = " << GetLanguageString() << std::endl << std::endl

	    << "[Run Options]" << std::endl
	    << "sGame                   = " << GetIniGameString(gl_game) << std::endl
	    << "sLastGame               = " << GetIniGameString(currentGameId) << std::endl  // Writing current game because that's what we want recorded when BOSS writes the ini.
	    << "sBOSSLogFormat          = " << GetLogFormatString() << std::endl
	    << "iDebugVerbosity         = " << IntToString(gl_debug_verbosity) << std::endl
	    << "iRevertLevel            = " << IntToString(gl_revert) << std::endl
	    << "bUpdateMasterlist       = " << BoolToString(gl_update) << std::endl
	    << "bOnlyUpdateMasterlist   = " << BoolToString(gl_update_only) << std::endl
	    << "bSilentRun              = " << BoolToString(gl_silent) << std::endl
	    << "bDisplayCRCs            = " << BoolToString(gl_show_CRCs) << std::endl
	    << "bDoTrialRun             = " << BoolToString(gl_trial_run) << std::endl << std::endl

	    << "[Repository URLs]" << std::endl
	    << "sOblivionRepoURL        = " << gl_oblivion_repo_url << std::endl
	    << "sNehrimRepoURL          = " << gl_nehrim_repo_url << std::endl
	    << "sSkyrimRepoURL          = " << gl_skyrim_repo_url << std::endl
	    << "sFallout3RepoURL        = " << gl_fallout3_repo_url << std::endl
	    << "sFalloutNVRepoURL       = " << gl_falloutnv_repo_url << std::endl;
	ini.close();
}

ParsingError Settings::ErrorBuffer() const {
	return errorBuffer;
}

void Settings::ErrorBuffer(const ParsingError buffer) {
	errorBuffer = buffer;
}

std::string Settings::GetValue(const std::string setting) const {
	boost::unordered_map<std::string, std::string>::const_iterator it = iniSettings.find(setting);
	if (it != iniSettings.end())
		return it->second;
	return "";
}

std::string Settings::GetIniGameString(const std::uint32_t game) const {
	// TODO(MCP): Change this to a switch-statement
	if (game == AUTODETECT)
		return "auto";
	else if (game == OBLIVION)
		return "Oblivion";
	else if (game == FALLOUT3)
		return "Fallout3";
	else if (game == NEHRIM)
		return "Nehrim";
	else if (game == FALLOUTNV)
		return "FalloutNV";
	else if (game == SKYRIM)
		return "Skyrim";
	return "";
}

std::string Settings::GetLogFormatString() const {
	// TODO(MCP): Change this to a switch-statement, too?
	if (gl_log_format == HTML)
		return "html";
	return "text";
}

std::string Settings::GetLanguageString() const {
	// TODO(MCP): Change this to a switch-statement
	if (gl_language == ENGLISH)
		return "english";
	else if (gl_language == SPANISH)
		return "spanish";
	else if (gl_language == GERMAN)
		return "german";
	else if (gl_language == RUSSIAN)
		return "russian";
	else if (gl_language == SIMPCHINESE)
		return "chinese";
	return "";
}

void Settings::ApplyIniSettings() {
	// MCP Note: Change this to a range-based for-loop?
	for (boost::unordered_map<std::string, std::string>::iterator iter = iniSettings.begin(); iter != iniSettings.end(); ++iter) {
		if (iter->second.empty())
			continue;

		// String settings.
		if (iter->first == "sBOSSLogFormat") {
			if (iter->second == "html")
				gl_log_format = HTML;
			else
				gl_log_format = PLAINTEXT;
		} else if (iter->first == "sGame") {
			if (iter->second == "auto")
				gl_game = AUTODETECT;
			else if (iter->second == "Oblivion")
				gl_game = OBLIVION;
			else if (iter->second == "Nehrim")
				gl_game = NEHRIM;
			else if (iter->second == "Fallout3")
				gl_game = FALLOUT3;
			else if (iter->second == "FalloutNV")
				gl_game = FALLOUTNV;
			else if (iter->second == "Skyrim")
				gl_game = SKYRIM;
		} else if (iter->first == "sLastGame") {
			if (iter->second == "auto")
				gl_last_game = AUTODETECT;
			else if (iter->second == "Oblivion")
				gl_last_game = OBLIVION;
			else if (iter->second == "Nehrim")
				gl_last_game = NEHRIM;
			else if (iter->second == "Fallout3")
				gl_last_game = FALLOUT3;
			else if (iter->second == "FalloutNV")
				gl_last_game = FALLOUTNV;
			else if (iter->second == "Skyrim")
				gl_last_game = SKYRIM;
		} else if (iter->first == "sLanguage") {
			if (iter->second == "english")
				gl_language = ENGLISH;
			else if (iter->second == "spanish")
				gl_language = SPANISH;
			else if (iter->second == "german")
				gl_language = GERMAN;
			else if (iter->second == "russian")
				gl_language = RUSSIAN;
			else if (iter->second == "chinese")
				gl_language = SIMPCHINESE;
		} else if (iter->first == "sOblivionRepoURL") {
			gl_oblivion_repo_url = iter->second;
		} else if (iter->first == "sNehrimRepoURL") {
			gl_nehrim_repo_url = iter->second;
		} else if (iter->first == "sSkyrimRepoURL") {
			gl_skyrim_repo_url = iter->second;
		} else if (iter->first == "sFallout3RepoURL") {
			gl_fallout3_repo_url = iter->second;
		} else if (iter->first == "sFalloutNVRepoURL") {
			gl_falloutnv_repo_url = iter->second;
		// Now integers.
		} else if (iter->first == "iRevertLevel") {
			std::uint32_t value = std::atoi(iter->second.c_str());
			if (value >= 0 && value < 3)
				gl_revert = value;
		} else if (iter->first == "iDebugVerbosity") {
			std::uint32_t value = std::atoi(iter->second.c_str());
			if (value >= 0 && value < 4)
				gl_debug_verbosity = value;
			// Now on to boolean settings.
		} else if (iter->first == "bUseUserRulesEditor") {
			gl_use_user_rules_manager = StringToBool(iter->second);
		} else if (iter->first == "bCloseGUIAfterSorting") {
			gl_close_gui_after_sorting = StringToBool(iter->second);
		} else if (iter->first == "bUpdateMasterlist") {
			gl_update = StringToBool(iter->second);
		} else if (iter->first == "bOnlyUpdateMasterlist") {
			gl_update_only = StringToBool(iter->second);
		} else if (iter->first == "bSilentRun") {
			gl_silent = StringToBool(iter->second);
		} else if (iter->first == "bDisplayCRCs") {
			gl_show_CRCs = StringToBool(iter->second);
		} else if (iter->first == "bDoTrialRun") {
			gl_trial_run = StringToBool(iter->second);
		}
	}
}

}  // namespace boss
