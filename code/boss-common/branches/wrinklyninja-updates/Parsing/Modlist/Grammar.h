/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __BOSS_MODLIST_GRAM_H__
#define __BOSS_MODLIST_GRAM_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include "Parsing/Data.h"
#include "Parsing/Skipper.h"
#include "Parsing/Parser.h"
#include "Common/Globals.h"
#include "Support/Helpers.h"
#include <sstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/home/phoenix/container.hpp>

#include <boost/unordered_set.hpp>

namespace boss {
	namespace unicode = boost::spirit::unicode;
	namespace phoenix = boost::phoenix;
	namespace qi = boost::spirit::qi;

	using namespace std;
	using namespace qi::labels;

	using qi::skip;
	using qi::eol;
	using qi::eoi;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;
	using qi::lit;
	using qi::omit;
	using qi::eps;

	using unicode::char_;
	using unicode::upper;
	using unicode::digit;

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	boost::unordered_set<string> setVars;  //Vars set by masterlist. For old format, this is FCOM, OOO and BC if they exist.
	boost::unordered_set<string>::iterator pos;
	bool skipItem = false, skipMessage = false;

	///////////////////////////////
	//Old Format Grammar Stuff
	///////////////////////////////

	vector<string> openGroups;  //Need to keep track of which groups are open to match up endings properly in old format.

	void path(fs::path& p, string const itemName) {
		if (itemName.length() == 0 && openGroups.size() > 0) 
			p = fs::path(openGroups.back());
		else
			p = fs::path(itemName);
		return;
	}

	void StoreMessage(vector<message>& messages, message currentMessage) {
		if (skipMessage)
			skipMessage = false;
		else
			messages.push_back(currentMessage);
		return;
	}

	void StoreItem(vector<item>& list, item currentItem) {
		if (currentItem.type == BEGINGROUP) {
			openGroups.push_back(currentItem.name.string());
		} else if (currentItem.type == ENDGROUP) {
			openGroups.pop_back();
		}
		if (skipItem)
			skipItem = false;
		else
			list.push_back(currentItem);
		return;
	}
	
	void EvaluateItem(char condition) {
		if (condition == '>') {
			pos = setVars.find("FCOM");
			if (pos == setVars.end())
				skipItem = true;
		} else if (condition == '<') {
			pos = setVars.find("FCOM");
			if (pos != setVars.end())
				skipItem = true;
		}
		return;
	}

	void EvaluateMessage(char condition) {
		if (condition == '>') {
			pos = setVars.find("FCOM");
			if (pos == setVars.end())
				skipMessage = true;
		} else if (condition == '<') {
			pos = setVars.find("FCOM");
			if (pos != setVars.end())
				skipMessage = true;
		}
		return;
	}

	void EvalMessKey(keyType key) {
		if (key == OOOSAY) {
			pos = setVars.find("OOO");
			if (pos == setVars.end())
				skipMessage = true;
		} else if (key == BCSAY) {
			pos = setVars.find("BC");
			if (pos == setVars.end())
				skipMessage = true;
		}
		return;
	}

