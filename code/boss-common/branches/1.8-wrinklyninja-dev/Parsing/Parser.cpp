/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains functions for userlist and modlist/masterlist parsing.

#include <fstream>
#include "Parser.h"
#include "Modlist/Grammar.h"
#include "Userlist/Grammar.h"
#include "Ini/Grammar.h"
#include "Support/Helpers.h"
#include "utf8/source/utf8.h"

namespace boss {
	using namespace std;
	namespace qi = boost::spirit::qi;
	namespace unicode = boost::spirit::unicode;

	//Parses userlist into the given data structure.
	bool parseUserlist(fs::path file, vector<rule>& ruleList) {
		Skipper<string::const_iterator> skipper;
		userlist_grammar<string::const_iterator> grammar;
		string::const_iterator begin, end;
		string contents;

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();

		bool r = qi::phrase_parse(begin, end, grammar, skipper, ruleList);

		if (r && begin == end)
			return true;
		else
			return false;
	}

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

	bool parseIni(fs::path file) {
		Skipper<string::const_iterator> skipper;
		//ini_grammar<string::const_iterator> grammar;
		string::const_iterator begin, end;
		string contents;

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();

		bool p;

		//bool r = qi::phrase_parse(begin, end, grammar, skipper, p);

		//if (r && begin == end)
		//	return true;
		//else
			return false;
	}

	//UTF-8 Validator
	bool ValidateUTF8File(fs::path file) {
		ifstream ifs(file.c_str());

		istreambuf_iterator<char> it(ifs.rdbuf());
		istreambuf_iterator<char> eos;

		if (!utf8::is_valid(it, eos))
			return false;
		else
			return true;
	}
}