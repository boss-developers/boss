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

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#ifndef __BOSS_EXECUTE_H__
#define __BOSS_EXECUTE_H__

#include <string>
#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include "Common/Classes.h"
#include "Common/DllDef.h"

namespace boss {
	namespace fs = boost::filesystem;
	using namespace std;

	struct BOSS_COMMON summaryCounters {
		uint32_t recognised; 
		uint32_t unrecognised;
		uint32_t inactive;
		uint32_t messages;
		uint32_t warnings;
		uint32_t errors;

		summaryCounters();
	};

	struct BOSS_COMMON bosslogContents {
		string updater;
		string userlistMessages;
		string seInfo;
		string recognisedPlugins;
		string unrecognisedPlugins;

		string oldRecognisedPlugins;

		string criticalError;
		vector<ParsingError> parsingErrors;
		vector<Message> globalMessages;
	};

	//Record recognised mod list from last HTML BOSSlog generated.
	BOSS_COMMON string GetOldRecognisedList(const fs::path log);

	//Performs BOSS's main sorting functionality. Each stage is implemented by a separate function for neatness and to make future adjustments easier. 
	BOSS_COMMON void PerformSortingFunctionality(fs::path file,
												ItemList& modlist,
												ItemList& masterlist,
												RuleList& userlist,
												const time_t esmtime,
												bosslogContents contents);

	//Create a modlist containing all the mods that are installed or referenced in the userlist with their masterlist messages.
	//Returns the vector position of the last recognised mod in modlist.
	BOSS_COMMON void BuildWorkingModlist(ItemList& modlist, ItemList& masterlist, RuleList& userlist);

	//Applies the userlist rules to the working modlist.
	BOSS_COMMON void ApplyUserRules(ItemList& modlist, RuleList& userlist, string& outputBuffer);
}
#endif