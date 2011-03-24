/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains the various functions/classes required for varied BOSSlog output formattings,etc.
//Still at the brainstorming stage.
//The idea is to separate the unformatted text from the formatting. Unformatted text is generated in main() and passed through something found
//here to be formatted appropriately.

#ifndef __BOSS_BOSSLOG_H__
#define __BOSS_BOSSLOG_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include <fstream>
#include <string>
#include "Lists.h"

#include <boost/spirit/include/lex_lexertl.hpp>

namespace boss {
	using namespace std;
	namespace unicode = boost::spirit::unicode;
	namespace lex = boost::spirit::lex;

	//Prints a given message to the bosslog, using format-safe Output function below.
	void ShowMessage(ofstream &log, string format, message currentMessage);

	//Prints ouptut with formatting according to format.
	void Output(ofstream &log, string format, string text);

	//Prints header if format is HTML, else nothing.
	void OutputHeader(ofstream &log, string format);

	//Converts an integer to a string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
	string IntToString(unsigned int n);

	//Converts an integer to a hex string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
	string IntToHexString(unsigned int n);

	/*enum token_ids
	{
		ID_WORD = 1000,
		ID_EOL,
		ID_CHAR
	};

	template <typename Lexer>
	struct word_count_tokens : lex::lexer<Lexer>
	{
		word_count_tokens()
		  : c(0), w(0), l(0)
		  , word("[^ \t\n]+")     // define tokens
		  , eol("\n")
		  , any(".")
		{
			using boost::spirit::lex::_start;
			using boost::spirit::lex::_end;
			using boost::phoenix::ref;

			// associate tokens with the lexer
			this->self 
				=   word  [++ref(w), ref(c) += distance(_start, _end)]
				|   eol   [++ref(c), ++ref(l)] 
				|   any   [++ref(c)]
				;
		}

		std::size_t c, w, l;
		lex::token_def<> word, eol, any;
	};*/


}
#endif