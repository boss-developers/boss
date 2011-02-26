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

#ifndef __BOSS_MODLIST_GRAM_H__
#define __BOSS_MODLIST_GRAM_H__

#include "Parsing/Data.h"
#include "Parser.h"
#include "Parsing/Skipper.h"

#include <sstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

namespace boss {
	namespace ascii = boost::spirit::ascii;
	namespace phoenix = boost::phoenix;
	namespace qi = boost::spirit::qi;

	using qi::skip;
	using qi::eol;
	using qi::eoi;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;
	using qi::lit;

	using ascii::char_;
	using ascii::space;
	using ascii::no_case;
	using ascii::upper;

	using boost::spirit::info;

///////////////////////////////
//Modlist/Masterlist Grammar
///////////////////////////////

	//New format grammar.
	template <typename Iterator>
	struct modlist_grammar : qi::grammar<Iterator, std::vector<item>(), Skipper<Iterator> > {
		modlist_grammar() : modlist_grammar::base_type(list, "modlist_grammar") {

			list %= item % eol; //A list is a vector of rules. Rules are separated by line endings.

			item = 
				*eol			//Soak up excess empty lines.
				> (modItem | groupStart | groupEnd | varLine);

			groupStart = 
				lit("BEGINGROUP:")
				> string;

			groupEnd = 
				lit("ENDGROUP:")
				> string;

			modItem =
				*eol			//Soak up excess empty lines.
				> (mod
				> eol
				> messages);

			varLine = lit("SET:") > +(upper - eol);

			mod %= string - (lit(".esp") | lit(".esm")) > (lit(".esp") | lit(".esm"));

			messages %= 
				*eol			//Soak up excess empty lines.
				> (message % +eol);

			message %=
				key
				> ':'
				> string;

			key %= no_case[messageKey];

			string %= lexeme[skip("//" >> *(char_ - eol))[+(char_ - eol)]]; //String, but skip comment if present.

			//Give each rule names.
			list.name("list");
			item.name("item");
			messages.name("messages");
			message.name("message");
			key.name("key");
			string.name("string");
		
			on_error<fail>(list,phoenix::bind(&modlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(item,phoenix::bind(&modlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(messages,phoenix::bind(&modlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(message,phoenix::bind(&modlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(key,phoenix::bind(&modlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(string,phoenix::bind(&modlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
		}

		typedef Skipper<Iterator> skipper;

		qi::rule<Iterator, std::vector<item>(), skipper> list;
		qi::rule<Iterator, item(), skipper> item, groupStart, groupEnd, modItem, varLine;
		qi::rule<Iterator, std::vector<message>(), skipper> messages;
		qi::rule<Iterator, message(), skipper> message;
		qi::rule<Iterator, keyType(), skipper> key;
		qi::rule<Iterator, std::string(), skipper> string, mod;
	
		void SyntaxError(Iterator const& first, Iterator const& last, Iterator const& errorpos, info const& what) {
			std::ostringstream value; 
			value << what;
			boss::SyntaxError(first, last, errorpos, value.str());
			
		}
	};
}

/*
Modlist grammar is very complicated.

If the condition is 'ascii::upper' then the condition references a variable that is stored in memory somehow (probably a hash table)
If the keyword is 'lit("SET")' and the string is 'ascii::upper', then the string needs to be stored in memory as a variable.

Because of the complexity, and because not all the information is needed for the resulting data structure, but still plays a vital
role in processing, it might be a better idea to tokenise the input first, then act on that, rather than parsing the file directly.


//Hashmap of variables declared in the modlist/masterlist.
boost::unordered_map<string, bool> hashmap;

template <typename BaseLexer>
struct modlist_tokens : lex::lexer<BaseLexer>
{
    modlist_tokens(): word_count_tokens::base_type(lex::match_flags::match_not_dot_newline)
    {
        // define tokens (the regular expression to match and the corresponding token id)
		conditional = "IF|IFNOT";
		messageKey = "TAG|SAY|REQ|WARN|ERROR"; //These might be better served in a symbol table since they are preserved.
		varKey = "SET";
		groupKey = "(BEGIN|END)GROUP"; //Perhaps also a candidate for a symbol table.
		colon = ":";
		versionComp = "[=\\>\\<]";
		or = "\\|\\|";
		and = "&&";
		mod = "[^\t\n\\/:\\\"\\*\\?\\>\\<\\|]+(.esp|.esm)"; //A mod name. Warning: Could occur in a message or a condition, not just a mod entry.
		listVar = ascii::upper; //An uppercase string.
		//associate them with the lexer 
        this->self =
			conditional
			| messageKey
			| groupKey
			| colon
			| versionComp
			| or
			| and
			| mod
			| listVar
        ;
    }

	lex::token_def<std::string> conditional;
	lex::token_def<std::string> messageKey;
	lex::token_def<std::string> groupKey;
	lex::token_def<std::string> colon;
	lex::token_def<std::string> versionComp;
	lex::token_def<std::string> or;
	lex::token_def<std::string> and;
	lex::token_def<std::string> mod;
	lex::token_def<std::string> listVar;
};

Grammar rules.

Pretty complicated. One storage type: item(itemType,string,vector<message>).

However, group lines have an empty vector<message> and mods might.
At least, groups don't have their messages displayed. No reason why the parser shouldn't accept them though.
This makes things easier.

Hence the most basic format for an item is:
header > eol > *(message % eol)

The header will be of the form: header = groupKey > ':' > string 
for a group. This would result in the correct format for the storage type.

For a mod, the header is of the form: header = -(expression > ':') >> mod
Expression is not parsed, since it has no keyword, but used to determine line validity. 
The format is still missing an itemType. This may be attachable via a semantic action.

Conditionals are important. They evaluate to true or false, and decide whether or not the next part of the line should be parsed or not.

template <typename Iterator>
struct modlist_grammar : qi::grammar<Iterator, boss::modlist(), modlist_skipper<Iterator> > {

	//Grammar rules.
    modlist_grammar() : modlist_grammar::base_type(list, "modlist_grammar") {

		items = item % *eol;

		item = 
			(groupHeader | modHeader)
			> eol
			> -(messageLine % eol);
			;

		groupHeader = 
			modlist::groupKey							//This needs to be used to figure out what type the item is, and record the type in boss::item.type.
			> ':'
			> string;							//This needs to turned into a path and recorded in boss::item.name


		modItem = 
			-(conditional > ':')				//This expression needs to be evaluated, but not recorded, although it will affect what is recorded. 
			>> mod;								//This needs to turned into a path and recorded in boss::item.name. It is also what decides the type.

		messageLine = 
			-conditional 
			>> keyword
			> ':' 
			> string;

		string = skip("//" >> *(char_ - eol))[+(char_ - eol)];

		conditional = 
			modlist::condKey 
			> '(' 
			> condition 
			> *( condOp > condition) 
			>> ')'

		mod = string - (".esp" | ".esm") > (".esp" | ".esm");

		condition = 
			ascii::upper 
			| (versionComp > string)
			| mod 
			| string;

		keyword = modlist::messageKey | modlist::varKey;

		versionComp = '=' | '>' | '<';

	}
	typedef modlist_skipper<Iterator> skipper;

	qi::rule<Iterator, std::vector<boss::item>, skipper> modlist;
	qi::rule<Iterator, boss::item, skipper> item;
	qi::rule<Iterator, fs::path, skipper> name;
	qi::rule<Iterator, boss::itemType, skipper> type;
	qi::rule<Iterator, boss::message, skipper> message;
	qi::rule<Iterator, boss::keyType, skipper> messageKey;
	qi::rule<Iterator, std::string, skipper> string;
};
*/
#endif