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

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>

namespace boss {
	namespace unicode = boost::spirit::unicode;
	namespace qi = boost::spirit::qi;
    namespace karma = boost::spirit::karma;

	using namespace std;

	enum formatType {
		HTML,
		PLAINTEXT
	};

	enum outputElement {
		br,
		block,
		para,
		span,
		ul,
		li,
		link,
		b,
		i,
		end
	};

	enum outputClass {
		title,
		error,
		success,
		warn,
		version,
		ghosted,
		tags
	};

	//Prints a given message to the bosslog, using format-safe Output function below.
	void ShowMessage(ofstream &log, formatType format, message currentMessage);

	//Prints ouptut with formatting according to format.
	void Output(ofstream &log, formatType format, string text);

	//Prints header if format is HTML, else nothing.
	void OutputHeader(ofstream &log, formatType format);
}
#endif