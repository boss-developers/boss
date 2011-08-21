/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "Parsing/Grammar.h"
#include "Common/Globals.h"
#include "Support/Helpers.h"
#include "Support/Logger.h"
#include "Common/BOSSLog.h"

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/algorithm/string.hpp>

#include <sstream>

namespace boss {
	namespace unicode = boost::spirit::unicode;
	namespace phoenix = boost::phoenix;

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
	using qi::hex;
	
	using unicode::char_;
	using unicode::no_case;
	using unicode::space;
	using unicode::xdigit;

	///////////////////////////////
	//Common Functions
	///////////////////////////////

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
		} else if (file == "SKSE") {
			file_path = "..";
			file = "skse_loader.exe";
		} else if (file == "BOSS") {
			file_path = ".";
			file = "BOSS.exe";
		} else if (file == "TES4") {
			file_path = "..";
			file = "Oblivion.exe";
		} else if (file == "TES5") {
			file_path = "..";
			file = "Skyrim.exe";
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
				else if (fs::exists(data_path / "NVSE"))
					file_path = data_path / fs::path("NVSE/Plugins");  //Fallout: New Vegas - NVSE plugins.
				else if (fs::exists(data_path / "SKSE"))
					file_path = data_path / fs::path("SKSE/Plugins");  //Fallout: New Vegas - NVSE plugins.
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
		boost::unordered_map<string,unsigned int>::iterator iter = fileCRCs.find(file);

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

	//Evaluate part of a shorthand conditional message.
	void EvaluateConditionalMessage(string& message, string version, string file, string mod) {
		fs::path file_path;
		GetPath(file_path,file);
		if (fs::exists(file_path / file)) {  //File exists. Was a version or checksum given? 
			if (!version.empty()) {
				if (version[0] == '>' || version[0] == '=' || version[0] == '<') {  //Version
					bool versionCheck;
					CheckVersion(versionCheck, version + "|" + file);
					if (versionCheck)
						return;	//Mod is of the correct version. No message should be printed for this item.
				} else {
					bool sumCheck;
					CheckSum(sumCheck, HexStringToInt(version), file);
					if (sumCheck)
						return;	//Mod is of the correct checksum. No message should be printed for this item.
				}
			}
			if (!message.empty())
				message += ", ";
			if (!mod.empty())  //Was a valid mod name given?
				message += "\"" + mod + "\"";
			else
				message += "\"" + file + "\"";
		}
	}

	//Converts a hex string to an integer using BOOST's Spirit.Qi. Faster than a stringstream conversion.
	unsigned int HexStringToInt(string str) {
		string::const_iterator begin, end;
		begin = str.begin();
		end = str.end();
		unsigned int out;
		qi::parse(begin, end, hex[phoenix::ref(out) = _1]);
		return out;
	}


	///////////////////////////////
	//Skipper Grammars
	///////////////////////////////
	
	//Constructor for modlist and userlist skipper.
	Skipper::Skipper() : Skipper::base_type(start, "Skipper") {

		start = 
			spc
			| UTF8
			| CComment
			| CPlusPlusComment
			| lineComment
			| eof;
			
		spc = space - eol;

		UTF8 = char_("\xef") >> char_("\xbb") >> char_("\xbf"); //UTF8 BOM

		CComment = "/*" >> *(char_ - "*/") >> "*/";

		CPlusPlusComment = !(lit("http:") | lit("https:")) >> "//" >> *(char_ - eol);

		//Need to skip lines that start with '\', but only if they don't follow with EndGroup or BeginGroup.
		lineComment = 
			lit("\\")
			>> !(lit("EndGroup") | lit("BeginGroup"))
			>> *(char_ - eol);

		eof = *(spc | CComment | CPlusPlusComment | lineComment | eol) >> eoi;
	}

	//Constructor for ini skipper.
	Ini_Skipper::Ini_Skipper() : Ini_Skipper::base_type(start, "Ini Skipper") {

		start = 
			spc
			| UTF8
			| comment
			| eof;
			
		spc = space - eol;

		UTF8 = char_("\xef") >> char_("\xbb") >> char_("\xbf"); //UTF8 BOM

		comment	= 
			lit("#") 
			>> *(char_ - eol);

		eof = *(spc | comment | eol) >> eoi;
	}

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	bool storeItem = true;
	bool storeMessage = true;  //Should the current item/message be stored.
	vector<string> openGroups;  //Need to keep track of which groups are open to match up endings properly in MF1.

	//Stores a message, should it be appropriate.
	void StoreMessage(vector<message>& messages, message currentMessage) {
		if (storeMessage && !currentMessage.data.empty())
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
	void StoreVar(string var) {
		if (storeItem)
			setVars.insert(var);
		return;
	}

	//Stores the global message.
	void StoreGlobalMessage(message currentMessage) {
		if (storeMessage)
			globalMessageBuffer.push_back(currentMessage);
		return;
	}

	//MF1 compatibility function. Evaluates the MF1 FCOM conditional. Like it says on the tin.
	void EvalOldFCOMConditional(bool& result, char var) {
		result = false;
		boost::unordered_set<string>::iterator pos = setVars.find("FCOM");
		if (var == '>' && pos != setVars.end())
				result = true;
		else if (var == '<' && pos == setVars.end())
				result = true;
		return;
	}

	//MF1 compatibility function. Evaluates the MF1 OOO/BC conditional message symbols.
	void EvalMessKey(keyType key) {
		if (key == OOOSAY) {
			boost::unordered_set<string>::iterator pos = setVars.find("OOO");
			if (pos == setVars.end())
				storeMessage = false;
		} else if (key == BCSAY) {
			boost::unordered_set<string>::iterator pos = setVars.find("BC");
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

	modlist_grammar::modlist_grammar() : modlist_grammar::base_type(modList, "modlist_grammar") {

		vector<message> noMessages;  //An empty set of messages.
		string emptyStr;

		modList = (metaLine[phoenix::bind(&StoreVar, _1)] | globalMessage[phoenix::bind(&StoreGlobalMessage, _1)] | listItem[phoenix::bind(&StoreItem, _val, _1)]) % +eol;

		metaLine =
			omit[conditionals[phoenix::ref(storeItem) = _1]]
			>> no_case[lit("set")]
			>> (lit(":")
			> charString);

		globalMessage =
			omit[conditionals[phoenix::ref(storeMessage) = _1]]
			>> no_case[lit("global")]
			>> (messageKeyword
			> lit(":")
			> messageString);

		listItem %= 
			omit[(oldConditional | conditionals)[phoenix::ref(storeItem) = _1]]
			> ItemType
			> itemName
			> itemMessages;

		ItemType %= no_case[typeKey] | eps[_val = MOD];

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

		charString %= lexeme[+(char_ - eol)]; //String, with no skipper.

		messageString = 
			((messageVersionCRC
			>> file 
			>> messageModString
			)[phoenix::bind(&EvaluateConditionalMessage, _val, _1, _2, _3)] % (lit("|") | lit(",")))	//Conditional message.
			| charString[_val = _1];				//Any other message

		messageModString %=
			(lit("=") >> file) 
			| eps;

		messageVersionCRC %=
			('\"' >> (
					((char_('=') | char_('>') | char_('<')) >> lexeme[+(char_ - '\"')]) 
					| xdigit)  
			>> '\"' >> lit(":")) 
			| eps;

		messageKeyword %= no_case[masterlistMsgKey];

		oldConditional = (char_(">") |  char_("<"))		[phoenix::bind(&EvalOldFCOMConditional, _val, _1)];

		conditionals = 
			(conditional[_val = _1] 
			> *((andOr > conditional)			[phoenix::bind(&EvaluateCompoundConditional, _val, _1, _2)]))
			| eps[_val = true];

		andOr %= unicode::string("&&") | unicode::string("||");

		conditional = (no_case[metaKey] > '(' > condition > ')')	[phoenix::bind(&EvaluateConditional, _val, _1, _2)];

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
		messageVersionCRC.name("conditional shorthand version/CRC");
		messageModString.name("conditional shorthand mod");
			
		on_error<fail>(modList,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(metaLine,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(listItem,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(ItemType,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(itemName,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(itemMessages,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(itemMessage,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(charString,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(messageString,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(messageVersionCRC,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(messageModString,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(messageKeyword,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(oldConditional,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(conditionals,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(andOr,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(conditional,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(condition,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(variable,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(file,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
		on_error<fail>(version,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));

	}

	void modlist_grammar::SyntaxErr(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, boost::spirit::info const& what) {
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
		masterlistErrorBuffer.push_back(msg);
		return;
	}


	////////////////////////////
	//Ini Grammar.
	////////////////////////////

	string currentHeading;

	//Set the BOSS variable values while parsing.
	void SetVar(string var, string value) {
			int intVal;
			bool bval;
			if (currentHeading == "GUI.LastOptions" || currentHeading == "GUI.Settings")
				return;
			if (currentHeading == "BOSS.InternetSettings") {
				if (var == "ProxyType")
					proxy_type = value;
				else if (var == "ProxyHostname")
					proxy_host = value;
				else if (var == "ProxyPort")
					proxy_port = value;
				return;
			} else if (currentHeading == "BOSS.RunOptions") {
				if (var == "Game") {
					if (value == "Oblivion")
						game = 1;
					else if (value == "Nehrim")
						game = 3;
					else if (value == "Fallout3")
						game = 2;
					else if (value == "FalloutNV")
						game = 4;
					else if (value == "Skyrim")
						game = 5;
					return;
				} else if (var == "BOSSlogFormat") {
					if (value == "html" || value == "text")
						log_format = value;
					return;
				} else if (var == "ProxyType")
					proxy_type = value;
				else if (var == "ProxyHostname")
					proxy_host = value;
				else if (var == "ProxyPort")
					proxy_port = value;

				intVal = atoi(value.c_str());
				if (intVal == 0)
					bval = false;
				else
					bval = true;

				if (var == "UpdateMasterlist")
					update = bval;
				else if (var == "OnlyUpdateMasterlist")
					update_only = bval;
				else if (var == "DisableMasterlistUpdate") {
					if (bval)
						update = false;
				} else if (var == "SilentRun")
					silent = bval;
				else if (var == "NoVersionParse")
					skip_version_parse = bval;
				else if (var == "Debug")
					debug = bval;
				else if (var == "DisplayCRCs")
					show_CRCs = bval;
				else if (var == "DoTrialRun")
					trial_run = bval;
				else if (var == "RevertLevel") {
					if (intVal >= 0 && intVal < 3)
						revert = intVal;
				} else if (var == "CommandLineVerbosity") {
					if (intVal >= 0 && intVal < 4)
						verbosity = intVal;
				}
				return;
			} else if (currentHeading == "BOSSlog.Filters") {
				intVal = atoi(value.c_str());
				if (intVal == 0)
					bval = false;
				else
					bval = true;
				if (var == "UseDarkColourScheme") {
					UseDarkColourScheme = bval;
				} else if (var == "HideVersionNumbers") {
					HideVersionNumbers = bval;
				} else if (var == "HideGhostedLabel") {
					HideGhostedLabel = bval;
				} else if (var == "HideChecksums") {
					HideChecksums = bval;
				} else if (var == "HideMessagelessMods") {
					HideMessagelessMods = bval;
				} else if (var == "HideGhostedMods") {
					HideGhostedMods = bval;
				} else if (var == "HideCleanMods") {
					HideCleanMods = bval;
				} else if (var == "HideRuleWarnings") {
					HideRuleWarnings = bval;
				} else if (var == "HideAllModMessages") {
					HideAllModMessages = bval;
				} else if (var == "HideNotes") {
					HideNotes = bval;
				} else if (var == "HideBashTagSuggestions") {
					HideBashTagSuggestions = bval;
				} else if (var == "HideRequirements") {
					HideRequirements = bval;
				} else if (var == "HideIncompatibilities") {
					HideIncompatibilities = bval;
				}
				return;
			} else if (currentHeading == "BOSSlog.Styles") {
				if (value == "")
					return;
				else if (var == "body")
					CSSBody = value;
				else if (var == ".filters")
					CSSFilters = value;
				else if (var == ".filters > li")
					CSSFiltersList = value;
				else if (var == "body > div:first-child")
					CSSTitle = value;
				else if (var == "body > div:first-child + div")
					CSSCopyright = value;
				else if (var == "body > div")
					CSSSections = value;
				else if (var == "body > div > span:first-child")
					CSSSectionTitle = value;
				else if (var == "body > div > span:first-child > span")
					CSSSectionPlusMinus = value;
				else if (var == "div > ul")
					CSSTopLevelList = value;
				else if (var == "body > div:last-child")
					CSSLastSection = value;
				else if (var == "body > div:last-child > span:first-child")
					CSSLastSectionTitle = value;
				else if (var == "div > ul > li")
					CSSTopLevelListItem = value;
				else if (var == "ul")
					CSSList = value;
				else if (var == "ul li")
					CSSListItem = value;
				else if (var == "li ul")
					CSSItemList = value;
				else if (var == "input[type='checkbox']")
					CSSCheckbox = value;
				else if (var == "blockquote")
					CSSBlockquote = value;
				else if (var == "#unrecognised > li")
					CSSUnrecognisedList = value;
				else if (var == "#summary > div")
					CSSSummaryRow = value;
				else if (var == "#summary > div > div")
					CSSSummaryCell = value;
				else if (var == ".error")
					CSSError = value;
				else if (var == ".warn")
					CSSWarning = value;
				else if (var == ".success")
					CSSSuccess = value;
				else if (var == ".version")
					CSSVersion = value;
				else if (var == ".ghosted")
					CSSGhost = value;
				else if (var == ".crc")
					CSSCRC = value;
				else if (var == ".tagPrefix")
					CSSTagPrefix = value;
				else if (var == ".dirty")
					CSSDirty = value;
				else if (var == ".message")
					CSSQuotedMessage = value;
				else if (var == ".mod")
					CSSMod = value;
				else if (var == ".tag")
					CSSTag = value;
				else if (var == ".note")
					CSSNote = value;
				else if (var == ".req")
					CSSRequirement = value;
				else if (var == ".inc")
					CSSIncompatibility = value;
			}
		}

	ini_grammar::ini_grammar() : ini_grammar::base_type(ini, "ini grammar") {

		ini =
			section % +eol;

		section =
			heading[phoenix::ref(currentHeading) = _1]
			> +eol
			> setting % +eol;

		heading %= 
			lit("[")
			>> (+(char_ - "]"))
			>> lit("]");

		setting =
			(var
			> '='
			> value)[phoenix::bind(&SetVar, _1, _2)];

		var %=
			lexeme[(lit("\"") >> +(char_ - lit("\"")) >> lit("\""))]
			|
			(!lit("[") >> +(char_ - '='));

		value %=
			(lit("{") >> lexeme[*(char_ - lit("}"))] >> lit("}"))
			|
			+(char_ - eol);

		//Give each rule names.
		ini.name("ini");
		section.name("section");
		heading.name("heading");
		setting.name("setting");
		var.name("variable");
		value.name("value");
		
		//Error handling.
		on_error<fail>(ini,phoenix::bind(&ini_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(section,phoenix::bind(&ini_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(heading,phoenix::bind(&ini_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(setting,phoenix::bind(&ini_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(var,phoenix::bind(&ini_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(value,phoenix::bind(&ini_grammar::SyntaxError,this,_1,_2,_3,_4));
	}

	void ini_grammar::SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, info const& what) {
		ostringstream out;
		out << what;
		string expect = out.str().substr(1,out.str().length()-2);
		if (expect == "eol")
			expect = "end of line";

		string context(errorpos, min(errorpos +50, last));
		boost::trim_left(context);

		LOG_ERROR("Ini Parsing Error: Expected a %s at \"%s\". Ini parsing aborted. No further settings will be applied.", expect.c_str(), context.c_str());
			
		expect = "&lt;" + expect + "&gt;";
		boost::replace_all(context, "\n", "<br />\n");
		string msg = (IniParsingErrorFormat % expect % context).str();
		iniErrorBuffer.push_back(msg);
		return;
	}

	////////////////////////////
	//Userlist Grammar.
	////////////////////////////

	bool storeLine = true;  //Should the current message/sort line be stored.

	failure::failure(keyType const& ruleKey, string const& ruleObject, string const& message) 
			: ruleKey(ruleKey), ruleObject(ruleObject), message(message) {}

	void AddSyntaxError(keyType const& rule, string const& object, string const& message) {
		string keystring = KeyToString(rule);
		string const msg = (SyntaxErrorFormat % keystring % object % message).str();
		userlistErrorBuffer.push_back(msg);
		return;
	}

	//Rule checker function, checks for syntax (not parsing) errors.
	void RuleSyntaxCheck(vector<rule>& userlist, rule currentRule) {
		bool skip = false;
		try {
			keyType ruleKey = currentRule.ruleKey;
			string ruleObject = currentRule.ruleObject;
			if (IsPlugin(ruleObject)) {
				if (ruleKey != FOR && IsMasterFile(ruleObject))
					throw failure(ruleKey, ruleObject, ESortingMasterEsm.str());
			} else {
				if (Tidy(ruleObject) == "esms")
					throw failure(ruleKey, ruleObject, ESortingGroupEsms.str());
				if (ruleKey == ADD && !IsPlugin(ruleObject))
					throw failure(ruleKey, ruleObject, EAddingModGroup.str());
				else if (ruleKey == FOR)
					throw failure(ruleKey, ruleObject, EAttachingMessageToGroup.str());
			}
			for (size_t i=0; i<currentRule.lines.size(); i++) {
				keyType key = currentRule.lines[i].key;
				string subject = currentRule.lines[i].object;
				if (key == BEFORE || key == AFTER) {
					if (ruleKey == FOR)
						throw failure(ruleKey, ruleObject, ESortLineInForRule.str());
					if ((IsPlugin(ruleObject) && !IsPlugin(subject)) || (!IsPlugin(ruleObject) && IsPlugin(subject)))
						throw failure(ruleKey, ruleObject, EReferencingModAndGroup.str());
					if (key == BEFORE) {
						if (Tidy(subject) == "esms")
							throw failure(ruleKey, ruleObject, ESortingGroupBeforeEsms.str());
						else if (IsMasterFile(subject))
							throw failure(ruleKey, ruleObject, ESortingModBeforeGameMaster.str());
					}
				} else if (key == TOP || key == BOTTOM) {
					if (ruleKey == FOR)
						throw failure(ruleKey, ruleObject, ESortLineInForRule.str());
					if (key == TOP && Tidy(subject) == "esms")
						throw failure(ruleKey, ruleObject, EInsertingToTopOfEsms.str());
					if (!IsPlugin(ruleObject) || IsPlugin(subject))
						throw failure(ruleKey, ruleObject, EInsertingGroupOrIntoMod.str());
				} else if (key == APPEND || key == REPLACE) {
					if (!IsPlugin(ruleObject))
						throw failure(ruleKey, ruleObject, EAttachingMessageToGroup.str());
				}
			}
		} catch (failure & e) {
			skip = true;
			AddSyntaxError(e.ruleKey, e.ruleObject, e.message);
			string const keystring = KeyToString(e.ruleKey);
			LOG_ERROR("Userlist Syntax Error: The rule beginning \"%s: %s\" %s", keystring.c_str(), e.ruleObject.c_str(), e.message.c_str());
		}
		if (!skip)
			userlist.push_back(currentRule);
		return;
	}

	//Stores the global message.
	void StoreCurrentLine(vector<line>& lines, line currentLine) {
		if (storeLine)
			lines.push_back(currentLine);
		return;
	}

	userlist_grammar::userlist_grammar() : userlist_grammar::base_type(ruleList, "userlist grammar") {

		//A list is a vector of rules. Rules are separated by line endings.
		ruleList = 
			*eol 
			> (eoi | (userlistRule[phoenix::bind(&RuleSyntaxCheck, _val, _1)] % eol)); 

		//A rule consists of a rule line containing a rule keyword and a rule object, followed by one or more message or sort lines.
		userlistRule %=
			*eol
			> ruleKey > ':' > object
			> +eol
			> sortOrMessageLines;

		sortOrMessageLines =
			sortOrMessageLine[phoenix::bind(&StoreCurrentLine, _val, _1)] % +eol;

		sortOrMessageLine %=
			sortOrMessageKey
			> ':'
			> omit[conditionals[phoenix::ref(storeLine) = _1]]
			> object;

		object %= lexeme[+(char_ - eol)]; //String, with no skipper.

		ruleKey %= no_case[ruleKeys];

		sortOrMessageKey %= no_case[sortOrMessageKeys];

		conditionals = 
			(conditional[_val = _1] 
			> *((andOr > conditional)			[phoenix::bind(&EvaluateCompoundConditional, _val, _1, _2)]))
			| eps[_val = true];

		andOr %= unicode::string("&&") | unicode::string("||");

		conditional = (no_case[metaKey] > '(' > condition > ')')	[phoenix::bind(&EvaluateConditional, _val, _1, _2)];

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

		//Give each rule names.
		ruleList.name("rules");
		userlistRule.name("rule");
		sortOrMessageLine.name("sort or message line");
		object.name("line object");
		ruleKey.name("rule keyword");
		sortOrMessageKey.name("sort or message keyword");
		
		on_error<fail>(ruleList,phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(userlistRule,phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(sortOrMessageLine,phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(object,phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(ruleKey,phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(sortOrMessageKey,phoenix::bind(&userlist_grammar::SyntaxError,this,_1,_2,_3,_4));
		on_error<fail>(conditionals,phoenix::bind(&userlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(andOr,phoenix::bind(&userlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(conditional,phoenix::bind(&userlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(condition,phoenix::bind(&userlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(variable,phoenix::bind(&userlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(file,phoenix::bind(&userlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		on_error<fail>(version,phoenix::bind(&userlist_grammar::SyntaxError, this, _1, _2, _3, _4));
	}

	void userlist_grammar::SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, info const& what) {
		ostringstream out;
		out << what;
		string expect = out.str().substr(1,out.str().length()-2);
		if (expect == "eol")
			expect = "end of line";

		string context(errorpos, min(errorpos +50, last));
		boost::trim_left(context);

		LOG_ERROR("Userlist Parsing Error: Expected a %s at \"%s\". Userlist parsing aborted. No rules will be applied.", expect.c_str(), context.c_str());
			
		expect = "&lt;" + expect + "&gt;";
		boost::replace_all(context, "\n", "<br />\n");
		string msg = (UserlistParsingErrorFormat % expect % context).str();
		userlistErrorBuffer.push_back(msg);
		return;
	}
}