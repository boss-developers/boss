/*	Dirty Mod List Generator

    Outputs a list of dirty mods and their ITM/UDR counts and CRCs,
	using information from the Better Oblivion Sorting Software masterlist.

	Written using code adapted from Better Oblivion Sorting Software.
	All code (new and adapted), authored by WrinklyNinja.

    Copyright (C) 2011  WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2512 $, $Date: 2011-04-01 10:38:36 +0100 (Fri, 01 Apr 2011) $
*/

//Contains functions for modlist/masterlist parsing.

#include <fstream>
#include <string>
#include "Parser.h"
#include "Modlist/Grammar.h"
#include "Common/Helpers.h"

namespace boss {
	using namespace std;
	namespace qi = boost::spirit::qi;

	//Parses the given masterlist into the given data structure. Also works for the modlist.
	bool parseMasterlist(fs::path file, vector<item>& modList) {
		Skipper<string::const_iterator> skipper;
		modlist_grammar<string::const_iterator> grammar;
		string::const_iterator begin, end;
		string contents;

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();
		bool r = qi::phrase_parse(begin, end, grammar, skipper, modList);

		 if (r && begin == end)
			 return true;
		 else
			 return false;  //For some reason this isn't returning false when the parser fails.
		 //More acturately, when the parser fails, it executes the failure function, then just keeps going.
	}
}