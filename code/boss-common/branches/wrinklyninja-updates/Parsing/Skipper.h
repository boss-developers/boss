/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Header file for skipper grammar definitions, ie. what the userlist and modlist parsers should silently skip.



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
				| comment
				| eof;

			spc = space - eol;

			comment	
				= lit("//") 
				>> *(char_ - eol);

			eof = *eol >> eoi;
		}

		qi::rule<Iterator> start;
		qi::rule<Iterator> spc;
		qi::rule<Iterator> comment;
		qi::rule<Iterator> eof;
	};
}
#endif