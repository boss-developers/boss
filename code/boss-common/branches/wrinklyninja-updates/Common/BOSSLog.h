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

#include <fstream>
#include <string>
#include "Lists.h"
#include <boost/spirit/include/karma.hpp>

namespace boss {
	using namespace std;
	namespace karma = boost::spirit::karma;

	//Prints a given message to the bosslog, using format-safe Output function below.
	void ShowMessage(ofstream &log, string format, message currentMessage);

	//Prints ouptut with formatting according to format.
	void Output(ofstream &log, string format, string text);

	//Prints header if format is HTML, else nothing.
	void OutputHeader(ofstream &log, string format);

	//Converts an integer to a string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
	string IntToString(unsigned int n);

	template <typename OutputIterator>
	struct bosslog_grammar : karma::grammar<OutputIterator, string()>
	{
		bosslog_grammar() : bosslog_grammar::base_type(text, "bosslog_grammar")
		{
			text %= +karma::char_;
		}

		karma::rule<OutputIterator, string()> text;
	};
}
#endif