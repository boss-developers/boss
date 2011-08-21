/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains functions for userlist and modlist/masterlist parsing.

#include <fstream>
#include "Parsing/Parser.h"
#include "Parsing/Grammar.h"
#include "source/utf8.h"

namespace boss {
	using namespace std;
	using boost::spirit::qi::phrase_parse;

	//Parses userlist into the given data structure.
	bool parseUserlist(fs::path file, vector<rule>& ruleList) {
		Skipper skipper;
		userlist_grammar grammar;
		string::const_iterator begin, end;
		string contents;

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();

		bool r = phrase_parse(begin, end, grammar, skipper, ruleList);

		if (r && begin == end)
			return true;
		else
			return false;
	}

	//Parses the given masterlist into the given data structure. Also works for the modlist.
	bool parseMasterlist(fs::path file, vector<item>& modList) {
		Skipper skipper;
		modlist_grammar grammar;
		string::const_iterator begin, end;
		string contents;

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();
		bool r = phrase_parse(begin, end, grammar, skipper, modList);

		 if (r && begin == end)
			 return true;
		 else
			 return false;  //For some reason this isn't returning false when the parser fails.
		 //More acturately, when the parser fails, it executes the failure function, then just keeps going.
	}

	//Parses the ini, applying its settings.
	bool parseIni(fs::path file) {
		Ini_Skipper skipper;
		ini_grammar grammar;
		string::const_iterator begin, end;
		string contents;

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();

		bool r = phrase_parse(begin, end, grammar, skipper);

		if (r && begin == end)
			return true;
		else
			return false;
	}

	//Reads an entire file into a string buffer.
	void fileToBuffer(const fs::path file, string& buffer) {
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