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
			string trueVersion;
			if (fs::exists(data_path / mod))
				trueVersion = GetModHeader(data_path / mod, false);
			else
				trueVersion = GetModHeader(data_path / mod, true);

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

	//Doesn't currently work - the BOSSLog outputs different checksums to what is calculated in this for some reason.
	void CheckSum(bool& result, int sum, string mod) {
		result = false;
		if (Exists(data_path / mod)) {
			int CRC;
			if (fs::exists(data_path / mod))
				CRC = GetCrc32(data_path / mod);
			else
				CRC = GetCrc32(data_path / fs::path(mod+".ghost"));
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
	
	//Old and new formats grammar.
	template <typename Iterator>
	struct modlist_grammar : qi::grammar<Iterator, vector<item>(), Skipper<Iterator> > {
		modlist_grammar() : modlist_grammar::base_type(modList, "modlist_grammar") {

			vector<message> noMessages;  //An empty set of messages.

			modList = (metaLine | listItem[phoenix::bind(&StoreItem, _val, _1)]) % eol;

			metaLine =
				*eol			//Soak up excess empty lines.
				>> (conditional
				> lit("SET")
				> ':'
				> upperString)[phoenix::bind(&StoreVar, _1, _2)];

			listItem %= 
				*eol			//Soak up excess empty lines.
				> ItemType 
				> omit[oldConditional[&EvaluateItem] | (conditional > ':')[&EvaluateItem] | eps]
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
				omit[oldConditional[&EvaluateMessage] | conditional[&EvaluateMessage] | eps] 
				>> messageKeyword[&EvalMessKey]
				> -lit(":")
				> charString;

			charString %= lexeme[skip(lit("//") >> *(char_ - eol))[+(char_ - eol)]]; //String, but skip comment if present.

			upperString %= lexeme[skip(lit("//") >> *(char_ - eol))[+(upper - eol)]]; //String of uppercase letters, with comment skipper.

			messageKeyword %= masterlistMsgKey;

			oldConditional = 
				char_(">")			[phoenix::bind(&EvalOldFCOMConditional, _val, _1)]
				|  char_("<")		[phoenix::bind(&EvalOldFCOMConditional, _val, _1)];

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
			upperString.name("upperString");
			messageKeyword.name("messageKeyword");
			oldConditional.name("oldConditional");
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
			on_error<fail>(messageKeyword,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
		}

		typedef Skipper<Iterator> skipper;

		qi::rule<Iterator, vector<item>(), skipper> modList;
		qi::rule<Iterator, item(), skipper> listItem;
		qi::rule<Iterator, itemType(), skipper> ItemType;
		qi::rule<Iterator, fs::path(), skipper> itemName;
		qi::rule<Iterator, vector<message>(), skipper> itemMessages;
		qi::rule<Iterator, message(), skipper> itemMessage;
		qi::rule<Iterator, string(), skipper> charString, upperString, variable, mod, version;
		qi::rule<Iterator, keyType(), skipper> messageKeyword;
		qi::rule<Iterator, bool(), skipper> conditional, condition, oldConditional;
		qi::rule<Iterator, skipper> metaLine;
		
		void SyntaxErr(Iterator const& first, Iterator const& last, Iterator const& errorpos, boost::spirit::info const& what) {
			ostringstream value; 
			value << what;
			boss::SyntaxError(first, last, errorpos, value.str());
		}
	};

	///////////////////////////////
	// Syntax Notes
	///////////////////////////////

	// Old syntax:
	/*
		\ Leading symbol code:
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
	*/

	// New syntax:
	/*
		// Syntax Info
		//
		// // 		A silent comment - will be ignored by the program.
		//
		// Outputting Keywords:
		//
		// TAG 		A Bashed Patch suggestion for the mod above. (Replaces %)
		// SAY	 	A comment about the mod above. (Replaces ?)
		// REQ		A requirement for the mod above. (Replaces :)
		// INC		An incompatibility for the mod above. (Replaces ")
		// WARN		Flags a non-critical installation mistake of the mod in question. Will show up in
		//			yellow for HTML output.
		// ERROR 	Flags a critical installation mistake of the mod in question. Will show up in red 
		//			for HTML output. (Replaces *)
		//
		// Operation Keywords:
		//
		// SET <var>	Defines a variable of the given name. Variable names must contain only uppercase letters.
		//
		// Conditionals:
		//
		// IF (<condition>) <result>
		// IFNOT (<condition>) <result>
		//
		// For the first, if the condition is true then the result will be parsed. If not, it will be ignored. 
		// The second is the opposite of the first.
		//
		// There are four types of condition:
		//
		// Type			Syntax				Description
		// ====================================================================================================================
		// Variable		$<var>							A '$' character followed by a variable name, given in uppercase letters.
		//												If the variable is defined, the condition evaluates to true, else false.
		//
		// Mod			"<mod>"							A mod filename, including the ".esp" or ".esm" extension, enclosed in double quotes.
		//												If the mod is installed, the condition evaluates to true, else false.
		//
		// Checksum		<integer>|"<mod>"				A checksum integer as given by BOSS, including "-" sign if it is negative, followed 
		//												by a "|" character and the mod the checksum is to be checked against. If the mod's 
		//												checksum is equal to the given checksum, the condition evaluates to true, else false.
		//
		// Version		<comparator><version>|"<mod>"	The comparator can be one of "=", ">" or "<", meaning "is equal to", "is greater than"
		//												and "is less than" respectively. The version is the version to check for, as given by BOSS.
		//												The mod is the mod that shall have its version checked. If the equation or inequality of the
		//												form '<version> <comparator> <mod version>' (eg. '1.5 > 1.2.5', which is true) holds true, 
		//												the condition evaluates to true, else false.
		//
		//
		// The colon : denotes the end of any keywords/conditions/functions in a line.
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
		// (also remember to add "IFNOT (NVAMP) ERROR: nVamp - Core.esm is missing." after nVamp - Core.esp)
	*/
}
#endif