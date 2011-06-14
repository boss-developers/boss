/*	Dirty Mod List Generator

    Outputs a list of dirty mods and their ITM/UDR counts and CRCs,
	using information from the Better Oblivion Sorting Software masterlist.

	Written using code adapted from Better Oblivion Sorting Software.
	All code (new and adapted), authored by WrinklyNinja.

    Copyright (C) 2011  WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2512 $, $Date: 2011-04-01 10:38:36 +0100 (Fri, 01 Apr 2011) $
*/

//Header file for skipper grammar definitions, ie. what the modlist parser should silently skip.

#ifndef __BOSS_PARSER_SKIPPER_H__
#define __BOSS_PARSER_SKIPPER_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include <boost/spirit/include/qi.hpp>

namespace boss {
	namespace unicode = boost::spirit::unicode;
	namespace qi = boost::spirit::qi;

	using qi::eol;
	using qi::eoi;
	using qi::lit;

	using unicode::char_;
	using unicode::space;
	
	template <typename Iterator>
	struct Skipper : qi::grammar<Iterator> {

		Skipper() : Skipper::base_type(start, "Skipper") {

			start = 
				spc
				| UTF8
				| comment
				| oldMasterlistComment
				| eof;
			
			spc = space - eol;

			UTF8 = char_("\xef") >> char_("\xbb") >> char_("\xbf"); //UTF8 BOM

			comment	= 
				lit("//") 
				>> *(char_ - eol);

			//Need to skip lines that start with '\', but only if they don't follow with EndGroup or BeginGroup.
			oldMasterlistComment = 
				lit("\\")
				>> !(lit("EndGroup") | lit("BeginGroup"))
				>> *(char_ - eol);

			eof = *(spc | comment | oldMasterlistComment | eol) >> eoi;
		}

		qi::rule<Iterator> start;
		qi::rule<Iterator> spc;
		qi::rule<Iterator> comment;
		qi::rule<Iterator> eof;
		qi::rule<Iterator> oldMasterlistComment;
		qi::rule<Iterator> UTF8;
	};
}
#endif