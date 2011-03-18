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
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;
	using qi::lit;
	using qi::omit;
	using qi::eps;
	using qi::int_;

	using unicode::char_;
	using unicode::upper;
	using unicode::digit;

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	boost::unordered_set<string> setVars;  //Vars set by masterlist.
	boost::unordered_set<string>::iterator pos;
	bool skipItem = false, skipMessage = false;

	vector<string> openGroups;  //Need to keep track of which groups are open to match up endings properly in old format.

	void path(fs::path& p, string const itemName) {
		if (itemName.length() == 0 && openGroups.size() > 0) 
			p = fs::path(openGroups.back());
		else
			p = fs::path(itemName);
		return;
	}

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
		string version = var.substr(1,pos-2);
		string mod = var.substr(pos);
		result = false;

		if (Exists(data_path / mod)) {
			string trueVersion = GetModHeader(data_path / mod);

			switch (comp) {
			case '>':
				if (trueVersion.compare(version) > 0)
					result = true;
				break;
			case '<':
				if (trueVersion.compare(version) < 0)
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

	void CheckSum(bool& result, int sum, string mod) {
		result = false;
		if (Exists(data_path / mod)) {
			int CRC = GetCrc32(data_path / mod);
			if (sum == CRC)
				result = true;
		}
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

	void StoreVar(bool result, string var) {
		if (result)
			setVars.insert(var);
		return;
	}

	void EvaluateConditional(bool& result, metaType type, bool condition) {
		result = false;
		if (type == IF && condition == true)
			result = true;
		else if (type == IFNOT && condition == false)
			result = true;
		return;
	}

	void EvalOldFCOMConditional(bool& result, char var) {
		result = false;
		pos = setVars.find("FCOM");
		if (var == '>' && pos != setVars.end())
				result = true;
		else if (var == '<' && pos == setVars.end())
				result = true;
		return;
	}

	void EvaluateItem(bool conditional) {
		if (!conditional)
			skipItem = true;
		return;
	}

	void EvaluateMessage(bool conditional) {
		if (!conditional)
			skipMessage = true;
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
	
	void Evaluate2ndConditional(bool& result, string andOr, bool condition2) {
		if (andOr == "||" && condition2 == true)
			result = true;
		else if (andOr == "&&" && result == true && condition2 == false)
			result = false;
	}

	//Old and new formats grammar.
	template <typename Iterator>
	struct modlist_grammar : qi::grammar<Iterator, vector<item>(), Skipper<Iterator> > {
		modlist_grammar() : modlist_grammar::base_type(modList, "modlist_grammar") {

			vector<message> noMessages;  //An empty set of messages.

			modList = (metaLine | listItem[phoenix::bind(&StoreItem, _val, _1)]) % eol;

			metaLine =
				*eol			//Soak up excess empty lines.
				>> (conditionals
				> lit("SET")
				> ':'
				> upperString)[phoenix::bind(&StoreVar, _1, _2)];

			listItem %= 
				*eol			//Soak up excess empty lines.
				> ItemType 
				> omit[oldConditional[&EvaluateItem] | (conditionals > ':')[&EvaluateItem] | eps]
				> itemName
				> itemMessages;

			ItemType %= 
				(groupKey >> -lit(":"))
				| eps[_val = MOD];

			itemName = 
				charString[phoenix::bind(&path, _val, _1)]
				| eps[phoenix::bind(&path, _val, "")];

			itemMessages = 
				(+eol
				>> itemMessage[phoenix::bind(&StoreMessage, _val, _1)] % +eol)
				| eps[_1 = noMessages];

			itemMessage %= 
				omit[oldConditional[&EvaluateMessage] | conditionals[&EvaluateMessage] | eps] 
				>> messageKeyword[&EvalMessKey]
				> -lit(":")
				> messageString;

			charString %= lexeme[skip(lit("//") >> *(char_ - eol))[+(char_ - eol)]]; //String, but skip comment if present. Don't skip web links.

			messageString %= lexeme[+(char_ - eol)]; //String, with no skipper. Used for messages, which can contain web links which the skipper would cut out.

			upperString %= lexeme[skip(lit("//") >> *(char_ - eol))[+(upper - eol)]]; //String of uppercase letters, with comment skipper.

			messageKeyword %= masterlistMsgKey;

			oldConditional = 
				char_(">")			[phoenix::bind(&EvalOldFCOMConditional, _val, _1)]
				|  char_("<")		[phoenix::bind(&EvalOldFCOMConditional, _val, _1)];

			conditionals = 
				conditional[_val = _1] > -(andOr > conditional)[phoenix::bind(&Evaluate2ndConditional, _val, _1, _2)];

			andOr %= unicode::string("&&") | unicode::string("||");

			conditional = 
				(metaKey > '(' > condition > ')')[phoenix::bind(&EvaluateConditional, _val, _1, _2)];  //Simple version, only one condition, later expand to n conditions in AND, OR combinations.

			condition = 
				variable[phoenix::bind(&CheckVar, _val, _1)]
				| version[phoenix::bind(&CheckVersion, _val, _1)]
				| (int_ > '|' > mod)[phoenix::bind(&CheckSum, _val, _1, _2)] //A CRC-32 checksum, as calculated by BOSS, followed by the mod it applies to.
				| mod[phoenix::bind(&CheckMod, _val, _1)];

			variable %= '$' > +(upper - ')');  //A masterlist variable, prepended by a '$' character to differentiate between vars and mods.

			mod %= lexeme['\"' > +(char_ - '\"') > '\"'];  //A mod, enclosed in quotes.

			version %=   //A version, followed by the mod it applies to.
				(char_('=') | char_('>') | char_('<'))
				> lexeme[+(char_ - '|')]
				> char_('|')
				> mod;

			modList.name("modList");
			metaLine.name("metaLine");
			listItem.name("listItem");
			ItemType.name("ItemType");
			itemName.name("itemName");
			itemMessages.name("itemMessages");
			itemMessage.name("itemMessage");
			charString.name("charString");
			messageString.name("messageString");
			upperString.name("upperString");
			messageKeyword.name("messageKeyword");
			oldConditional.name("oldConditional");
			conditionals.name("conditional");
			andOr.name("andOr");
			conditional.name("conditional");
			condition.name("condition");
			variable.name("variable");
			mod.name("mod");
			version.name("version");
			
			on_error<fail>(modList,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(listItem,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(ItemType,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemName,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemMessages,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemMessage,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(charString,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(messageString,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(messageKeyword,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));

			on_error<fail>(metaLine,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(upperString,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(oldConditional,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(conditionals,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(andOr,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(conditional,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(condition,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(variable,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(mod,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(version,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));

		}

		typedef Skipper<Iterator> skipper;

		qi::rule<Iterator, vector<item>(), skipper> modList;
		qi::rule<Iterator, item(), skipper> listItem;
		qi::rule<Iterator, itemType(), skipper> ItemType;
		qi::rule<Iterator, fs::path(), skipper> itemName;
		qi::rule<Iterator, vector<message>(), skipper> itemMessages;
		qi::rule<Iterator, message(), skipper> itemMessage;
		qi::rule<Iterator, string(), skipper> charString, upperString, messageString, variable, mod, version, andOr;
		qi::rule<Iterator, keyType(), skipper> messageKeyword;
		qi::rule<Iterator, bool(), skipper> conditional, conditionals, condition, oldConditional;
		qi::rule<Iterator, skipper> metaLine;
		
		void SyntaxErr(Iterator const& first, Iterator const& last, Iterator const& errorpos, boost::spirit::info const& what) {
			cout << what;
		}
	};
}
#endif