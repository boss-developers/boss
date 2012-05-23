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

	oldMasterlistMsgKey_::oldMasterlistMsgKey_() {
		add //Old message symbols.
			("?",SAY)
			("%",TAG)
			(":",REQ)
			("\"",INC) //Incompatibility
			("*",ERR) //FCOM install error.
		;
	}

	typeKey_::typeKey_() {
		add //Group keywords.
			("begingroup:",BEGINGROUP)  //Needs the colon there unfortunately.
			("endgroup:",ENDGROUP)  //Needs the colon there unfortunately.
			("endgroup",ENDGROUP)
			("\\begingroup\\:",BEGINGROUP)
			("\\endgroup\\\\",ENDGROUP)
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
			| lineComment
			| iniComment
			| eof;
			
		spc = space - eol;

		UTF8 = char_("\xef") >> char_("\xbb") >> char_("\xbf"); //UTF8 BOM

		CComment = "/*" >> *(char_ - "*/") >> "*/";

		iniComment = lit("#") >> *(char_ - eol);

		CPlusPlusComment = !(lit("http:") | lit("https:")) >> "//" >> *(char_ - eol);

		//Need to skip lines that start with '\', but only if they don't follow with EndGroup or BeginGroup.
		lineComment = 
			lit("\\")
			>> !(lit("EndGroup") | lit("BeginGroup"))
			>> *(char_ - eol);

		eof = *(spc | CComment | CPlusPlusComment | lineComment | eol) >> eoi;
	}

	void Skipper::SkipIniComments(const bool b) {
		if (b)
			iniComment = lit("#") >> *(char_ - eol);
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
		oldMasterlistMsgKey_ oldMasterlistMsgKey;
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
					lit(':')
					> charString
				);

		globalMessage =
			conditionals
			>> no_case[lit("global")]
			>>	(
					messageKeyword
					>> lit(':')
					>> charString
				);

		listItem %= 
			(
				oldConditional 
				| conditionals
			)
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
				>> (itemMessage | oldCondItemMessage) % +eol
			) | eps		[_1 = noMessages];

		itemMessage %= 
			(
				oldConditional 
				| conditionals
			) 
			>> ((messageSymbol | (messageKeyword >> ':'))
			>> charString)	//The double >> matters. A single > doesn't work.
			;

		oldCondItemMessage %=
			(
				unicode::string("$")		[phoenix::bind(&modlist_grammar::ConvertOldConditional, this, _1, '$')]
				| unicode::string("^")		[phoenix::bind(&modlist_grammar::ConvertOldConditional, this, _1, '^')]
			)
			>> (qi::attr(SAY)
			>> -lit(':')	//The double >> matters. A single > doesn't work.
			>> charString)	//The double >> matters. A single > doesn't work.
			;

		charString %= lexeme[+(char_ - eol)]; //String, with no skipper.

		messageKeyword %= no_case[masterlistMsgKey];

		messageSymbol %= no_case[oldMasterlistMsgKey];

		oldConditional = 
			(
				char_('>') 
				| char_('<')
			)			[phoenix::bind(&modlist_grammar::ConvertOldConditional, this, _val, _1)];

		conditionals = 
			(
				conditional								[_val = _1] 
				> *((andOr > conditional)				[_val += _1 + _2])
			)
			| eps [_val = ""];

		andOr %= 
			unicode::string("&&") 
			| unicode::string("||");

		conditional %= 
			(
				no_case[unicode::string("ifnot")
				| unicode::string("if")]
			) 
			> char_('(') 
			> condition
			> char_(')');

		condition %= 
			variable
			| version
			| (+(xdigit - '|') > char_('|') > file)
			| file
			| regexFile
			;

		variable %= char_('$') > +(char_ - ')');  //A masterlist variable, prepended by a '$' character to differentiate between vars and mods.

		file %= lexeme[char_('"') > +(char_ - '"') > char_('"')];  //An OBSE plugin or a mod plugin.

		version %=   //A version, followed by the mod it applies to.
			(char_('=') | char_('>') | char_('<'))
			> lexeme[+(char_ - '|')]
			> char_('|')
			> file;

		regexFile %= no_case[char_('r')] > file;

		modList.name("modList");
		listVar.name("listVar");
		listItem.name("listItem");
		ItemType.name("ItemType");
		itemName.name("itemName");
		itemMessages.name("itemMessages");
		itemMessage.name("itemMessage");
		charString.name("charString");
		messageKeyword.name("messageKeyword");
		messageSymbol.name("messageSymbol");
		oldConditional.name("oldConditional");
		conditionals.name("conditional");
		andOr.name("andOr");
		conditional.name("conditional");
			
		on_error<fail>(modList,				phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(listVar,				phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(listItem,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(ItemType,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(itemName,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(itemMessages,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(itemMessage,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(charString,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(messageKeyword,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(messageSymbol,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(oldConditional,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(conditionals,		phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(andOr,				phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(conditional,			phoenix::bind(&modlist_grammar::SyntaxError, this, _1, _2, _3, _4));
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

	//MF1 compatibility function. Evaluates the MF1 FCOM conditional. Like it says on the tin.
	void modlist_grammar::ConvertOldConditional(string& result, const char var) {
		switch(var) {
		case '>':
			if (parentGame->Id() == OBLIVION)
				result = "IF ($FCOM)";
			else if (parentGame->Id() == FALLOUT3)
				result = "IF ($FOOK2)";
			else if (parentGame->Id() == FALLOUTNV)
				result = "IF ($NVAMP)";
			else
				result = "";
			break;
		case '<':
			if (parentGame->Id() == OBLIVION)
				result = "IFNOT ($FCOM)";
			else if (parentGame->Id() == FALLOUT3)
				result = "IFNOT ($FOOK2)";
			else if (parentGame->Id() == FALLOUTNV)
				result = "IFNOT ($NVAMP)";
			else
				result = "";
			break;
		case '$':
			if (parentGame->Id() == OBLIVION)
				result = "IF ($OOO)";
			else if (parentGame->Id() == FALLOUT3)
				result = "IF ($FWE)";
			else if (parentGame->Id() == FALLOUTNV)
				result = "IF ($FOOK)";
			else
				result = "";
			break;
		case '^':
			if (parentGame->Id() == OBLIVION)
				result = "IF ($BC)";
			else
				result = "";
			break;
		}
	}

	//Turns a given string into a path. Can't be done directly because of the openGroups checks.
	void modlist_grammar::ToName(string& p, string itemName) {
		boost::algorithm::trim(itemName);
		if (itemName.length() == 0 && !openGroups.empty()) 
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
			| eps[_val = true];

		andOr %= unicode::string("&&") | unicode::string("||");

		ifIfNot %= no_case[unicode::string("ifnot") | unicode::string("if")];

		conditional = (ifIfNot > '(' > condition > ')')	[phoenix::bind(&conditional_grammar::EvaluateConditional, this, _val, _1, _2)];

		condition = 
			variable									[phoenix::bind(&conditional_grammar::CheckVar, this, _val, _1)]
			| version									[phoenix::bind(&conditional_grammar::CheckVersion, this, _val, _1)]
			| (hex > '|' > file)						[phoenix::bind(&conditional_grammar::CheckSum, this, _val, _1, _2)] //A CRC-32 checksum, as calculated by BOSS, followed by the file it applies to.
			| file										[phoenix::bind(&conditional_grammar::CheckFile, this, _val, _1)]
			| regexFile									[phoenix::bind(&conditional_grammar::CheckRegex, this, _val, _1)]
			;

		variable %= '$' > +(char_ - ')');  //A masterlist variable, prepended by a '$' character to differentiate between vars and mods.

		file %= lexeme['"' > +(char_ - '"') > '"'];  //An OBSE plugin or a mod plugin.

		version %=   //A version, followed by the mod it applies to.
			(char_('=') | char_('>') | char_('<'))
			> lexeme[+(char_ - '|')]
			> char_('|')
			> file;

		regexFile %= no_case['r'] > file;
		

		conditionals.name("conditional");
		andOr.name("andOr");
		conditional.name("conditional");
		condition.name("condition");
		variable.name("variable");
		file.name("file");
		version.name("version");
		regexFile.name("regex file");
			
		on_error<fail>(conditionals,	phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(andOr,			phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(conditional,		phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(condition,		phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(variable,		phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(file,			phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(version,			phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(regexFile,		phoenix::bind(&conditional_grammar::SyntaxError, this, _1, _2, _3, _4));
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
	
	void conditional_grammar::SetErrorBuffer(ParsingError * inErrorBuffer) {
		errorBuffer = inErrorBuffer;
	}

	void conditional_grammar::SetVarStore(boost::unordered_set<string> * varStore) {
		setVars = varStore;
	}

	void conditional_grammar::SetCRCStore(boost::unordered_map<string,uint32_t> * CRCStore) {
		fileCRCs = CRCStore;
	}

	void conditional_grammar::SetParentGame(const Game * game) {
		parentGame = game;
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
	void conditional_grammar::CheckVersion(bool& result, const string var) {
		result = false;
		if (parentGame == NULL)
			return;

		char comp = var[0];
		size_t pos = var.find("|") + 1;
		Version givenVersion = var.substr(1,pos-2);
		string file = var.substr(pos);

		Item tempItem(file);
		Version trueVersion;
		if (tempItem.Exists(*parentGame))
			trueVersion = tempItem.GetVersion(*parentGame);
		else {
			fs::path filePath = GetPath(file);
			if (fs::exists(filePath))
				trueVersion = Version(filePath);
			else
				return;
		}

		//Note that this string comparison is unsafe (may give incorrect result).
		switch (comp) {
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

		fs::directory_iterator iter_end;
		for (fs::directory_iterator itr(file_path); itr!=iter_end; ++itr) {
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

	//Checks if the given mod has the given checksum.
	void conditional_grammar::CheckSum(bool& result, const uint32_t sum, string file) {
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

		var %= char_("a-zA-Z_") >> *char_("a-zA-Z_0-9");

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