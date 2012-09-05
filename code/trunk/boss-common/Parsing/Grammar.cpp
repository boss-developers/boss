/*	BOSS
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of BOSS.

    BOSS is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see 
	<http://www.gnu.org/licenses/>.

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "Parsing/Grammar.h"
#include "Common/Globals.h"
#include "Common/Error.h"
#include "Support/Helpers.h"
#include "Support/Logger.h"
#include "Output/Output.h"

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <sstream>

namespace boss {
	namespace unicode = boost::spirit::unicode;
	namespace phoenix = boost::phoenix;

	using namespace qi::labels;
	using boost::algorithm::to_lower_copy;

	using qi::skip;
	using qi::eol;
	using qi::eoi;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;
	using qi::lit;
	using qi::omit;
	using qi::eps;
	using qi::hex;
	using qi::bool_;
	using qi::uint_;
	
	using unicode::char_;
	using unicode::no_case;
	using unicode::space;
	using unicode::xdigit;


	///////////////////////////////
	// Keyword structures
	///////////////////////////////

	masterlistMsgKey_::masterlistMsgKey_() {
		add //New Message keywords.
			("say",SAY)
			("tag",TAG)
			("req",REQ)
			("inc", INC)
			("dirty",DIRTY)
			("warn",WARN)
			("error",ERR)
		;
	}

	typeKey_::typeKey_() {
		add //Group keywords.
			("begingroup:", BEGINGROUP)  //Needs the colon there unfortunately.
			("endgroup:", ENDGROUP)  //Needs the colon there unfortunately.
			("endgroup", ENDGROUP)
			("mod:", MOD)  //Needs the colon there unfortunately.
			("regex:", REGEX)
		;
	}

	ruleKeys_::ruleKeys_() {
		add
			("add",ADD)
			("override",OVERRIDE)
			("for",FOR)
		;
	}

	messageKeys_::messageKeys_() {
		add
			("append",APPEND)
			("replace",REPLACE)
			("before",BEFORE)
			("after",AFTER)
			("top",TOP)
			("bottom",BOTTOM)
		;
	}

	///////////////////////////////
	//Skipper Grammars
	///////////////////////////////

	//Constructor for modlist and userlist skipper.
	Skipper::Skipper() : Skipper::base_type(start, "skipper grammar") {

		start = 
			spc
			| UTF8
			| CComment
			| CPlusPlusComment
			| iniComment
			| eof;
			
		spc = space - eol;

		UTF8 = char_("\xef") >> char_("\xbb") >> char_("\xbf"); //UTF8 BOM

		CComment = "/*" >> *(char_ - "*/") >> "*/";

		iniComment = lit("#") >> *(char_ - eol);

		CPlusPlusComment = !(lit("http:") | lit("https:") | lit("file:")) >> "//" >> *(char_ - eol);

		eof = *(spc | CComment | CPlusPlusComment | eol) >> eoi;
	}

	void Skipper::SkipIniComments(const bool b) {
		if (b)
			iniComment = (lit("#") | lit(";")) >> *(char_ - eol);
		else
			iniComment = CComment;
	}

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	//Modlist grammar constructor.
	modlist_grammar::modlist_grammar() 
		: modlist_grammar::base_type(modList, "modlist grammar"), 
		  errorBuffer(NULL) {
		masterlistMsgKey_ masterlistMsgKey;
		typeKey_ typeKey;
		const vector<Message> noMessages;  //An empty set of messages.

		modList = 
			*eol 
			>
			(
				listVar			[phoenix::bind(&modlist_grammar::StoreVar, this, _1)] 
				| globalMessage	[phoenix::bind(&modlist_grammar::StoreGlobalMessage, this, _1)] 
				| listItem		[phoenix::bind(&modlist_grammar::StoreItem, this, _val, _1)]
			) % +eol;

		listVar %=
			conditionals
			>> no_case[lit("set")]
			>>	(
					':'
					> charString
				);

		globalMessage =
			conditionals
			>> no_case[lit("global")]
			>>	(
					messageKeyword
					>> ':'
					>> charString
				);

		listItem %= 
			conditionals
			> ItemType
			> itemName
			> itemMessages;

		ItemType %= 
			no_case[typeKey]
			| eps		[_val = MOD];

		itemName = 
			charString	[phoenix::bind(&modlist_grammar::ToName, this, _val, _1)]
			| eps		[phoenix::bind(&modlist_grammar::ToName, this, _val, "")];

		itemMessages %= 
			(
				+eol
				>> itemMessage % +eol
			) | eps		[_1 = noMessages];

		itemMessage %= 
			conditionals 
			>> messageKeyword 
			>> ':'
			>> charString	//The double >> matters. A single > doesn't work.
			;

		charString %= lexeme[+(char_ - eol)]; //String, with no skipper.

		messageKeyword %= no_case[masterlistMsgKey];

		conditionals = 
			(
				conditional								[_val = _1] 
				> *((andOr > conditional)				[_val += _1 + _2])
			)
			| no_case[unicode::string("else")][_val = _1]
			| eps [_val = ""];

		andOr %= 
			unicode::string("&&") 
			| unicode::string("||");

		conditional %= 
			(
				no_case[unicode::string("ifnot")
				| unicode::string("if")]
			) 
			> functCondition;
			
		functCondition %=
			(
				no_case[unicode::string("var")] > char_('(') > variable > char_(')')													//Variable condition.
			) | (
				no_case[unicode::string("file")] > char_('(') > file > char_(')')														//File condition.
			) | (
				no_case[unicode::string("checksum")] > char_('(') > file > char_(',') > checksum > char_(')')							//Checksum condition.
			) | (
				no_case[unicode::string("version")] > char_('(') > file > char_(',') > version > char_(',') > comparator > char_(')')	//Version condition.
			) | (
				no_case[unicode::string("regex")] > char_('(') > regex > char_(')')														//Regex condition.
			) | (
				no_case[unicode::string("active")] > char_('(') > file > char_(')')														//Active condition.
			) | (
				no_case[unicode::string("lang")] > char_('(') > language > char_(')')													//Language condition.
			)
			;

		variable %= +(char_ - (')' | eol)); 

		file %= lexeme[char_('"') > +(char_ - ('"' | eol)) > char_('"')];

		checksum %= +xdigit;

		version %= file;

		comparator %= char_('=') | char_('>') | char_('<');

		regex %= file;

		language %= file;


		modList.name("modList");
		listVar.name("listVar");
		listItem.name("listItem");
		ItemType.name("ItemType");
		itemName.name("itemName");
		itemMessages.name("itemMessages");
		itemMessage.name("itemMessage");
		charString.name("charString");
		messageKeyword.name("messageKeyword");
		conditionals.name("conditional");
		andOr.name("andOr");
		conditional.name("conditional");
		functCondition.name("condition");
		shortCondition.name("condition");
		variable.name("variable");
		file.name("file");
		checksum.name("checksum");
		version.name("version");
		comparator.name("comparator");
		regex.name("regex file");
		file.name("language");
			
		on_error<fail>(modList,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(listVar,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(listItem,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(ItemType,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(itemName,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(itemMessages,	phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(itemMessage,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(charString,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(messageKeyword,	phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(conditionals,	phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(andOr,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(conditional,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(functCondition,	phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(shortCondition,	phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(variable,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(file,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(checksum,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(version,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(comparator,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(regex,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(language,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
	}

	void modlist_grammar::SetErrorBuffer(ParsingError * inErrorBuffer) { 
		errorBuffer = inErrorBuffer; 
	}

	void modlist_grammar::SetGlobalMessageBuffer(vector<Message> * inGlobalMessageBuffer) { 
		globalMessageBuffer = inGlobalMessageBuffer; 
	}

	void modlist_grammar::SetVarStore(vector<MasterlistVar> * varStore) { 
		setVars = varStore; 
	}

	void modlist_grammar::SetCRCStore(boost::unordered_map<string,uint32_t> * CRCStore) {
		fileCRCs = CRCStore; 
	}

	void modlist_grammar::SetParentGame(const Game * game) {
		parentGame = game;
	}

	//Parser error reporter.
	void modlist_grammar::SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, boost::spirit::info const& what) {
		if (errorBuffer == NULL || !errorBuffer->Empty())
			return;
		
		ostringstream out;
		out << what;
		string expect = out.str();

		string context(errorpos, min(errorpos +50, last));
		boost::trim_left(context);

		ParsingError e(str(MasterlistParsingErrorHeader % expect), context, MasterlistParsingErrorFooter);
		*errorBuffer = e;
		LOG_ERROR(Outputter(PLAINTEXT, e).AsString().c_str());
		return;
	}

	//Stores the given item and records any changes to open groups.
	void modlist_grammar::StoreItem(vector<Item>& list, Item currentItem) {
		if (currentItem.Type() == BEGINGROUP)
			openGroups.push_back(currentItem.Name());
		else if (currentItem.Type() == ENDGROUP)
			openGroups.pop_back();
		if (!currentItem.Name().empty())  //A blank line can be confused with a mod entry. 
			list.push_back(currentItem);
	}

	//Stores the masterlist variable.
	void modlist_grammar::StoreVar(const MasterlistVar var) {
		setVars->push_back(var);
	}

	//Stores the global message.
	void modlist_grammar::StoreGlobalMessage(const Message message) {
		globalMessageBuffer->push_back(message);
	}

	//Turns a given string into a path. Can't be done directly because of the openGroups checks.
	void modlist_grammar::ToName(string& p, string itemName) {
		boost::algorithm::trim(itemName);
		if (itemName.empty() && !openGroups.empty()) 
			p = openGroups.back();
		else
			p = itemName;
	}


	////////////////////////////
	// Conditional Grammar
	////////////////////////////

	//Conditional grammar constructor.
	conditional_grammar::conditional_grammar() : conditional_grammar::base_type(conditionals, "modlist grammar") {

		conditionals = 
			(conditional[_val = _1] 
			> *((andOr > conditional)			[phoenix::bind(&conditional_grammar::EvaluateCompoundConditional, this, _val, _1, _2)]))
			| no_case[unicode::string("else")]	[phoenix::bind(&conditional_grammar::EvalElseConditional, this, _val, _pass)]
			| eps[_val = true];

		andOr %= unicode::string("&&") | unicode::string("||");

		ifIfNot %= no_case[unicode::string("ifnot") | unicode::string("if")];

		conditional = (ifIfNot > condition)									[phoenix::bind(&conditional_grammar::EvaluateConditional, this, _val, _1, _2)];

		condition =
			(no_case[lit("var")] > '(' > variable > ')')									[phoenix::bind(&conditional_grammar::CheckVar, this, _val, _1)]
			| 
			(no_case[lit("file")] > '(' > file > ')')										[phoenix::bind(&conditional_grammar::CheckFile, this, _val, _1)]
			| 
			(no_case[lit("checksum")] > '(' > file > ',' > checksum > ')')					[phoenix::bind(&conditional_grammar::CheckSum, this, _val, _1, _2)]
			| 
			(no_case[lit("version")] > '(' > file > ',' > version > ',' > comparator > ')')	[phoenix::bind(&conditional_grammar::CheckVersion, this, _val, _1, _2, _3)]
			| 
			(no_case[lit("regex")] > '(' > regex > ')')										[phoenix::bind(&conditional_grammar::CheckRegex, this, _val, _1)]
			| 
			(no_case[lit("active")] > '(' > file > ')')										[phoenix::bind(&conditional_grammar::CheckActive, this, _val, _1)]
			| 
			(no_case[lit("lang")] > '(' > language > ')')									[phoenix::bind(&conditional_grammar::CheckLanguage, this, _val, _1)]
			;

		variable %= +(char_ - (')' | eol)); 

		file %= lexeme['"' > +(char_ - ('"' | eol)) > '"'];

		checksum %= hex;

		version %= file;

		comparator %= char_('=') | char_('>') | char_('<');

		regex %= file;

		language %= file;
		

		conditionals.name("conditional");
		andOr.name("andOr");
		conditional.name("conditional");
		functCondition.name("condition");
		shortCondition.name("condition");
		variable.name("variable");
		file.name("file");
		checksum.name("checksum");
		version.name("version");
		comparator.name("comparator");
		regex.name("regex file");
		language.name("language");
			
		on_error<fail>(conditionals,	phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(andOr,			phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(conditional,		phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(functCondition,	phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(shortCondition,	phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(variable,		phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(file,			phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(checksum,		phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(version,			phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(comparator,		phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(regex,			phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(language,		phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
	}

	//Evaluate a single conditional.
	void conditional_grammar::EvaluateConditional(bool& result, const string type, const bool condition) {
		if (to_lower_copy(type) == "if")
			result = condition;
		else
			result = !condition;
		return;
	}

	//Evaluate the second half of a complex conditional.
	void conditional_grammar::EvaluateCompoundConditional(bool& lhsCondition, const string andOr, const bool rhsCondition) {
		if (andOr == "||" && !lhsCondition && !rhsCondition)
			lhsCondition = false;
		else if (andOr == "||" && !lhsCondition && rhsCondition)
			lhsCondition = true;
		else if (andOr == "&&" && lhsCondition && !rhsCondition)
			lhsCondition = false;
	}
	
	void conditional_grammar::EvalElseConditional(bool& result, bool& ok) {
		if (lastResult == NULL) {
			ok = false;
			result = false;
		} else
			result = !(*lastResult);
	}

	void conditional_grammar::SetErrorBuffer(ParsingError * inErrorBuffer) {
		errorBuffer = inErrorBuffer;
	}

	void conditional_grammar::SetVarStore(boost::unordered_set<string> * varStore) {
		setVars = varStore;
	}

	void conditional_grammar::SetCRCStore(boost::unordered_map<string,uint32_t> * CRCStore) {
		fileCRCs = CRCStore;
	}

	void conditional_grammar::SetActivePlugins(boost::unordered_set<string> * plugins) {
		activePlugins = plugins;
	}

	void conditional_grammar::SetParentGame(const Game * game) {
		parentGame = game;
	}

	void conditional_grammar::SetLastConditionalResult(bool * result) {
		lastResult = result;
	}

	//Returns the true path based on what type of file or keyword it is.
	fs::path conditional_grammar::GetPath(const string file) {
		if (file == "OBSE" || file == "FOSE" || file == "NVSE" || file == "SKSE" || file == "MWSE")
			return parentGame->SEExecutable();
		else if (file == "TES3" || file == "TES4" || file == "TES5" || file == "FO3" || file == "FONV")
			return parentGame->Executable();
		else if (file == "BOSS")
			return boss_path / "BOSS.exe";
		else if (boost::iequals(fs::path(file).extension().string(), ".dll") && file.find('/') == string::npos && file.find('\\') == string::npos && fs::exists(parentGame->SEPluginsFolder()))
			return parentGame->SEPluginsFolder() / file;
		else
			return parentGame->DataFolder() / file;
	}

	//Checks if the given file (plugin or dll/exe) has a version for which the comparison holds true.
	void conditional_grammar::CheckVersion(bool& result, const string file, const string version, const char comparator) {
		result = false;
		if (parentGame == NULL)
			return;

		fs::path filePath = GetPath(file);
		if (!fs::exists(filePath)) {
			if (comparator == '<')
				result = true;
			return;
		}
		
		Version givenVersion = Version(version);
		Version trueVersion;
		if (Item(file).IsPlugin())
			trueVersion = Item(file).GetVersion(*parentGame);
		else
			trueVersion = Version(filePath);

		switch (comparator) {
		case '>':
			if (trueVersion > givenVersion)
				result = true;
			break;
		case '<':
			if (trueVersion < givenVersion)
				result = true;
			break;
		case '=':
			if (trueVersion == givenVersion)
				result = true;
			break;
		}
		return;
	}

	//Checks if the given file exists.
	void conditional_grammar::CheckFile(bool& result, string file) {
		result = false;
		if (parentGame == NULL)
			return;
		result = fs::exists(GetPath(file));
	}

	//Checks if a file which matches the given regex exists.
	//This might not work when the regex specifies a file and a path, eg. "path/to/file.txt", because characters like '.' need to be escaped in regex
	//so the regex would be "path/to/file\.txt". boost::filesystem might interpret that as a path of "path / to / file / .txt" though.
	//In windows, the above path would be "path\to\file.txt", which would become "path\\to\\file\.txt" in regex. Basically, the extra backslashes need to
	//be removed when getting the path and filename.
	void conditional_grammar::CheckRegex(bool& result, string reg) {
		result = false;
		fs::path file_path;
		//If the regex includes '/' or '\\' then it includes folders. Need to split the regex into the parent path and the filename.
		//No reason for the regex to include both.
		if (reg.find("/") != string::npos) {
			size_t pos1 = reg.rfind("/");
			string p = reg.substr(0,pos1);
			reg = reg.substr(pos1+1);
			file_path = fs::path(p);
		} else if (reg.find("\\\\") != string::npos) {
			size_t pos1 = reg.rfind("\\\\");
			string p = reg.substr(0,pos1);
			reg = reg.substr(pos1+2);
			boost::algorithm::replace_all(p,"\\\\","\\");
			file_path = fs::path(p);
		} else if (to_lower_copy(fs::path(reg).extension().string()) == ".dll" && fs::exists(parentGame->SEPluginsFolder()))
			file_path = parentGame->SEPluginsFolder();
		else
			file_path = parentGame->DataFolder();
		boost::regex regex;
		try {
			regex = boost::regex(reg, boost::regex::extended|boost::regex::icase);
		} catch (boost::regex_error e) {
			LOG_ERROR("\"%s\" is not a valid regular expression. Item skipped.", reg.c_str());
			result = false;  //Fail the check.
			return;
		}

		for (fs::directory_iterator itr(file_path); itr!=fs::directory_iterator(); ++itr) {
			if (fs::is_regular_file(itr->status())) {
				if (boost::regex_match(itr->path().filename().string(),regex)) {
					result = true;
					break;
				}
			}
		}
	}

	//Checks if a masterlist variable is defined.
	void conditional_grammar::CheckVar(bool& result, const string var) {
		if (setVars->find(var) == setVars->end())
			result = false;
		else
			result = true;
		return;
	}

	//Checks if the given plugin is active.
	void conditional_grammar::CheckActive(bool& result, const string plugin) {
		if (activePlugins->find(to_lower_copy(plugin)) != activePlugins->end())
			result = true;
		else
			result = false;
	}

	//Checks if the given language is the current language.
	void conditional_grammar::CheckLanguage(bool& result, const string language) {
		if (boost::iequals(language, "english"))
			result = (gl_language == ENGLISH);
		else if (boost::iequals(language, "russian"))
			result = (gl_language == RUSSIAN);
		else if (boost::iequals(language, "german"))
			result = (gl_language == GERMAN);
		else if (boost::iequals(language, "spanish"))
			result = (gl_language == SPANISH);
		else if (boost::iequals(language, "chinese"))
			result = (gl_language == SIMPCHINESE);
		else
			result = false;
	}

	//Checks if the given mod has the given checksum.
	void conditional_grammar::CheckSum(bool& result, string file, const uint32_t sum) {
		result = false;
		if (parentGame == NULL)
			return;
		uint32_t CRC;
		fs::path file_path = GetPath(file);
		boost::unordered_map<string,uint32_t>::iterator iter = fileCRCs->find(file);

		if (iter != fileCRCs->end()) {
			CRC = fileCRCs->at(file);
		} else {
			if (fs::exists(file_path))
				CRC = GetCrc32(file_path);
			else if (Item(file).IsGhosted(*parentGame))
				CRC = GetCrc32(fs::path(file_path.string() + ".ghost"));
			else 
				return;

			fileCRCs->emplace(file,CRC);
		}

		if (sum == CRC)
			result = true;
		return;
	}

	//Parser error reporter.
	void conditional_grammar::SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, boost::spirit::info const& what) {
		if (errorBuffer == NULL || !errorBuffer->Empty())
			return;
		
		ostringstream out;
		out << what;
		string expect = out.str();

		string context(errorpos, min(errorpos +50, last));
		boost::trim_left(context);

		ParsingError e(str(MasterlistParsingErrorHeader % expect), context, MasterlistParsingErrorFooter);
		*errorBuffer = e;
		LOG_ERROR(Outputter(PLAINTEXT, e).AsString().c_str());
		return;
	}


	////////////////////////////
	//Ini Grammar.
	////////////////////////////

	ini_grammar::ini_grammar() 
		: ini_grammar::base_type(ini, "ini grammar"), 
		  errorBuffer(NULL) {

		ini %= *eol
				> (omit[heading] | (!lit('[') >> setting)) % +eol
				> *eol;

		heading = '[' > +(char_ - ']') > ']';

		setting %= var > '=' > stringVal;

		var %= +(char_ - '=');

		stringVal %= lexeme[*(char_ - eol)];
		
		//Give each rule names.
		ini.name("ini");
		heading.name("heading");
		setting.name("setting");
		var.name("variable");
		stringVal.name("string value");
		
		//Error handling.
		on_error<fail>(ini,			phoenix::bind(&ini_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(heading,		phoenix::bind(&ini_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(setting,		phoenix::bind(&ini_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(var,			phoenix::bind(&ini_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(stringVal,	phoenix::bind(&ini_grammar::SyntaxError,this,_1,_2,_3,_4));
	}

	void ini_grammar::SetErrorBuffer(ParsingError * inErrorBuffer) { 
		errorBuffer = inErrorBuffer; 
	}

	void ini_grammar::SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, info const& what) {
		if (errorBuffer == NULL || !errorBuffer->Empty())
			return;
		
		ostringstream out;
		out << what;
		string expect = out.str();

		string context(errorpos, min(errorpos +50, last));
		boost::trim_left(context);

		ParsingError e(str(IniParsingErrorHeader % expect), context, IniParsingErrorFooter);
		*errorBuffer = e;
		LOG_ERROR(Outputter(PLAINTEXT, e).AsString().c_str());
		return;
	}

	////////////////////////////
	//RuleList Grammar.
	////////////////////////////

	userlist_grammar::userlist_grammar() 
		: userlist_grammar::base_type(ruleList, "userlist grammar"), 
		  errorBuffer(NULL) {

		ruleKeys_ ruleKeys;
		messageKeys_ sortOrMessageKeys;

		//A list is a vector of rules. Rules are separated by line endings.
		ruleList %= 
			*eol 
			> (eoi | (userlistRule % eol)); 

		//A rule consists of a rule line containing a rule keyword and a rule object, followed by one or more message or sort lines.
		userlistRule %=
			*eol
			> stateKey > ruleKey > ':' > object
			> +eol
			> sortOrMessageLine % +eol;

		sortOrMessageLine %=
			sortOrMessageKey
			> ':'
			> object;

		object %= lexeme[+(char_ - eol)]; //String, with no skipper.

		ruleKey %= no_case[ruleKeys];

		sortOrMessageKey %= no_case[sortOrMessageKeys];

		stateKey = no_case[lit("disable")][_val = false] | eps[_val = true];

		//Give each rule names.
		ruleList.name("rules");
		userlistRule.name("rule");
		sortOrMessageLine.name("sort or message line");
		object.name("line object");
		ruleKey.name("rule keyword");
		sortOrMessageKey.name("sort or message keyword");
		stateKey.name("state keyword");
		
		on_error<fail>(ruleList,			phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(userlistRule,		phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(sortOrMessageLine,	phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(object,				phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(ruleKey,				phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(sortOrMessageKey,	phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(stateKey,			phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
	}

	void userlist_grammar::SetErrorBuffer(vector<ParsingError> * inErrorBuffer) { 
		errorBuffer = inErrorBuffer; 
	}

	void userlist_grammar::SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, info const& what) {
		if (errorBuffer == NULL || !errorBuffer->empty())
			return;
		
		ostringstream out;
		out << what;
		string expect = out.str();

		string context(errorpos, min(errorpos +50, last));
		boost::trim_left(context);

		ParsingError e(str(RuleListParsingErrorHeader % expect), context, RuleListParsingErrorFooter);
		if (errorBuffer != NULL)
			errorBuffer->push_back(e);
		LOG_ERROR(Outputter(PLAINTEXT, e).AsString().c_str());
		return;
	}
}