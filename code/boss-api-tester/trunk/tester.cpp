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
#include <stdint.h>

#include "BOSS-API.h"

using namespace std;

int main() {

	boss_db db;
	uint8_t * mPath = reinterpret_cast<uint8_t *>("masterlist.txt");
	uint8_t * uPath = reinterpret_cast<uint8_t *>("userlist.txt");
	uint8_t * dPath = reinterpret_cast<uint8_t *>("../Data");

	if (BOSS_API_ERROR_OK == Load(db, mPath, uPath, dPath)) {
		cout << "This works, hallelujah." << endl;

		uint8_t ** sortedPlugins;
	//	TrialSortMods(db, sortedPlugins, BOSS_API_GAME_OBLIVION);  //Symbol error.

		cout << *sortedPlugins << endl;

		const uint8_t ** message;
		const uint8_t * mod = reinterpret_cast<uint8_t *>("Hammerfell.esm");
		uint32_t * toClean;
		GetDirtyMessage(db,mod,message,toClean);

//		cout << *message << endl; //Causes a crash.

		const uint8_t * file = reinterpret_cast<uint8_t *>("testDump.txt");
		DumpMinimal(db,file,false);

	} else
		cout << "I'm afraid it's just not working, Dave." << endl;
	return 0;
}