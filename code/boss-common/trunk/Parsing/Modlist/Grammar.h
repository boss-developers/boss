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
#include "Support/Logger.h"

#include <sstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

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
	using qi::hex;

	using unicode::char_;

	using boost::format;
	using boost::spirit::info;

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	boost::unordered_set<string> setVars;  //Vars set by masterlist.
	boost::unordered_set<string>::iterator pos;
	bool storeItem = true, storeMessage = true;
	vector<string> openGroups;  //Need to keep track of which groups are open to match up endings properly in MF1.
	boost::unordered_map<string,unsigned int> fileCRCs;
	boost::unordered_map<string,unsigned int>::iterator iter;

	//Parsing error message format.
	static format MasterlistParsingErrorFormat("<p><span class='error'>Masterlist Parsing Error: Expected a %1% at:</span>"
		"<blockquote>%2%</blockquote>"
		"<span class='error'>Masterlist parsing aborted. Utility will end now.</span></p>\n\n");

	//Checks if a masterlist variable is defined.
	void CheckVar(bool& result, string var) {
		if (setVars.find(var) == setVars.end())
			result = false;
		else
			result = true;
		return;
	}

	//Returns the true path based on what type of file or keyword it is.
	void GetPath(fs::path& file_path, string& file) {
		if (file == "OBSE") {
			file_path = "..";
			file = "obse_1_2_416.dll";  //Don't look for the loader because steam users don't need it.
		} else if (file == "FOSE") {
			file_path = "..";
			file = "fose_loader.exe";
		} else if (file == "NVSE") {
			file_path = "..";
			file = "nvse_loader.exe";
		} else if (file == "BOSS") {
			file_path = ".";
			file = "BOSS.exe";
		} else if (file == "TES4") {
			file_path = "..";
			file = "Oblivion.exe";
		} else if (file == "FO3") {
			file_path = "..";
			file = "Fallout3.exe";
		} else if (file == "FONV") {
			file_path = "..";
			file = "FalloutNV.exe";
		} else {
			fs::path p(file);
			if (Tidy(p.extension().string()) == ".dll" && p.string().find("/") == string::npos && p.string().find("\\") == string::npos) {
				if (fs::exists(data_path / "OBSE"))
					file_path = data_path / fs::path("OBSE/Plugins");  //Oblivion - OBSE plugins.
				else if (fs::exists(data_path / "FOSE"))
					file_path = data_path / fs::path("FOSE/Plugins");  //Fallout 3 - FOSE plugins.
				else
					file_path = data_path / fs::path("NVSE/Plugins");  //Fallout: New Vegas - NVSE plugins.
			} else
				file_path = data_path;
		}

	}

	//Checks if the given mod has a version for which the comparison holds true.
	void CheckVersion(bool& result, string var) {
		char comp = var[0];
		size_t pos = var.find("|") + 1;
		string version = var.substr(1,pos-2);
		string file = var.substr(pos);
		result = false;
		fs::path file_path;

		GetPath(file_path,file);

		if (Exists(file_path / file)) {
			string trueVersion;
			if (file_path == data_path) {
				if (IsGhosted(file_path / file)) 
					trueVersion = GetModHeader(file_path / fs::path(file + ".ghost"));
				else 
					trueVersion = GetModHeader(file_path / file);
			} else
				trueVersion = GetExeDllVersion(file_path / file);

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

	//Checks if the given mod has the given checksum.
	void CheckSum(bool& result, unsigned int sum, string file) {
		result = false;
		fs::path file_path;
		unsigned int CRC;

		GetPath(file_path,file);
		iter = fileCRCs.find(file);

		if (iter != fileCRCs.end()) {
			CRC = fileCRCs.at(file);
		} else if (Exists(file_path / file)) {
			if (file_path == data_path) {
				if (IsGhosted(file_path / file)) 
					CRC = GetCrc32(file_path / fs::path(file + ".ghost"));
				else
					CRC = GetCrc32(file_path / file);
			} else
				CRC = GetCrc32(file_path / file);
			fileCRCs.emplace(file,CRC);
		}

		if (sum == CRC)
			result = true;
		return;
	}

	void CheckFile(bool& result, string file) {
		result = false;
		fs::path file_path;
		GetPath(file_path,file);
		result = fs::exists(file_path / file);
	}

	//Stores a message, should it be appropriate.
	void StoreMessage(vector<message>& messages, message currentMessage) {
		if (storeMessage)
			messages.push_back(currentMessage);
		return;
	}

	//Stores the given item, should it be appropriate, and records any changes to open groups.
	void StoreItem(vector<item>& list, item currentItem) {
		if (currentItem.type == BEGINGROUP) {
			openGroups.push_back(currentItem.name.string());
		} else if (currentItem.type == ENDGROUP) {
			openGroups.pop_back();
		}
		if (storeItem)
			list.push_back(currentItem);
		return;
	}

	//Defines the given masterlist variable, if appropriate.
	void StoreVar(bool result, string var) {
		if (result)
			setVars.insert(var);
		return;
	}

	//Defines the given masterlist variable, if appropriate.
	void StoreGlobalMessage(bool result, keyType messageKey, string messageData) {
		if (result) {
			message m;
			m.key = messageKey;
			m.data = messageData;
			globalMessageBuffer.push_back(m);
		}
		return;
	}

	//Evaluate a single conditional.
	void EvaluateConditional(bool& result, metaType type, bool condition) {
		result = false;
		if (type == IF && condition == true)
			result = true;
		else if (type == IFNOT && condition == false)
			result = true;
		return;
	}

	//Evaluate the second half of a complex conditional.
	void EvaluateCompoundConditional(bool& result, string andOr, bool condition) {
		if (andOr == "||" && condition == true)
			result = true;
		else if (andOr == "&&" && result == true && condition == false)
			result = false;
	}

	//MF1 compatibility function. Evaluates the MF1 FCOM conditional. Like it says on the tin.
	void EvalOldFCOMConditional(bool& result, char var) {
		result = false;
		pos = setVars.find("FCOM");
		if (var == '>' && pos != setVars.end())
				result = true;
		else if (var == '<' && pos == setVars.end())
				result = true;
		return;
	}

	//MF1 compatibility function. Evaluates the MF1 OOO/BC conditional message symbols.
	void EvalMessKey(keyType key) {
		if (key == OOOSAY) {
			pos = setVars.find("OOO");
			if (pos == setVars.end())
				storeMessage = false;
		} else if (key == BCSAY) {
			pos = setVars.find("BC");
			if (pos == setVars.end())
				storeMessage = false;
		}
		return;
	}
	
	//Turns a given string into a path. Can't be done directly because of the openGroups checks.
	void path(fs::path& p, string const itemName) {
		if (itemName.length() == 0 && openGroups.size() > 0) 
			p = fs::path(openGroups.back());
		else
			p = fs::path(itemName);
		return;
	}

	//Old and new formats grammar.
	template <typename Iterator>
	struct modlist_grammar : qi::grammar<Iterator, vector<item>(), Skipper<Iterator> > {
		modlist_grammar() : modlist_grammar::base_type(modList, "modlist_grammar") {

			vector<message> noMessages;  //An empty set of messages.

			modList = (metaLine | globalMessage | listItem[phoenix::bind(&StoreItem, _val, _1)]) % +eol;

			metaLine =
				(conditionals
				>> (lit("SET")
				> ':'
				> charString))[phoenix::bind(&StoreVar, _1, _2)];

			globalMessage =
				(conditionals
				>> lit("GLOBAL")
				> messageKeyword
				> ':'
				> messageString)[phoenix::bind(&StoreGlobalMessage, _1, _2, _3)];

			listItem %= 
				omit[(oldConditional | conditionals)[phoenix::ref(storeItem) = _1]]
				> ItemType
				> itemName
				> itemMessages;

			ItemType %= (typeKey >> ':') | eps[_val = MOD];

			itemName = 
				charString[phoenix::bind(&path, _val, _1)]
				| eps[phoenix::bind(&path, _val, "")];

			itemMessages = 
				(+eol
				>> itemMessage[phoenix::bind(&StoreMessage, _val, _1)] % +eol)
				| eps[_1 = noMessages];

			itemMessage %= 
				omit[(oldConditional | conditionals)[phoenix::ref(storeMessage) = _1]]
				>>(messageKeyword[&EvalMessKey]
				> -lit(":")
				> messageString);

			charString %= lexeme[skip(lit("//") >> *(char_ - eol))[+(char_ - eol)]]; //String, but skip comment if present.

			messageString %= lexeme[+(char_ - eol)]; //String, with no skipper. Used for messages, which can contain web links which the skipper would cut out.

			messageKeyword %= masterlistMsgKey;

			oldConditional = (char_(">") |  char_("<"))		[phoenix::bind(&EvalOldFCOMConditional, _val, _1)];

			conditionals = 
				(conditional[_val = _1] 
				> *((andOr > conditional)			[phoenix::bind(&EvaluateCompoundConditional, _val, _1, _2)]))
				| eps[_val = true];

			andOr %= unicode::string("&&") | unicode::string("||");

			conditional = (metaKey > '(' > condition > ')')	[phoenix::bind(&EvaluateConditional, _val, _1, _2)];

			condition = 
				variable									[phoenix::bind(&CheckVar, _val, _1)]
				| version									[phoenix::bind(&CheckVersion, _val, _1)]
				| (hex > '|' > file)						[phoenix::bind(&CheckSum, _val, _1, _2)] //A CRC-32 checksum, as calculated by BOSS, followed by the file it applies to.
				| file										[phoenix::bind(&CheckFile, _val, _1)]
				;

			variable %= '$' > +(char_ - ')');  //A masterlist variable, prepended by a '$' character to differentiate between vars and mods.

			file %= lexeme['\"' > +(char_ - '\"') > '\"'];  //An OBSE plugin or a mod plugin.

			version %=   //A version, followed by the mod it applies to.
				(char_('=') | char_('>') | char_('<'))
				> lexeme[+(char_ - '|')]
				> char_('|')
				> (file | keyword);

			

			modList.name("modList");
			metaLine.name("metaLine");
			listItem.name("listItem");
			ItemType.name("ItemType");
			itemName.name("itemName");
			itemMessages.name("itemMessages");
			itemMessage.name("itemMessage");
			charString.name("charString");
			messageString.name("messageString");
			messageKeyword.name("messageKeyword");
			oldConditional.name("oldConditional");
			conditionals.name("conditional");
			andOr.name("andOr");
			conditional.name("conditional");
			condition.name("condition");
			variable.name("variable");
			file.name("file");
			version.name("version");
			
			on_error<fail>(modList,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(metaLine,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(listItem,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(ItemType,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemName,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemMessages,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemMessage,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(charString,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(messageString,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(messageKeyword,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(oldConditional,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(conditionals,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(andOr,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(conditional,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(condition,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(variable,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(file,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(version,phoenix::bind(&modlist_grammar<Iterator>::SyntaxErr, this, _1, _2, _3, _4));

		}

		typedef Skipper<Iterator> skipper;

		qi::rule<Iterator, vector<item>(), skipper> modList;
		qi::rule<Iterator, item(), skipper> listItem;
		qi::rule<Iterator, itemType(), skipper> ItemType;
		qi::rule<Iterator, fs::path(), skipper> itemName;
		qi::rule<Iterator, vector<message>(), skipper> itemMessages;
		qi::rule<Iterator, message(), skipper> itemMessage;
		qi::rule<Iterator, string(), skipper> charString, messageString, variable, file, version, andOr, keyword;
		qi::rule<Iterator, keyType(), skipper> messageKeyword;
		qi::rule<Iterator, bool(), skipper> conditional, conditionals, condition, oldConditional;
		qi::rule<Iterator, skipper> metaLine, globalMessage;
		
		void SyntaxErr(Iterator const& /*first*/, Iterator const& last, Iterator const& errorpos, boost::spirit::info const& what) {
			ostringstream out;
			out << what;
			string expect = out.str().substr(1,out.str().length()-2);
			if (expect == "eol")
				expect = "end of line";

			string context(errorpos, min(errorpos +50, last));
			boost::trim_left(context);

			LOG_ERROR("Masterlist Parsing Error: Expected a %s at \"%s\". Masterlist parsing aborted. Utility will end now.", expect.c_str(), context.c_str());
			
			expect = "&lt;" + expect + "&gt;";
			boost::replace_all(context, "\n", "<br />\n");
			string msg = (MasterlistParsingErrorFormat % expect % context).str();
			errorMessageBuffer.push_back(msg);
			return;
		}
	};
}
#endif