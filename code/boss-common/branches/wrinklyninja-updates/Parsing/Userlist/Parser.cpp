/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//File parser to handle BOSS masterlist/modlist. Working the way up from basics though.

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "Parser.h"

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

void fileToBuffer(fs::path file, std::string& buffer) {
	std::ifstream ifile(file.c_str());
	if (ifile.fail())
		return;
	ifile.unsetf(std::ios::skipws); // No white space skipping!
	std::copy(
		std::istream_iterator<char>(ifile),
		std::istream_iterator<char>(),
		std::back_inserter(buffer)
	);
}

bool parseUserlist(fs::path file, boss::userlist& list) {
	typedef std::string::const_iterator iterator;
    userlist_grammar<iterator> grammar;
	userlist_skipper<iterator> skipper;
	iterator begin, end;
	std::string contents;

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
void RuleCheck(boss::rule rule) {
	//This check should be case insensitive. The required function isn't available ATM though.
	if (rule.ruleObject.find(".esp") != std::string::npos || rule.ruleObject.find(".esm") != std::string::npos) {
		//Rule object is a mod. 
		//Check if it exists. If not, I AM ERROR.
		if (!fs::exists(rule.ruleObject) && !fs::exists(rule.ruleObject + ".ghost")) {
			std::cout << "ERROR: The mod: \"" << rule.ruleObject << "\" does not exist." << std::endl;
			//Throw a proper error once handling is in.
		//Check that the mod isn't one of the main master files. If it is, ASK ERROR OF RUTO ABOUT THE PROBLEM.
		} else if (rule.ruleObject == "oblivion.esm") { //This should be using the general function.
			std::cout << "ERROR: The game's main master file cannot be sorted." << std::endl;
			//Throw a proper error once handling is in.
		}
	} else {
		//Rule object is a group.
		//Check if the group is "ESMs". If it is, you know the drill.
		//This should be case-insensitive. The required function isn't available ATM though.
		if (rule.ruleObject == "ESMs") {
			std::cout << "ERROR: The group 'ESMs' cannot be the subject of a rule." << std::endl;
			//Throw a proper error once handling is in.
		}
		if (rule.ruleKey == boss::ADD) {
			std::cout << "ERROR: Groups cannot be added by a rule." << std::endl;
			//Throw a proper error once handling is in.
		} else if (rule.ruleKey == boss::FOR) {
			std::cout << "ERROR: Groups cannot have messages attached to them." << std::endl;
			//Throw a proper error once handling is in.
		}
	}
	for (size_t i=0; i<rule.lines.size(); i++) {
		//Does the rule try sorting a mod before the main master file or a group before ESMs?
		if (rule.lines[i].key == boss::BEFORE) {
			if (rule.lines[i].object == "ESMs") { //This should be case-insensitive.
				std::cout << "ERROR: Groups cannot be sorted before the group 'ESMs'." << std::endl;
				//Throw a proper error once handling is in.
			} else if (rule.lines[i].object == "oblivion.esm") { //This should be using the general function.
				std::cout << "ERROR: Mods cannot be sorted before the game's main master file." << std::endl;
				//Throw a proper error once handling is in.
			}
		} else if (rule.lines[i].key == boss::TOP) {
			if (rule.lines[i].object == "ESMs") { //This should be case-insensitive.
				std::cout << "ERROR: Mods cannot be inserted into the top of the group 'ESMs'." << std::endl;
				//Throw a proper error once handling is in.
			}
		}
	}
}
/*
int main() {
	typedef std::string::const_iterator iterator;
    userlist_grammar<iterator> grammar;
	userlist_skipper<iterator> skipper;
	iterator begin, end;
	std::string contents,str;

    std::cout << "/////////////////////////////////////////////////////////\n\n"
			  << "\t\tA userlist parser for Spirit...\n\n"
			  << "/////////////////////////////////////////////////////////\n\n";

    std::cout << "Specify a userlist file to parse.\n"
			  << "Type [q or Q] to quit\n\n";

    while (getline(std::cin, str))
    {
		boss::userlist rules;
        if (str.empty())
            break;

		if (!fs::exists(str)) {
			std::cout << "File does not exist." << std::endl;
		} else {

			bool r = parseUserlist(fs::path(str), rules);

			if (r) {
				std::cout << "-------------------------\n";
				std::cout << "Parsing succeeded\n";
				std::cout << "got: "; 
		
				for (std::vector<boss::line>::size_type i = 0; i < rules.size(); ++i) {
					std::cout << std::endl << rules[i].ruleKey << ":" << rules[i].ruleObject << std::endl;
					for (std::vector<boss::line>::size_type j = 0; j < rules[i].lines.size(); ++j)
						std::cout << rules[i].lines[j].key << ":"  << rules[i].lines[j].object << std::endl;
				}
				
				std::cout << "\n-------------------------\n";
			} else {
				std::cout << "-------------------------\n";
				std::cout << "Parsing failed\n";
				std::cout << "-------------------------\n";
			}
		}
    }

    std::cout << "Bye... :-) \n\n";
    return 0;
}*/