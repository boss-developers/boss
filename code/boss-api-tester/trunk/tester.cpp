/*	Better Oblivion Sorting Software

	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011    BOSS Development Team.

	This file is part of Better Oblivion Sorting Software.

    Better Oblivion Sorting Software is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    Better Oblivion Sorting Software is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Better Oblivion Sorting Software.  If not, see 
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
	uint8_t * mPath = reinterpret_cast<uint8_t *>("masterlist.txt");
	uint8_t * uPath = reinterpret_cast<uint8_t *>("userlist.txt");
	uint8_t * dPath = reinterpret_cast<uint8_t *>("../Data");

	uint32_t ret;
	ofstream out("API test.txt");

	out << "TESTING IsCompatibleVersion(...)" << endl;
	bool b = IsCompatibleVersion(1,9,1);
	if (b)
		out << '\t' << "API is compatible." << endl;
	else
		out << '\t' << "API is incompatible." << endl;

	out << "TESTING GetVersionString(...)" << endl;
	const uint8_t * ver;
	ret = GetVersionString(&ver);
	if (ret != BOSS_API_ERROR_OK)
		out << '\t' << "Failed to get version string. Error: " << ret << endl;
	else
		out << '\t' << "Version: " << *ver << endl;
	
	out << "TESTING UpdateMasterlist(...)" << endl;
	ret = UpdateMasterlist(BOSS_API_GAME_OBLIVION, mPath);
	if (ret != BOSS_API_ERROR_OK)
		out << '\t' << "Masterlist update failed. Error: " << ret << endl;
	
	out << "TESTING CreateBossDb(...)" << endl;
	ret = CreateBossDb(&db);
	if (ret != BOSS_API_ERROR_OK) 
		out << '\t' << "Creation failed. Error: " << ret << endl;
	else {
		out << "TESTING Load(...)" << endl;
		ret = Load(db, mPath, uPath, dPath, BOSS_API_GAME_OBLIVION);
		if (ret != BOSS_API_ERROR_OK)
			out << '\t' << "Loading failed. Error: " << ret << endl;
		else {
			out << "TESTING EvalConditionals(...)" << endl;
			ret = EvalConditionals(db, dPath);
			if (BOSS_API_ERROR_OK != ret)
				out << '\t' << "Conditional evaluation failed. Error: " << ret << endl;
			else {
				
				out << "TESTING SortMods(...)" << endl;
				size_t lastPos;
				ret = SortMods(db, &lastPos);
				if (BOSS_API_ERROR_OK != ret)
					out << '\t' << "Sorting failed. Error: " << ret << endl;
				else
					out << '\t' << "Last recognised pos: " << lastPos << endl;
				
				out << "TESTING TrialSortMods(...)" << endl;
				uint8_t ** sortedPlugins;
				size_t len;
				ret = TrialSortMods(db, &sortedPlugins, &len, &lastPos);
				if (BOSS_API_ERROR_OK != ret)
					out << '\t' << "Trial sorting failed. Error: " << ret << endl;
				else {
					out << '\t' << "List size: " << len << endl;
					out << '\t' << "Last recognised pos: " << lastPos << endl;
					for (size_t i=0; i<len; i++) {
						out << '\t' << '\t' << i << " : " << sortedPlugins[i] << endl;
					}
				}
				
				out << "TESTING GetDirtyMessage(...)" << endl;
				uint8_t * message;
				const uint8_t * mod = reinterpret_cast<uint8_t *>("Hammerfell.esm");
				uint32_t toClean;
				//This should give no dirty message.
				ret = GetDirtyMessage(db, mod, &message, &toClean);
				if (ret != BOSS_API_ERROR_OK)
					out << '\t' << "Failed to get dirty info on \"Hammerfell.esm\". Error no " << ret << endl;
				else {
					out << '\t' << "\"Hammerfell.esm\" clean status: " << toClean << endl;
					if (message != NULL) {
						out << '\t' << "Message: " << message << endl;
					}
				}
				
				out << "TESTING GetDirtyMessage(...)" << endl;
				//Now try getting dirty message from one that will have one.
				const uint8_t * mod2 = reinterpret_cast<uint8_t *>("Oscuro's_Oblivion_Overhaul.esm");
				ret = GetDirtyMessage(db, mod2, &message, &toClean);
				if (ret != BOSS_API_ERROR_OK)
					out << '\t' << "Failed to get dirty info on \"Oscuro's_Oblivion_Overhaul.esm\". Error no " << ret << endl;
				else {
					out << '\t' << "\"Oscuro's_Oblivion_Overhaul.esm\" clean status: " << toClean << endl;
					if (message != NULL) {
						out << '\t' << "Message: " << message << endl;
					}
				}
				
				out << "TESTING DumpMinimal(...)" << endl;
				const uint8_t * file = reinterpret_cast<uint8_t *>("minimal.txt");
				uint32_t ret = DumpMinimal(db, file, true);
				if (ret != BOSS_API_ERROR_OK)
					out << '\t' << "Dump failed. Error no " << ret << endl;
				
				out << "TESTING GetBashTagMap(...)" << endl;
				BashTag * BTmap;
				size_t size;
				ret = GetBashTagMap(db, &BTmap, &size);
				if (ret != BOSS_API_ERROR_OK)
					out << '\t' << "Failed to get Bash Tag map. Error no " << ret << endl;
				else {
					out << '\t' << "Tag map size: " << size << endl;
					out << '\t' << "Bash Tags:" << endl;
					for (size_t i=0; i<size; i++)
						out << '\t' << '\t' << BTmap[i].id << " : " << BTmap[i].name << endl;
				}
				
				out << "TESTING GetModBashTags(...)" << endl;
				const uint8_t * mod3 = reinterpret_cast<uint8_t *>("Oscuro's_Oblivion_Overhaul.esm");
				size_t numTagsAdded, numTagsRemoved;
				bool modified;
				uint32_t * tagIDsAdded, * tagIDsRemoved;
				ret = GetModBashTags(db, mod3, &tagIDsAdded, &numTagsAdded, &tagIDsRemoved, &numTagsRemoved, &modified);
				if (ret != BOSS_API_ERROR_OK)
					out << '\t' << "Failed to get Bash Tags for \"Oscuro's_Oblivion_Overhaul.esm\". Error no " << ret << endl;
				else {
					out << '\t' << "Tags for \"Oscuro's_Oblivion_Overhaul.esm\":" << endl
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
			}
		}

		DestroyBossDb(db);
	}

	out.close();
	return 0;
}