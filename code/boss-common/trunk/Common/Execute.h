/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011  Random/Random007/jpearce, WrinklyNinja & the BOSS 
	development team. Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#ifndef __BOSS_EXECUTE_H__
#define __BOSS_EXECUTE_H__

#include <string>
#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/regex.hpp>
#include "Common/Classes.h"
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

		string criticalError;
		string updaterErrors;
		string iniParsingError;
		string userlistParsingError;
		vector<string> userlistSyntaxErrors;
		vector<Message> globalMessages;
	};

	//Searches a hashset for the first matching string of a regex and returns its iterator position. Usage internal to BOSS-Common.
	BOSS_COMMON_EXP boost::unordered_set<string>::iterator FindRegexMatch(const boost::unordered_set<string> set, const boost::regex reg, boost::unordered_set<string>::iterator startPos);

	//Record recognised mod list from last HTML BOSSlog generated.
	BOSS_COMMON_EXP string GetOldRecognisedList(const fs::path log);

	//Detect the game BOSS is installed for. Returns an enum as defined in Globals.h.
	//Throws exception on fail.
	BOSS_COMMON_EXP void GetGame();

	//Gets the string representation of the detected game.
	BOSS_COMMON_EXP string GetGameString();

	//Gets the timestamp of the game's master file. Throws exception if error.
	//Throws exception on fail.
	BOSS_COMMON_EXP time_t GetMasterTime();

	//Performs BOSS's main sorting functionality. Each stage is implemented by a separate function for neatness and to make future adjustments easier. 
	BOSS_COMMON_EXP void PerformSortingFunctionality(fs::path file,
												ItemList& modlist,
												ItemList& masterlist,
												RuleList& userlist,
												const time_t esmtime,
												bosslogContents contents);

	//Create a modlist containing all the mods that are installed or referenced in the userlist with their masterlist messages.
	//Returns the vector position of the last recognised mod in modlist.
	BOSS_COMMON_EXP void BuildWorkingModlist(ItemList& modlist, ItemList& masterlist, RuleList& userlist);

	//Applies the userlist rules to the working modlist.
	BOSS_COMMON_EXP void ApplyUserRules(ItemList& modlist, RuleList& userlist, string& outputBuffer);
}
#endif