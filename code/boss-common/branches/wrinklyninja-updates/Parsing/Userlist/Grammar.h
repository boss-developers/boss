/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Header file for userlist grammar definition.

/* Need to:
1. Get error handling working properly (currently throws silent exceptions and doesn't specify area).
2. Make error reports useful.
2. Add content checks.
3. Make content checks actually affect final parsing product.
*/
#ifndef __BOSS_USERLIST_GRAM_H__
#define __BOSS_USERLIST_GRAM_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include "Parsing/Data.h"
#include "Parsing/Skipper.h"
#include "Parsing/Parser.h"
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
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;

	using unicode::char_;
	using unicode::no_case;

	using boost::spirit::info;

	////////////////////////////
	//Userlist Grammar.
	////////////////////////////

	//Need to structure things to that it provides more meaningful error reporting.
	//Need to know what specific component failed.

	template <typename Iterator>
	struct userlist_grammar : qi::grammar<Iterator, std::vector<rule>(), Skipper<Iterator> > {
		userlist_grammar() : userlist_grammar::base_type(list, "userlist grammar") {

			//A list is a vector of rules. Rules are separated by line endings.
			list %= userlistRule[&RuleSyntaxCheck] % eol; 

			//A rule consists of a rule line containing a rule keyword and a rule object, followed by one or more message or sort lines.
			userlistRule %=
				*eol			//Soak up excess empty lines.
				> ((ruleKey > ':' > object)
				> +eol
				> ((sortLine | messageLine) % +eol));

			sortLine %=
				sortKey
				> ':'
				> object;

			messageLine %=
				messageKey
				> ':'
				> object;

			object %= lexeme[skip("//" >> *(char_ - eol))[+(char_ - eol)]]; //String, but skip comment if present.

			ruleKey %= no_case[ruleKeys];

			sortKey %= no_case[sortKeys];

			messageKey %= no_case[messageKeys];

			//Give each rule names.
			list.name("list");
			userlistRule.name("rule");
			sortLine.name("sort line");
			messageLine.name("message line");
			object.name("object");
			ruleKey.name("rule keyword");
			sortKey.name("sort keyword");
			messageKey.name("message keyword");
		
			on_error<fail>(list,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(userlistRule,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(sortLine,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(messageLine,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(object,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(ruleKey,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(sortKey,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(messageKey,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
		}

		typedef Skipper<Iterator> skipper;

		qi::rule<Iterator, std::vector<rule>(), skipper> list;
		qi::rule<Iterator, rule(), skipper> userlistRule;
		qi::rule<Iterator, line(), skipper> sortLine, messageLine;
		qi::rule<Iterator, keyType(), skipper> ruleKey, sortKey, messageKey;
		qi::rule<Iterator, std::string(), skipper> object;
	
		void SyntaxError(Iterator const& first, Iterator const& last, Iterator const& errorpos, info const& what) {
			std::ostringstream value; 
			value << what;
			boss::SyntaxError(first, last, errorpos, value.str());
		}
	};
}


#endif