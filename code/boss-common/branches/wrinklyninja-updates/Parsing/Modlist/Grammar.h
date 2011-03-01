/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __BOSS_MODLIST_GRAM_H__
#define __BOSS_MODLIST_GRAM_H__

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
	namespace iso8859_1 = boost::spirit::iso8859_1;
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
	using qi::_val;
	using qi::omit;
	using qi::eps;

	using iso8859_1::char_;
	using iso8859_1::space;
	using iso8859_1::no_case;
	using iso8859_1::upper;

	using boost::spirit::info;

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	/*\ Leading symbol code:
	\
	\ \ A silent comment - will be ignored by the program.
	\ % A Bashed Patch suggestion for the mod above.
	\ ? A comment about the mod above.
	\ * Flags a critical mistake for FCOM installation in relation to the mod above.
	\ : An installation requirement (ie another mod or OBSE etc.).
	\ " A specific incompatibility.
	\ $ An OOO specific comment.
	\ ^ A Better Cities specific comment
	\
	\ > process this line only if FCOM is installed.
	\ < process this line only if FCOM isn't installed.
	\
	\ Multiple remark/comment/bash/error lines allowed.
	\ Lines beginning with \ and blank lines are treated the same (ignored).

	\BeginGroup\: ESMs
	\EndGroup\\
	*/

	std::vector<std::string> openGroups; //Need to keep track of which groups are open to match up endings properly in old format.
	bool OOO, BC, FCOM;  //Need to check for these specific mods in old format.
	

	//Old format grammar.
	template <typename Iterator>
	struct modlist_old_grammar : qi::grammar<Iterator, std::vector<item>(), Skipper<Iterator> > {
		modlist_old_grammar() : modlist_old_grammar::base_type(modList, "modlist_old_grammar") {

			std::vector<boss::message> m;

			modList %= listItem % eol;

			listItem %= 
				*eol			//Soak up excess empty lines.
				> (ItemType > itemName
				> eps[qi::_1 = m]) 
				| 
				(ItemType > itemName
				//> eps[qi::_1 = m]) 
				> +eol > itemMessages)
			;

			ItemType %= 
				groupKey 
				| (groupKey > lit("\\"))
				| eps[_val = MOD];

			itemName = string[phoenix::bind(&ThePatherator, _val, qi::_1)]
				| eps[phoenix::bind(&ThePatherator, _val, "ENDGROUP")];

			itemMessages %= itemMessage % +eol;

			itemMessage %= 
				messageKeyword
				> string;

			string %= lexeme[skip(lit("//") >> *(char_ - eol))[+(char_ - eol)]]; //String, but skip comment if present.

			messageKeyword %= oldMasterlistMsgKey;

			modList.name("modList");
			listItem.name("listItem");
			ItemType.name("ItemType");
			itemName.name("itemName");
			itemMessages.name("itemMessages");
			itemMessage.name("itemMessage");
			string.name("string");
			messageKeyword.name("messageKeyword");

			on_error<fail>(modList,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(listItem,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(ItemType,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(itemName,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(itemMessages,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(itemMessage,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(string,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(messageKeyword,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr,this,qi::_1,qi::_2,qi::_3,qi::_4));
		}

		typedef Skipper<Iterator> skipper;

		qi::rule<Iterator, std::vector<item>(), skipper> modList;
		qi::rule<Iterator, item(), skipper> listItem;			//Needs to have components keyType, fs::path, vector<message>
		qi::rule<Iterator, itemType(), skipper> ItemType;
		qi::rule<Iterator, fs::path(), skipper> itemName;			//
		qi::rule<Iterator, std::vector<message>(), skipper> itemMessages;
		qi::rule<Iterator, message(), skipper> itemMessage;
		qi::rule<Iterator, std::string(), skipper> string;
		qi::rule<Iterator, keyType(), skipper> messageKeyword;
		
		void SyntaxErr(Iterator const& first, Iterator const& last, Iterator const& errorpos, info const& what) {
			std::ostringstream value; 
			value << what;
			std::string context(errorpos, std::min(errorpos + 50, last));
			cout << "HOLY HELL, " << what << "JUST BROKE AT " << context << "!";
			
			boss::SyntaxError(first, last, errorpos, value.str());
		}
	};

	void ThePatherator(fs::path& p, std::string const& str) {
		p = fs::path(str);
		return;
	}

	void TheMessagizor(std::vector<message>& in) {
		std::vector<message> out;
		in = out;
		return;
	}

}
	/*
	//Old format grammar.
	template <typename Iterator>
	struct old_modlist_grammar : qi::grammar<Iterator, std::vector<item>(), Skipper<Iterator> > {
		old_modlist_grammar() : old_modlist_grammar::base_type(list, "old_modlist_grammar") {

			list %= listItem % eol; //A list is a vector of rules. Rules are separated by line endings.

			listItem = 
				*eol			//Soak up excess empty lines.
				> (string								
				> eol 
				> -itemMessages)				[phoenix::bind(&old_modlist_grammar<Iterator>::VarSet, this, _val, qi::_1, qi::_2)];

			itemMessages %= 
				*eol			//Soak up excess empty lines.
				> (itemMessage % eol);

			itemMessage %=
				oldMasterlistMsgKey
				> ':'
				> string;

			string %= lexeme[skip("//" >> *(char_ - eol))[+(char_ - eol)]]; //String, but skip comment if present.

			//Give each rule names.
			list.name("list");
			listItem.name("item");
			itemMessages.name("messages");
			itemMessage.name("message");
			string.name("string");
		
			on_error<fail>(list,phoenix::bind(&old_modlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(listItem,phoenix::bind(&old_modlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(itemMessages,phoenix::bind(&old_modlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(itemMessage,phoenix::bind(&old_modlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(string,phoenix::bind(&old_modlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
		}

		typedef Skipper<Iterator> skipper;

		qi::rule<Iterator, std::vector<item>(), skipper> list;
		qi::rule<Iterator, item(), skipper> listItem;
		qi::rule<Iterator, std::vector<message>(), skipper> itemMessages;
		qi::rule<Iterator, message(), skipper> itemMessage;
		qi::rule<Iterator, std::string(), skipper> string;
	
		void SyntaxError(Iterator const& first, Iterator const& last, Iterator const& errorpos, info const& what) {
			std::ostringstream value; 
			value << what;
	//		boss::SyntaxError(first, last, errorpos, value.str());
			
		}

		void VarSet(item listItem, std::string strname, std::vector<message> itemMessages) {
			if (strname.find("EndGroup")) {
				listItem.type = ENDGROUP;
				listItem.name = fs::path(openGroups.back());
				openGroups.pop_back();
				listItem.messages.clear();
			} else if (strname.find("BeginGroup")) {
				listItem.type = BEGINGROUP;
				size_t pos = strname.find(":");
				openGroups.push_back(strname.substr(pos+1));
				listItem.name = fs::path(strname.substr(pos+1));
				listItem.messages.clear();
			} else {
				listItem.type = MOD;
				listItem.name = strname;
				listItem.messages = itemMessages;
			}
		}
	};
}
*/
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