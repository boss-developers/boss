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
#include "Common/Lists.h"

namespace boss {
	namespace fs = boost::filesystem;
	using namespace std;

	struct summaryCounters {
		int recognised; 
		int unrecognised;
		int ghosted;
		int messages;
		int warnings;
		int errors;
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
	};

	//Record recognised mod list from last HTML BOSSlog generated.
	string GetOldRecognisedList(fs::path log);

	//Detect the game BOSS is installed for.
	//1 = Oblivion, 2 = Fallout 3, 3 = Nehrim, 4 = Fallout: New Vegas, 5 = Skyrim. Throws exception if error.
	void GetGame();

	//Gets the string representation of the detected game.
	string GetGameString();

	//Gets the timestamp of the game's master file. Throws exception if error.
	time_t GetMasterTime();

	//Create a modlist containing all the mods that are installed or referenced in the userlist with their masterlist messages.
	//Returns the vector position of the last recognised mod in modlist.
	size_t BuildWorkingModlist(vector<item>& modlist, vector<item> masterlist, const vector<rule>& userlist);

	//Applies the userlist rules to the working modlist.
	void ApplyUserRules(vector<item>& modlist, const vector<rule>& userlist, string& ouputBuffer, size_t& lastRecognisedPos);

	//Lists Script Extender plugin info in the output buffer. Returns the Script Extender detected.
	string GetSEPluginInfo(string& outputBuffer);

	//Sort recognised mods.
	void SortRecognisedMods(const vector<item>& modlist, const size_t lastRecognisedPos, string& ouputBuffer, const time_t esmtime, summaryCounters& counters);

	//List unrecognised mods.
	void ListUnrecognisedMods(const vector<item>& modlist, const size_t lastRecognisedPos, string& ouputBuffer, const time_t esmtime, summaryCounters& counters);

	//Prints the full BOSSlog.
	void PrintBOSSlog(const bosslogContents contents, const summaryCounters counters, const string scriptExtender);
}
#endif