/*	Dirty Mod List Generator

    Outputs a list of dirty mods and their ITM/UDR counts and CRCs,
	using information from the Better Oblivion Sorting Software masterlist.

	Written using code adapted from Better Oblivion Sorting Software.
	All code (new and adapted), authored by WrinklyNinja.

    Copyright (C) 2011  WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2512 $, $Date: 2011-04-01 10:38:36 +0100 (Fri, 01 Apr 2011) $
*/

#define NOMINMAX // we don't want the dummy min/max macros since they overlap with the std:: algorithms

#include "Main.h"
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <clocale>
#include <string>
#include <iostream>
#include <algorithm>

using namespace boss;
using namespace std;

int main() {
	ofstream dirtylist;
	vector<item> Masterlist;
	const fs::path masterlist_path		= "masterlist.txt";
	const fs::path dirtylist_path		= "dirtylist.txt";

	//Set the locale to get encoding conversions working correctly.
	//Not sure if this is still needed, but better safe than sorry.
	setlocale(LC_CTYPE, "");
	locale global_loc = locale();
	locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	//Open output file.
	dirtylist.open(dirtylist_path.c_str());
	if (dirtylist.fail()) {
		cout << "Critical Error: \"" +dirtylist_path.string() +"\" cannot be opened for writing! Exiting." << endl;
	}

	//Check if it actually exists, because the parser doesn't fail if there is no file...
	if (!fs::exists(masterlist_path)) {
		//Print error message to console and exit.
		cout << "Critical Error: \"" +masterlist_path.string() +"\" cannot be read! Exiting." << endl;
        exit (1); //fail in screaming heap.
    }
	//Parse masterlist into data structure.
	parseMasterlist(masterlist_path,Masterlist);
	//Check if parsing failed - the parsed bool always returns true for some reason, so check size of errorMessageBuffer.
	if (errorMessageBuffer.size() != 0) {
		cout << "There were errors encountered when parsing the masterlist. Try running BOSS using the same masterlist to see what the errors were. Exiting." << endl;
        exit (1); //fail in screaming heap.
	}

	//Sort mods alphabetically.
	sort(Masterlist.begin(),Masterlist.end(), SortModsByName);
	//Output formatted dirtylist.txt.
	SaveModlist(Masterlist, dirtylist_path);
	return (0);
}