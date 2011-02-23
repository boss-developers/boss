/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//File parser to handle BOSS masterlist/modlist. Working the way up from basics though.

#include <fstream>
#include <vector>

#include "Parser.h"
#include "Grammar.h"

namespace boss {
	using namespace std;
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	void fileToBuffer(fs::path file, string& buffer) {
		ifstream ifile(file.c_str());
		if (ifile.fail())
			return;
		ifile.unsetf(ios::skipws); // No white space skipping!
		copy(
			istream_iterator<char>(ifile),
			istream_iterator<char>(),
			back_inserter(buffer)
		);
	}

	bool parseUserlist(fs::path file, userlist& list) {
		userlist_skipper<string::const_iterator> skipper;
		userlist_grammar<string::const_iterator> grammar;
		string::const_iterator begin, end;
		string contents;

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();
		bool r = qi::phrase_parse(begin, end, grammar, skipper, list.rules);

		 if (r && begin == end)
			 return true;
		 else
			 return false;
	}

		//Rule checker function. If a rule breaks the RULES, remove it somehow and print an error message.
	void RuleCheck(rule ruleToCheck) {
		//This check should be case insensitive. The required function isn't available ATM though.
		if (ruleToCheck.ruleObject.find(".esp") != string::npos || ruleToCheck.ruleObject.find(".esm") != string::npos) {
			//Rule object is a mod. 
			//Check if it exists. If not, I AM ERROR.
			if (!fs::exists(ruleToCheck.ruleObject) && !fs::exists(ruleToCheck.ruleObject + ".ghost")) {
				cout << "ERROR: The mod: \"" << ruleToCheck.ruleObject << "\" does not exist." << endl;
				//Throw a proper error once handling is in.
			//Check that the mod isn't one of the main master files. If it is, ASK ERROR OF RUTO ABOUT THE PROBLEM.
			} else if (ruleToCheck.ruleObject == "oblivion.esm") { //This should be using the general function.
				cout << "ERROR: The game's main master file cannot be sorted." << endl;
				//Throw a proper error once handling is in.
			}
		} else {
			//Rule object is a group.
			//Check if the group is "ESMs". If it is, you know the drill.
			//This should be case-insensitive. The required function isn't available ATM though.
			if (ruleToCheck.ruleObject == "ESMs") {
				cout << "ERROR: The group 'ESMs' cannot be the subject of a rule." << endl;
				//Throw a proper error once handling is in.
			}
			if (ruleToCheck.ruleKey == ADD) {
				cout << "ERROR: Groups cannot be added by a rule." << endl;
				//Throw a proper error once handling is in.
			} else if (ruleToCheck.ruleKey == boss::FOR) {
				cout << "ERROR: Groups cannot have messages attached to them." << endl;
				//Throw a proper error once handling is in.
			}
		}
		for (size_t i=0; i<ruleToCheck.lines.size(); i++) {
			//Does the rule try sorting a mod before the main master file or a group before ESMs?
			if (ruleToCheck.lines[i].key == BEFORE) {
				if (ruleToCheck.lines[i].object == "ESMs") { //This should be case-insensitive.
					cout << "ERROR: Groups cannot be sorted before the group 'ESMs'." << endl;
					//Throw a proper error once handling is in.
				} else if (ruleToCheck.lines[i].object == "oblivion.esm") { //This should be using the general function.
					cout << "ERROR: Mods cannot be sorted before the game's main master file." << endl;
					//Throw a proper error once handling is in.
				}
			} else if (ruleToCheck.lines[i].key == TOP) {
				if (ruleToCheck.lines[i].object == "ESMs") { //This should be case-insensitive.
					cout << "ERROR: Mods cannot be inserted into the top of the group 'ESMs'." << endl;
					//Throw a proper error once handling is in.
				}
			}
		}
	}
}