	//Old format grammar.
	template <typename Iterator>
	struct modlist_old_grammar : qi::grammar<Iterator, vector<item>(), Skipper<Iterator> > {
		modlist_old_grammar() : modlist_old_grammar::base_type(modList, "modlist_old_grammar") {

			vector<message> noMessages;  //An empty set of messages.

			modList = listItem[phoenix::bind(&StoreItem, _val, _1)] % eol;

			listItem %= 
				*eol			//Soak up excess empty lines.
				> ItemType > omit[condition[&EvaluateItem]] > itemName
				> itemMessages;

			ItemType %= 
				groupKey
				| eps[_val = MOD];

			itemName = 
				charString[phoenix::bind(&path, _val, _1)]
				| eps[phoenix::bind(&path, _val, "")];

			itemMessages = 
				(+eol
				>> itemMessage[phoenix::bind(&StoreMessage, _val, _1)] % +eol)
				| eps[_1 = noMessages];

			itemMessage %= omit[condition[&EvaluateMessage]] >>
				messageKeyword[&EvalMessKey]
				> charString;

			condition %= char_('>') |  char_('<') | eps;

			charString %= lexeme[+(char_ - eol)];

			messageKeyword %= masterlistMsgKey;

			modList.name("modList");
			listItem.name("listItem");
			ItemType.name("ItemType");
			itemName.name("itemName");
			itemMessages.name("itemMessages");
			itemMessage.name("itemMessage");
			condition.name("condition");
			charString.name("charString");
			messageKeyword.name("messageKeyword");

			on_error<fail>(modList,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(listItem,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(ItemType,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemName,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemMessages,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemMessage,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(charString,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(condition,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(messageKeyword,phoenix::bind(&modlist_old_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
		}

		typedef Skipper<Iterator> skipper;

		qi::rule<Iterator, vector<item>(), skipper> modList;
		qi::rule<Iterator, item(), skipper> listItem;
		qi::rule<Iterator, itemType(), skipper> ItemType;
		qi::rule<Iterator, fs::path(), skipper> itemName;
		qi::rule<Iterator, vector<message>(), skipper> itemMessages;
		qi::rule<Iterator, message(), skipper> itemMessage;
		qi::rule<Iterator, string(), skipper> charString;
		qi::rule<Iterator, keyType(), skipper> messageKeyword;
		qi::rule<Iterator, char(), skipper> condition;
		
		void SyntaxErr(Iterator const& first, Iterator const& last, Iterator const& errorpos, boost::spirit::info const& what) {
			ostringstream value; 
			value << what;
			boss::SyntaxError(first, last, errorpos, value.str());
		}
	};

	///////////////////////////////
	//New Format Grammar Stuff
	///////////////////////////////

	/*New format grammar is very much like the old format grammar, except that all items have explicit names and conditionals are
	a lot more complex, and keywords and names/messages are separated by semicolons.

	General format:
	conditional = "IF" | "IFNOT" >> "(" >> string - ")" >> ")" >> -":";

	The conditional should not be recorded. The final semicolon is optional because if it is followed by a message or set line, there
	will be a keyword to follow, which will then have a semicolon after it.

	Trick is to make the presence of a conditional optional, but if it is present, to make sure that it must be true to add that item/message.

	// Syntax Info
	//
	// // 		A silent comment - will be ignored by the program.
	//
	// Outputting Keywords:
	//
	// TAG 		A Bashed Patch suggestion for the mod above. (Replaces %)
	// SAY	 	A comment about the mod above. (Replaces ?)
	// REQ		A non-mod requirement (OBSE, Wrye Bash, etc.). (Partially replaces :)
	// WARN		Flags a non-critical installation mistake of the mod in question. Will show up in
	//			yellow for HTML output. (Replaces " and the rest of :)
	// ERROR 	Flags a critical installation mistake of the mod in question. Will show up in red 
	//			for HTML output. (Replaces *)
	//
	// Operation Keywords:
	//
	// SET <var>	Defines a variable of the given name. Variable names must be in uppercase and contain
	//				no numbers.
	//
	// Conditionals:
	//
	// IF (<condition>) <result>
	// IFNOT (<condition>) <result>
	//
	// For the first, if the condition is true then the result will be parsed. If not, it will be ignored. 
	// The second is the opposite of the first.
	//
	// The conditions can be:
	// 
	// A variable name		A string containing only uppercase letters. If the variable is defined, the 
	//						condition will be true and vice-versa.
	// A mod name			A string containing ".esp" or ".esm". If the mod exists, the condition will 
	//						be true and vice-versa.
	// A version number		A string that doesn't contain ".esp" or ".esm" and is preceded by an operator. 
	//						The operators are = < >, and mean 'is equal to', 'is less than' and 
	//						'is greater than' respectively. If the version of the mod currently being processed 
	//						(ie. in the last mod line) fits the expression, then the condition will be true, 
	//						otherwise it will be false.
	// A checksum			A string that doesn't contain ".esp" or ".esm" and isn't preceded by an operator. 
	//						The current mod will have its checksum calculated, and if it matches the given
	//						checksum, the condition will be true, otherwise false.
	//
	// The colon : denotes the end of any keywords/conditions/functions in a line.
	// All lines starting with a keyword or condition must have a leading space, ie: " IF ..."
	// Multiple remark/comment/bash/error lines allowed.
	//
	// The other special keywords that are not to be mixed in the above expressions are:
	//
	// BEGINGROUP: <name>		Begins a group with the given name.
	// ENDGROUP: <name>			Ends the group with the given name.

	// Masterlist variable setup

	//For Oblivion:
	 IF (Oscuro's_Oblivion_Overhaul.esm) SET: OOO
	 IF (Better Cities Resources.esm) SET: BC
	 IF (FCOM_Convergence.esm) SET: FCOM
	// (also remember to add "IFNOT (FCOM) ERROR: FCOM_Convergence.esm is missing." after FCOM_Convergence.esp)
 
	//For Fallout 3:
	 IF (FO3 Wanderers Edition - Main File.esm) SET: FWE
	 IF (FOOK2 - Main.esm) SET: FOOK2
	// (also remember to add "IFNOT (FOOK2) ERROR: FOOK2 - Main.esm is missing." after FOOK2 - Main.esp)
 
	//For Fallout: New Vegas:
	 IF (nVamp - Core.esm) SET: NVAMP
	// (also remember to add "IFNOT (NVAMP) ERROR: FCOM_Convergence.esm is missing." after nVamp - Core.esp)

	//Begin masterlist mod listing.
	*/

	void CheckVar(bool& result, string var) {
		if (setVars.find(var) == setVars.end())
			result = false;
		else
			result = true;
		return;
	}

	void CheckMod(bool& result, string var) {
		if (Exists(data_path / var))
			result = true;
		else
			result = false;
		return;
	}

	void CheckVersion(bool& result, string var) {
		char comp = var[0];
		size_t pos = var.find("|") + 1;
		string version = var.substr(1,pos-1);
		string mod = var.substr(pos+1);

		result = false;

		if (Exists(data_path / mod)) {
			string trueVersion;
			if (fs::exists(data_path / mod))
				trueVersion = GetModHeader(data_path / mod, false);
			else
				trueVersion = GetModHeader(data_path / mod, true);

			switch (comp) {
			case '>':
				if (version > trueVersion)
					result = true;
				break;
			case '<':
				if (version < trueVersion)
					result = true;
				break;
			case '=':
				if (version == trueVersion)
					result = true;
				break;
			}
		}
		return;
	}

	void CheckSum(bool& result, string var) {
		size_t pos = var.find("|") + 1;
		string sum = var.substr(1,pos-1);
		string mod = var.substr(pos+1);

		result = false;

		if (Exists(data_path / mod)) {
			int CRC;
			if (fs::exists(data_path / mod))
				CRC = GetCrc32(data_path / mod);
			else
				CRC = GetCrc32(fs::path((data_path / mod).string()+".ghost"));

			if (atoi(sum.c_str()) == CRC)
				result = true;
		}
		return;
	}

	void StoreVar(string var) {
		setVars.insert(var);
		return;
	}

	//New format grammar.
	template <typename Iterator>
	struct modlist_grammar : qi::grammar<Iterator, vector<item>(), Skipper<Iterator> > {
		modlist_grammar() : modlist_grammar::base_type(modList, "modlist_grammar") {

			vector<message> noMessages;  //An empty set of messages.

			//Three types of item: mod, group and meta. The third is the line that sets variables using SET: <varname>. It does not get recorded.
			modList = (listItem | omit[metaLine]) % eol;

			//Matches a line that sets a variable, eg. SET: FCOM. Vars must be uppercase letters. The expression may also be prepended by a conditional.
			metaLine =
				*eol			//Soak up excess empty lines.
				> -conditional
				>> (lit("SET")
				> ':'
				> (+upper));

			//Mod or group item.
			listItem =
				*eol			//Soak up excess empty lines.
				> ItemType
				> omit[(conditional > ':') | eps]
				> itemName
				> itemMessages;

			ItemType %= 
				(groupKey > ':') 
				| eps[_1 = MOD];

			itemName = charString[phoenix::bind(&path, _val, _1)];

			itemMessages = 
				(+eol			//Soak up excess empty lines.
				>> itemMessage[phoenix::bind(&StoreMessage, _val, _1)] % +eol)
				| eps[_1 = noMessages];

			itemMessage %= 
				omit[conditional | eps] 
				> messageKeyword 
				> ':'
				> charString;

			charString %= lexeme[skip(lit("//") >> *(char_ - eol))[+(char_ - eol)]]; //String, but skip comment if present.

			messageKeyword %= masterlistMsgKey;

			conditional = metaKey > '(' > condition > ')';  //Simple version, only one condition, later expand to n conditions in AND, OR combinations.

			condition = 
				variable[phoenix::bind(&CheckVar, _val, _1)]
				| version[phoenix::bind(&CheckMod, _val, _1)]
				| checksum[phoenix::bind(&CheckSum, _val, _1)]
				| mod[phoenix::bind(&CheckVersion, _val, _1)];

			variable %= +(upper - ')');  //A masterlist variable.

			mod %= +(char_ - ')');  //A mod.

			version %=   //A version, followed by the mod it applies to.
				(char_('=') | char_('>') | char_('<'))
				> +(char_ - '|')
				> char_('|')
				> mod;

			checksum %=  //A CRC-32 checksum, as calculated by BOSS, followed by the mod it applies to.
				+digit
				> char_('|')
				> mod;

			modList.name("modList");
			listItem.name("listItem");
			ItemType.name("ItemType");
			itemName.name("itemName");
			itemMessages.name("itemMessages");
			itemMessage.name("itemMessage");
			conditional.name("condition");
			charString.name("charString");
			messageKeyword.name("messageKeyword");

			on_error<fail>(modList,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(listItem,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(ItemType,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemName,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemMessages,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemMessage,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(charString,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(condition,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(messageKeyword,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
		}

		typedef Skipper<Iterator> skipper;

		qi::rule<Iterator, vector<item>(), skipper> modList;
		qi::rule<Iterator, item(), skipper> listItem;
		qi::rule<Iterator, itemType(), skipper> ItemType;
		qi::rule<Iterator, fs::path(), skipper> itemName;
		qi::rule<Iterator, vector<message>(), skipper> itemMessages;
		qi::rule<Iterator, message(), skipper> itemMessage;
		qi::rule<Iterator, string(), skipper> charString, variable, mod, checksum, version;
		qi::rule<Iterator, keyType(), skipper> messageKeyword;
		qi::rule<Iterator, skipper> conditional;
		qi::rule<Iterator, bool(), skipper> condition;
		qi::rule<Iterator, skipper> metaLine;

		void SyntaxErr(Iterator const& first, Iterator const& last, Iterator const& errorpos, boost::spirit::info const& what) {
			ostringstream value; 
			value << what;
			boss::SyntaxError(first, last, errorpos, value.str());
		}
	};
}
#endif