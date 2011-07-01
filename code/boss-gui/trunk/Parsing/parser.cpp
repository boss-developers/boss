/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/


	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "Parsing/parser.h"
#include "Parsing/grammars.h"

namespace boss {
	using namespace std;
	namespace qi = boost::spirit::qi;

	//Parses the given ini file.
	bool parseIni(fs::path file) {
		Ini_Skipper<string::const_iterator> skipper;
		ini_grammar<string::const_iterator> grammar;
		string::const_iterator begin, end;
		string contents;

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();

		bool r = qi::phrase_parse(begin, end, grammar, skipper);

		if (r && begin == end)
			return true;
		else
			return false;
	}

	//Reads the given file into the given string buffer.
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
}