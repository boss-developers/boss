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
	uint8_t * mPath = reinterpret_cast<uint8_t *>("Skyrim/masterlist.txt");
	uint8_t * uPath = reinterpret_cast<uint8_t *>("Skyrim/userlist.txt");
	uint8_t * dPath = reinterpret_cast<uint8_t *>("../Data");
	uint32_t game = BOSS_API_GAME_SKYRIM;

	const uint8_t * file = reinterpret_cast<uint8_t *>("minimal.txt");
	const uint8_t * cleanMod = reinterpret_cast<uint8_t *>("All Natural.esp");
	const uint8_t * doNotCleanMod = reinterpret_cast<uint8_t *>("bgBalancingEVLAMEAddition.esp");
	const uint8_t * inactiveMod = reinterpret_cast<uint8_t *>("français.esp");
	const uint8_t * isActiveMod = reinterpret_cast<uint8_t *>("汉语漢語.esp");
	uint8_t ** sortedPlugins;
	size_t len;
	size_t lastPos;
	uint8_t * message;
	size_t numTagsAdded, numTagsRemoved;
	bool modified;
	uint32_t * tagIDsAdded, * tagIDsRemoved;
	bool active;
	BashTag * BTmap;
	size_t size;
	uint32_t toClean;

	uint32_t ret;
	ofstream out("API test.txt");

	out << "TESTING IsCompatibleVersion(...)" << endl;
	bool b = IsCompatibleVersion(2,0,0);
	if (b)
		out << '\t' << "API is compatible." << endl;
	else
		out << '\t' << "API is incompatible." << endl;

	out << "TESTING GetVersionString(...)" << endl;
	const uint8_t * ver;
	ret = GetVersionString(&ver);
	if (ret != BOSS_API_OK)
		out << '\t' << "Failed to get version string. Error: " << ret << endl;
	else
		out << '\t' << "Version: " << *ver << endl;
	
	out << "TESTING CreateBossDb(...)" << endl;
	ret = CreateBossDb(&db, game, dPath);
	if (ret != BOSS_API_OK) 
		out << '\t' << "Creation failed. Error: " << ret << endl;
	else {
		out << "TESTING UpdateMasterlist(...)" << endl;
		ret = UpdateMasterlist(db, mPath);
		if (ret != BOSS_API_OK)
			out << '\t' << "Masterlist update failed. Error: " << ret << endl;

		out << "TESTING Load(...)" << endl;
		ret = Load(db, mPath, uPath);
		if (ret != BOSS_API_OK)
			out << '\t' << "Loading failed. Error: " << ret << endl;
		else {
			out << "TESTING EvalConditionals(...)" << endl;
			ret = EvalConditionals(db);
			if (BOSS_API_OK != ret)
				out << '\t' << "Conditional evaluation failed. Error: " << ret << endl;
			else {
				out << "TESTING SortMods(...)" << endl;
				ret = SortMods(db, false, &sortedPlugins, &len, &lastPos);
				if (BOSS_API_OK != ret)
					out << '\t' << "Sorting failed. Error: " << ret << endl;
				else {
					out << '\t' << "List size: " << len << endl;
					out << '\t' << "Last recognised pos: " << lastPos << endl;
					for (size_t i=0; i<len; i++) {
						out << '\t' << '\t' << i << " : " << sortedPlugins[i] << endl;
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
				
				out << "TESTING SetPluginActive(...)" << endl;
				ret = SetPluginActive(db, inactiveMod, true);
				if (BOSS_API_OK != ret)
					out << '\t' << "SetPluginActive(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "\"" << inactiveMod << "\" activated." << endl;
				}

				out << "TESTING IsPluginActive(...)" << endl;
				ret = IsPluginActive(db, inactiveMod, &active);
				if (BOSS_API_OK != ret)
					out << '\t' << "IsPluginActive(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "\"" << inactiveMod << "\" active status: " << active << endl;
				}
				
				out << "TESTING SetPluginActive(...)" << endl;
				ret = SetPluginActive(db, isActiveMod, true);
				if (BOSS_API_OK != ret)
					out << '\t' << "SetPluginActive(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "\"" << isActiveMod << "\" deactivated." << endl;
				}

				out << "TESTING IsPluginActive(...)" << endl;
				ret = IsPluginActive(db, isActiveMod, &active);
				if (BOSS_API_OK != ret)
					out << '\t' << "IsPluginActive(...) failed. Error: " << ret << endl;
				else {
					out << '\t' << "\"" << isActiveMod << "\" active status: " << active << endl;
				}
			}
		}

		DestroyBossDb(db);
	}

	out.close();
	return 0;
}