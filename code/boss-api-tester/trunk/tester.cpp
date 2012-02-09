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

#include <string>
#include <iostream>
#include <vector>
#include <stdint.h>

#include "BOSS-API.h"

using namespace std;

int main() {

	boss_db db;
	uint8_t * mPath = reinterpret_cast<uint8_t *>("masterlist.txt");
	uint8_t * uPath = reinterpret_cast<uint8_t *>("userlist.txt");
	uint8_t * dPath = reinterpret_cast<uint8_t *>("../Data");

	uint32_t ret;

	bool b = IsCompatibleVersion(1,9,1);
	if (b)
		cout << "API is compatible." << endl;
	else
		cout << "API is incompatible." << endl;

	const uint8_t ** ver;
	ret = GetVersionString(ver);
	if (ret != BOSS_API_ERROR_OK)
		cout << "Failed to get version string." << endl;
	else
		cout << "Version: " << *ver << endl;

	ret = UpdateMasterlist(BOSS_API_GAME_OBLIVION, mPath);
	if (ret != BOSS_API_ERROR_OK)
		cout << "Masterlist update failed. Error: " << ret << endl;

	if (BOSS_API_ERROR_OK != CreateBossDb(&db)) 
		cout << "Creation failed." << endl;
	else {
		ret = Load(db, mPath, uPath, dPath, BOSS_API_GAME_OBLIVION);
		if (BOSS_API_ERROR_OK != ret)
			cout << "Loading failed." << endl;
		else {
			ret = EvalConditionals(db, dPath);
			if (BOSS_API_ERROR_OK != ret)
				cout << "Conditional evaluation failed. Error: " << ret << endl;
			else {

				ret = SortMods(db);
				if (BOSS_API_ERROR_OK != ret)
				cout << "Sorting failed. Error: " << ret << endl;

				const uint8_t ** sortedPlugins;
				size_t * len;
				ret = TrialSortMods(db, sortedPlugins, len);  //Symbol error.
				if (BOSS_API_ERROR_OK != ret)
					cout << "Trial sorting failed. Error: " << ret << endl;
				else {
					cout << "List size: " << *len << endl;
					const uint8_t * sp = *sortedPlugins;
					for (size_t i=0; i<*len; i++) {
						cout << sp[i] << endl;
					}
				}

/*				const uint8_t ** message;
				const uint8_t * mod = reinterpret_cast<uint8_t *>("Hammerfell.esm");
				uint32_t * toClean;
				ret = GetDirtyMessage(db, mod, message, toClean);  //Should be none for Hammerfell.
				if (ret != BOSS_API_ERROR_OK)
					cout << "Failed to get dirty info on \"Hammerfell.esm\". Error no " << ret << endl;
				else {
					cout << "\"Hammerfell.esm\" clean status: " << *toClean << endl;
					if (*message != NULL) {
						cout << "Message: " << *message << endl;
					}
				}

				//Now try getting dirty message from one that will have one.
				const uint8_t * mod2 = reinterpret_cast<uint8_t *>("Oscuro's_Oblivion_Overhaul.esm");
				ret = GetDirtyMessage(db, mod2, message, toClean);  //Should be none for Hammerfell.
				if (ret != BOSS_API_ERROR_OK)
					cout << "Failed to get dirty info on \"Oscuro's_Oblivion_Overhaul.esm\". Error no " << ret << endl;
				else {
					cout << "\"Oscuro's_Oblivion_Overhaul.esm\" clean status: " << *toClean << endl;
					if (*message != NULL) {
						cout << "Message: " << *message << endl;
					}
				}
*/
				const uint8_t * file = reinterpret_cast<uint8_t *>("minimal.txt");
				uint32_t ret = DumpMinimal(db, file, true);
				if (ret != BOSS_API_ERROR_OK)
					cout << "Dump failed. Error no " << ret << endl;
/*
				BashTag ** BTmap;
				size_t * size;
				ret = GetBashTagMap(db, BTmap, size);
				if (ret != BOSS_API_ERROR_OK)
					cout << "Failed to get Bash Tag map. Error no " << ret << endl;
				else {
					cout << "Tag map size: " << *size << endl;
					BashTag * tagArr = *BTmap;
					cout << "Bash Tags:" << endl;
					for (size_t i=0; i<*size; i++) {
						cout << '\t' << tagArr[i].id << " : " << tagArr[i].name << endl;
					}
				}

				const uint8_t * mod3 = reinterpret_cast<uint8_t *>("Oscuro's_Oblivion_Overhaul.esm");
				size_t * numTagsAdded, * numTagsRemoved;
				bool * modified;
				uint32_t ** tagIDsAdded, ** tagIDsRemoved;
				ret = GetModBashTags(db, mod3, tagIDsAdded, numTagsAdded, tagIDsRemoved, numTagsRemoved, modified);
				if (ret != BOSS_API_ERROR_OK)
					cout << "Failed to get Bash Tags for \"Oscuro's_Oblivion_Overhaul.esm\". Error no " << ret << endl;
				else {
					cout << "Tags for \"Oscuro's_Oblivion_Overhaul.esm\":" << endl
						<< '\t' << "Tags modified by userlist: " << *modified << endl
						<< '\t' << "Number of tags added: " << *numTagsAdded << endl
						<< '\t' << "Number of tags removed: " << *numTagsRemoved << endl
						<< '\t' << "Tags added:" << endl;
					for (size_t i=0; i<*numTagsAdded; i++) {
						cout << '\t' << (*tagIDsAdded)[i] << endl;
					}
					cout << '\t' << "Tags removed:" << endl;
					for (size_t i=0; i<*numTagsRemoved; i++) {
						cout << '\t' << (*tagIDsRemoved)[i] << endl;
					}
				}
*/			}
		}
	}
	return 0;
}