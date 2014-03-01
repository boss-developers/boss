/*	BOSS

	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011    BOSS Development Team.

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

	$Revision: 1783 $, $Date: 2010-10-31 23:05:28 +0000 (Sun, 31 Oct 2010) $
*/

#include <iostream>
#include <stdint.h>
#include <fstream>
#include <clocale>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/filesystem.hpp>

#include "BOSS-API.h"

using namespace std;

int main() {
	//Set the locale to get encoding conversions working correctly.
	setlocale(LC_CTYPE, "");
	locale global_loc = locale();
	locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	boss_db db;
	uint8_t * mPath = reinterpret_cast<uint8_t *>("Oblivion/masterlist.txt");
	uint8_t * uPath = reinterpret_cast<uint8_t *>("");
	uint8_t * gPath = reinterpret_cast<uint8_t *>("..");
	uint32_t game = BOSS_API_GAME_OBLIVION;

	const uint8_t * file = reinterpret_cast<uint8_t *>("minimal.txt");
	const uint8_t * cleanMod = reinterpret_cast<uint8_t *>("All Natural Base.esm");
	const uint8_t * doNotCleanMod = reinterpret_cast<uint8_t *>("bgBalancingEVLAMEAddition.esp");
	const uint8_t * inactiveMod = reinterpret_cast<uint8_t *>("français.esp");
	const uint8_t * messageMod = reinterpret_cast<uint8_t *>("PerkUP-5555.esp");
	const uint8_t * isActiveMod = reinterpret_cast<uint8_t *>("汉语漢語.esp");
	const uint8_t * info = reinterpret_cast<uint8_t *>("Testing the API's SubmitUnrecognisedPlugin function.");
	uint8_t ** sortedPlugins;
	uint8_t ** unrecPlugins;
	size_t len;
	uint8_t * message;
	size_t numTagsAdded, numTagsRemoved;
	bool modified;
	uint32_t * tagIDsAdded, * tagIDsRemoved;
	bool active;
	BashTag * BTmap;
	size_t size;
	uint32_t toClean;
	BossMessage * messages;

	uint32_t ret;
	ofstream out("API test.txt");

	out << "TESTING IsCompatibleVersion(...)" << endl;
	bool b = IsCompatibleVersion(2,1,0);
	if (b)
		out << '\t' << "API is compatible." << endl;
	else {
		out << '\t' << "API is incompatible." << endl;
		exit(0);
	}

	out << "TESTING GetVersionString(...)" << endl;
	ret = GetVersionString(&message);
	if (ret != BOSS_API_OK)
		out << '\t' << "Failed to get version string. Error: " << ret << endl;
	else
		out << '\t' << "Version: " << *message << endl;
	
	out << "TESTING CreateBossDb(...)" << endl;
	ret = CreateBossDb(&db, game, NULL);
	if (ret != BOSS_API_OK) 
		out << '\t' << "Creation failed. Error: " << ret << endl;
	else {
		out << "TESTING UpdateMasterlist(...)" << endl;
		ret = UpdateMasterlist(db, mPath);
		if (ret != BOSS_API_OK)
			out << '\t' << "Masterlist update failed. Error: " << ret << endl;

		out << "TESTING SubmitUnrecognisedPlugin(...)" << endl;
		ret = SubmitUnrecognisedPlugin(db, cleanMod, NULL, info);
		if (ret != BOSS_API_OK)
			out << '\t' << "SubmitUnrecognisedPlugin(...) failed. Error: " << ret << endl;

		out << "TESTING Load(...)" << endl;
		ret = Load(db, mPath, NULL);
		if (ret != BOSS_API_OK) {
			uint32_t ret2 = GetLastErrorDetails(&message);
			out << '\t' << "Loading failed. Error: " << ret << ", " << ret2 << ", " << message << endl;
		} else {
			out << "TESTING EvalConditionals(...)" << endl;
			ret = EvalConditionals(db);
			if (BOSS_API_OK != ret)
				out << '\t' << "Conditional evaluation failed. Error: " << ret << endl;
			else {
				out << "TESTING SortMods(...)" << endl;
				ret = SortMods(db, false, &sortedPlugins, &len, &unrecPlugins, &size);
				if (BOSS_API_OK != ret)
					out << '\t' << "Sorting failed. Error: " << ret << endl;
				else {
					out << '\t' << "List size: " << len << endl;
					for (size_t i=0; i<len; i++) {
						out << '\t' << '\t' << i << " : " << sortedPlugins[i] << endl;
					}
					out << '\t' << "Unrecognised plugin list size: " << size << endl;
					for (size_t i=0; i<size; i++) {
						out << '\t' << '\t' << i << " : " << unrecPlugins[i] << endl;
					}
				}
				
				out << "TESTING GetDirtyMessage(...)" << endl;
				//This should give no dirty message.
				ret = GetDirtyMessage(db, cleanMod, &message, &toClean);
				if (ret != BOSS_API_OK)
					out << '\t' << "Failed to get dirty info on \"" << cleanMod << "\". Error no " << ret << endl;
				else {
					out << '\t' << "\"" << cleanMod << "\" clean status: " << toClean << endl;
					if (message != NULL) {
						out << '\t' << "Message: " << message << endl;
					}
				}
				
				out << "TESTING GetDirtyMessage(...)" << endl;
				//Now try getting dirty message from one that will have one.
				ret = GetDirtyMessage(db, doNotCleanMod, &message, &toClean);
				if (ret != BOSS_API_OK)
					out << '\t' << "Failed to get dirty info on \"" << doNotCleanMod << "\". Error no " << ret << endl;
				else {
					out << '\t' << "\"" << doNotCleanMod << "\" clean status: " << toClean << endl;
					if (message != NULL) {
						out << '\t' << "Message: " << message << endl;
					}
				}
				
				out << "TESTING DumpMinimal(...)" << endl;
				ret = DumpMinimal(db, file, true);
				if (ret != BOSS_API_OK)
					out << '\t' << "Dump failed. Error no " << ret << endl;
				
				out << "TESTING GetBashTagMap(...)" << endl;
				ret = GetBashTagMap(db, &BTmap, &size);
				if (ret != BOSS_API_OK)
					out << '\t' << "Failed to get Bash Tag map. Error no " << ret << endl;
				else {
					out << '\t' << "Tag map size: " << size << endl;
					out << '\t' << "Bash Tags:" << endl;
					for (size_t i=0; i<size; i++)
						out << '\t' << '\t' << BTmap[i].id << " : " << BTmap[i].name << endl;
				}
				
				out << "TESTING GetModBashTags(...)" << endl;
				ret = GetModBashTags(db, doNotCleanMod, &tagIDsAdded, &numTagsAdded, &tagIDsRemoved, &numTagsRemoved, &modified);
				if (ret != BOSS_API_OK)
					out << '\t' << "Failed to get Bash Tags for \"" << doNotCleanMod << "\". Error no " << ret << endl;
				else {
					out << '\t' << "Tags for \"" << doNotCleanMod << "\":" << endl
						<< '\t' << '\t' << "Modified by userlist: " << modified << endl
						<< '\t' << '\t' << "Number of tags added: " << numTagsAdded << endl
						<< '\t' << '\t' << "Number of tags removed: " << numTagsRemoved << endl
						<< '\t' << '\t' << "Tags added:" << endl;
					for (size_t i=0; i<numTagsAdded; i++)
						out << '\t' << '\t' << tagIDsAdded[i] << endl;
					out << '\t' << '\t' << "Tags removed:" << endl;
					for (size_t i=0; i<numTagsRemoved; i++)
						out << '\t' << '\t' << tagIDsRemoved[i] << endl;
				}

				out << "TESTING GetLoadOrder(...)" << endl;
				ret = GetLoadOrder(db, &sortedPlugins, &len);
				if (BOSS_API_OK != ret)
					out << '\t' << "GetLoadOrder(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "List size: " << len << endl;
					for (size_t i=0; i<len; i++) {
						out << '\t' << '\t' << i << " : " << sortedPlugins[i] << endl;
					}
				}
				
				out << "TESTING SetLoadOrder(...)" << endl;
				ret = SetLoadOrder(db, sortedPlugins, len);
				if (BOSS_API_OK != ret)
					out << '\t' << "SetLoadOrder(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "List size: " << len << endl;
					for (size_t i=0; i<len; i++) {
						out << '\t' << '\t' << i << " : " << sortedPlugins[i] << endl;
					}
				}

				out << "TESTING GetActivePlugins(...)" << endl;
				ret = GetActivePlugins(db, &sortedPlugins, &len);
				if (BOSS_API_OK != ret)
					out << '\t' << "GetActivePlugins(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "List size: " << len << endl;
					for (size_t i=0; i<len; i++) {
						out << '\t' << '\t' << i << " : " << sortedPlugins[i] << endl;
					}
				}
				
				out << "TESTING SetActivePlugins(...)" << endl;
				ret = SetActivePlugins(db, sortedPlugins, len);
				if (BOSS_API_OK != ret)
					out << '\t' << "SetActivePlugins(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "List size: " << len << endl;
					for (size_t i=0; i<len; i++) {
						out << '\t' << '\t' << i << " : " << sortedPlugins[i] << endl;
					}
				}
				
				out << "TESTING GetPluginLoadOrder(...)" << endl;
				ret = GetPluginLoadOrder(db, doNotCleanMod, &len);
				if (BOSS_API_OK != ret)
					out << '\t' << "GetPluginLoadOrder(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "\"" << doNotCleanMod << "\" position: " << len << endl;
				}
				
				out << "TESTING SetPluginLoadOrder(...)" << endl;
				len = 1;
				ret = SetPluginLoadOrder(db, doNotCleanMod, len);
				if (BOSS_API_OK != ret)
					out << '\t' << "SetPluginLoadOrder(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "\"" << doNotCleanMod << "\" set position: " << len << endl;
				}

				out << "TESTING GetIndexedPlugin(...)" << endl;
				len = 10;
				ret = GetIndexedPlugin(db, len, &message);
				if (BOSS_API_OK != ret)
					out << '\t' << "GetIndexedPlugin(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "Plugin at position " << len << " : " << message << endl;
				}

				out << "TESTING GetPluginMessages(...)" << endl;
				ret = GetPluginMessages(db, messageMod, &messages, &len);
				if (BOSS_API_OK != ret)
					out << '\t' << "GetIndexedPlugin(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "Message array size: " << len << endl;
					for (size_t i=0; i<len; i++)
						out << '\t' << '\t' << messages[i].type << " : " << messages[i].message << endl;
				}

				//Assume there are at least 3 plugins.
				out << "TESTING SetPluginActive(...)" << endl;
				ret = SetPluginActive(db, sortedPlugins[2], true);
				if (BOSS_API_OK != ret)
					out << '\t' << "SetPluginActive(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "\"" << sortedPlugins[2] << "\" activated." << endl;
				}
				
				//Assume there are at least 3 plugins.
				out << "TESTING IsPluginActive(...)" << endl;
				ret = IsPluginActive(db, sortedPlugins[2], &active);
				if (BOSS_API_OK != ret)
					out << '\t' << "IsPluginActive(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "\"" << sortedPlugins[2] << "\" active status: " << active << endl;
				}

				out << "TESTING IsPluginMaster(...)" << endl;
				ret = IsPluginMaster(db, cleanMod, &active);
				if (BOSS_API_OK != ret)
					out << '\t' << "IsPluginMaster(...) failed. Error: " << ret << endl;
				else if (active)
					out << '\t' << "\"" << cleanMod << "\" is a master." << endl;
				else
					out << '\t' << "\"" << cleanMod << "\" is not a master." << endl;

				out << "TESTING IsPluginMaster(...)" << endl;
				ret = IsPluginMaster(db, doNotCleanMod, &active);
				if (BOSS_API_OK != ret)
					out << '\t' << "IsPluginMaster(...) failed. Error: " << ret << endl;
				else if (active)
					out << '\t' << "\"" << doNotCleanMod << "\" is a master." << endl;
				else
					out << '\t' << "\"" << doNotCleanMod << "\" is not a master." << endl;
			}
		}

		DestroyBossDb(db);
	}

	out.close();
	return 0;
}