/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Header file for modlist grammar definition.

/* Starting with building a parser for the current format, and then working my way up to a parser for the new format.
*/

#ifndef __BOSS_USERLIST_GRAM_H__
#define __BOSS_USERLIST_GRAM_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE
#endif

#include "Parsing/Data.h"
#include "Parsing/Skipper.h"
#include "Parser.h"
#include <sstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

namespace boss {
	namespace unicode = boost::spirit::unicode;
	namespace phoenix = boost::phoenix;
	namespace qi = boost::spirit::qi;

	using qi::skip;
	using qi::eol;
	using qi::eoi;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;
	using qi::lit;

	using unicode::char_;
	using unicode::space;
	using unicode::space_type;
	using unicode::no_case;

	using boost::spirit::info;

////////////////////////////
//Userlist Grammar.
////////////////////////////

	template <typename Iterator>
	struct userlist_grammar : qi::grammar<Iterator, std::vector<rule>(), Skipper<Iterator> > {
		userlist_grammar() : userlist_grammar::base_type(list, "userlist_grammar") {

			list %= userlistRule[&RuleSyntaxCheck] % eol; //A list is a vector of rules. Rules are separated by line endings.

			userlistRule %=
				*eol			//Soak up excess empty lines.
				> ((headerKey
				> ':'
				> object)
				> eol
				> body);

			body %= 
				*eol			//Soak up excess empty lines.
				> (bodyLine % +eol);

			bodyLine %=
				bodyKey
				> ':'
				> object;

			headerKey %= no_case[ruleKeys];

			object %= lexeme[skip("//" >> *(char_ - eol))[+(char_ - eol)]]; //String, but skip comment if present.

			bodyKey %= no_case[lineKeys];

			//Give each rule names.
			list.name("list");
			userlistRule.name("userlistRule");
			body.name("body");
			bodyLine.name("bodyLine");
			headerKey.name("headerKey");
			object.name("object");
			bodyKey.name("bodyKey");
		
			on_error<fail>(list,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(userlistRule,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(body,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(bodyLine,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(headerKey,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(object,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(bodyKey,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
		}

		typedef Skipper<Iterator> skipper;

		qi::rule<Iterator, std::vector<rule>(), skipper> list;
		qi::rule<Iterator, rule(), skipper> userlistRule;
		qi::rule<Iterator, std::vector<line>(), skipper> body;
		qi::rule<Iterator, line(), skipper> bodyLine;
		qi::rule<Iterator, keyType(), skipper> headerKey,bodyKey;
		qi::rule<Iterator, std::string(), skipper> object;
	
		void SyntaxError(Iterator const& first, Iterator const& last, Iterator const& errorpos, info const& what) {
			std::ostringstream value; 
			value << what;
			boss::SyntaxError(first, last, errorpos, value.str());
			
		}
	};
}


#endif