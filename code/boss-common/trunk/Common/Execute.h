/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#ifndef __BOSS_EXECUTE_H__
#define __BOSS_EXECUTE_H__

#include <string>
#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/regex.hpp>
#include "Common/Lists.h"
#include "Common/DllDef.h"

namespace boss {
	namespace fs = boost::filesystem;
	using namespace std;

	struct summaryCounters {
		unsigned int recognised; 
		unsigned int unrecognised;
		unsigned int ghosted;
		unsigned int messages;
		unsigned int warnings;
		unsigned int errors;
		summaryCounters();
	};

	struct bosslogContents {
		string generalMessages;
		string summary;
		string userlistMessages;
		string seInfo;
		string recognisedPlugins;
		string unrecognisedPlugins;

		string oldRecognisedPlugins;
		string updaterErrors;
		bosslogContents();
	};

	//Searches a hashset for the first matching string of a regex and returns its iterator position.
	BOSS_COMMON_EXP boost::unordered_set<string>::iterator FindRegexMatch(const boost::unordered_set<string> set, const boost::regex reg, boost::unordered_set<string>::iterator startPos);

	//Record recognised mod list from last HTML BOSSlog generated.
	BOSS_COMMON_EXP string GetOldRecognisedList(const fs::path log);

	//Detect the game BOSS is installed for.
	//1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas, 5 = Skyrim. Throws exception if error.
	BOSS_COMMON_EXP void GetGame();

	//Gets the string representation of the detected game.
	BOSS_COMMON_EXP string GetGameString();

	//Returns the expeccted master file.
	BOSS_COMMON_EXP string GameMasterFile();

	//Gets the timestamp of the game's master file. Throws exception if error.
	BOSS_COMMON_EXP time_t GetMasterTime();

	//Create a modlist containing all the mods that are installed or referenced in the userlist with their masterlist messages.
	//Returns the vector position of the last recognised mod in modlist.
	BOSS_COMMON_EXP size_t BuildWorkingModlist(vector<item>& modlist, vector<item> masterlist, const vector<rule>& userlist);

	//Applies the userlist rules to the working modlist.
	BOSS_COMMON_EXP void ApplyUserRules(vector<item>& modlist, const vector<rule>& userlist, string& ouputBuffer, size_t& lastRecognisedPos);

	//Lists Script Extender plugin info in the output buffer. Returns the Script Extender detected.
	BOSS_COMMON_EXP string GetSEPluginInfo(string& outputBuffer);

	//Sort recognised mods.
	BOSS_COMMON_EXP void SortRecognisedMods(const vector<item>& modlist, const size_t lastRecognisedPos, string& ouputBuffer, const time_t esmtime, summaryCounters& counters);

	//List unrecognised mods.
	BOSS_COMMON_EXP void ListUnrecognisedMods(const vector<item>& modlist, const size_t lastRecognisedPos, string& ouputBuffer, const time_t esmtime, summaryCounters& counters);

	//Prints the full BOSSlog.
	BOSS_COMMON_EXP void PrintBOSSlog(const bosslogContents contents, const summaryCounters counters, const string scriptExtender);
}
#endif