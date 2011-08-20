/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __BOSS_GRAMMAR_H__
#define __BOSS_GRAMMAR_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include "Parsing/Data.h"
#include "Common/Globals.h"
#include "Support/Helpers.h"
#include "Support/Logger.h"
#include "Common/BOSSLog.h"

#include <sstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
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

	using boost::format;
	using boost::spirit::info;

	///////////////////////////////
	//Common Functions
	///////////////////////////////

	//Checks if a masterlist variable is defined.
	void CheckVar(bool& result, string var);

	//Returns the true path based on what type of file or keyword it is.
	void GetPath(fs::path& file_path, string& file);

	//Checks if the given mod has a version for which the comparison holds true.
	void CheckVersion(bool& result, string var);

	//Checks if the given mod has the given checksum.
	void CheckSum(bool& result, unsigned int sum, string file);

	void CheckFile(bool& result, string file);

	//Evaluate a single conditional.
	void EvaluateConditional(bool& result, metaType type, bool condition);

	//Evaluate the second half of a complex conditional.
	void EvaluateCompoundConditional(bool& result, string andOr, bool condition);


	///////////////////////////////
	//Skipper Grammars
	///////////////////////////////


	
	//Skipper for userlist and modlist parsers.
	struct Skipper : qi::grammar<string::const_iterator> {

		Skipper() : Skipper::base_type(start, "Skipper") {

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

		qi::rule<string::const_iterator> start, spc, eof, CComment, CPlusPlusComment, lineComment, UTF8;
	};

	//Skipper for ini parser.
	struct Ini_Skipper : qi::grammar<string::const_iterator> {

		Ini_Skipper() : Ini_Skipper::base_type(start, "Ini Skipper") {

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

		qi::rule<string::const_iterator> start, spc, eof, comment, UTF8;
	};

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	extern bool storeItem;
	extern bool storeMessage;  //Should the current item/message be stored.
	extern vector<string> openGroups;  //Need to keep track of which groups are open to match up endings properly in MF1.

	//Parsing error message format.
	static format MasterlistParsingErrorFormat("<p><span class='error'>Masterlist Parsing Error: Expected a %1% at:</span>"
		"<blockquote>%2%</blockquote>"
		"<span class='error'>Masterlist parsing aborted. Utility will end now.</span></p>\n\n");

	//Stores a message, should it be appropriate.
	//The SPECIFIC_REQ and SPECIFIC_INC 'parsers' are not space-safe within items.
	void StoreMessage(vector<message>& messages, message currentMessage);

	//Stores the given item, should it be appropriate, and records any changes to open groups.
	void StoreItem(vector<item>& list, item currentItem);

	//Defines the given masterlist variable, if appropriate.
	void StoreVar(string var);

	//Stores the global message.
	void StoreGlobalMessage(message currentMessage);

	//MF1 compatibility function. Evaluates the MF1 FCOM conditional. Like it says on the tin.
	void EvalOldFCOMConditional(bool& result, char var);

	//MF1 compatibility function. Evaluates the MF1 OOO/BC conditional message symbols.
	void EvalMessKey(keyType key);
	
	//Turns a given string into a path. Can't be done directly because of the openGroups checks.
	void path(fs::path& p, string const itemName);

	//Modlist/Masterlist grammar.
	struct modlist_grammar : qi::grammar<string::const_iterator, vector<item>(), Skipper> {
		modlist_grammar() : modlist_grammar::base_type(modList, "modlist_grammar") {

			vector<message> noMessages;  //An empty set of messages.

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

			messageString %= lexeme[+(char_ - eol)]; //String, with no skipper. Used for messages, which can contain web links which the skipper would cut out.

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
			
			on_error<fail>(modList,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(metaLine,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(listItem,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(ItemType,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemName,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemMessages,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(itemMessage,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(charString,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
			on_error<fail>(messageString,phoenix::bind(&modlist_grammar::SyntaxErr, this, _1, _2, _3, _4));
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

		qi::rule<string::const_iterator, vector<item>(), Skipper> modList;
		qi::rule<string::const_iterator, item(), Skipper> listItem;
		qi::rule<string::const_iterator, itemType(), Skipper> ItemType;
		qi::rule<string::const_iterator, fs::path(), Skipper> itemName;
		qi::rule<string::const_iterator, vector<message>(), Skipper> itemMessages;
		qi::rule<string::const_iterator, message(), Skipper> itemMessage, globalMessage;
		qi::rule<string::const_iterator, string(), Skipper> charString, messageString, variable, file, version, andOr, keyword, metaLine;
		qi::rule<string::const_iterator, keyType(), Skipper> messageKeyword;
		qi::rule<string::const_iterator, bool(), Skipper> conditional, conditionals, condition, oldConditional;
		
		void SyntaxErr(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, boost::spirit::info const& what) {
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
	};

	////////////////////////////
	//Ini Grammar.
	////////////////////////////

	extern string currentHeading;  //The current ini section heading.

	//Parsing error formatting.
	static format IniParsingErrorFormat("<li><span class='error'>Ini Parsing Error: Expected a %1% at:</span>"
		"<blockquote>%2%</blockquote>"
		"<span class='error'>Ini parsing aborted. Some or all of the options may not have been set correctly.</span></li>\n");

	//Set the BOSS variable values while parsing.
	void SetVar(string var, string value);

	//Ini grammar.
	struct ini_grammar : qi::grammar<string::const_iterator, Ini_Skipper> {

		ini_grammar() : ini_grammar::base_type(ini, "ini grammar") {

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

		qi::rule<string::const_iterator, Ini_Skipper> ini, section, setting;
		qi::rule<string::const_iterator, string(), Ini_Skipper> var, value, heading;
	
		void SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, info const& what) {
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
	};

	////////////////////////////
	//Userlist Grammar.
	////////////////////////////

	extern bool storeLine;  //Should the current message/sort line be stored.

	// Error messages for rule validation
	static format ESortLineInForRule("includes a sort line in a rule with a FOR rule keyword.");
	static format EAddingModGroup("tries to add a group.");
	static format ESortingGroupEsms("tries to sort the group \"ESMs\".");
	static format ESortingMasterEsm("tries to sort the master .ESM file.");
	static format EReferencingModAndGroup("references a mod and a group.");
	static format ESortingGroupBeforeEsms("tries to sort a group before the group \"ESMs\".");
	static format ESortingModBeforeGameMaster("tries to sort a mod before the master .ESM file.");
	static format EInsertingToTopOfEsms("tries to insert a mod into the top of the group \"ESMs\", before the master .ESM file.");
	static format EInsertingGroupOrIntoMod("tries to insert a group or insert something into a mod.");
	static format EAttachingMessageToGroup("tries to attach a message to a group.");

	//Syntax error formatting.
	static format SyntaxErrorFormat("<li class='error'>"
		"Userlist Syntax Error: The rule beginning \"%1%: %2%\" %3%"
		"</li>\n");

	//Parsing error formatting.
	static format UserlistParsingErrorFormat("<li><span class='error'>Userlist Parsing Error: Expected a %1% at:</span>"
		"<blockquote>%2%</blockquote>"
		"<span class='error'>Userlist parsing aborted. No rules will be applied.</span></li>\n");

	// Used to throw as exception when signaling a userlist syntax error, in order to make the code a bit more compact.
	struct failure {
		failure(keyType const& ruleKey, string const& ruleObject, string const& message) 
			: ruleKey(ruleKey), ruleObject(ruleObject), message(message) {}

		keyType ruleKey;
		string ruleObject;
		string message;
	};

	//Add syntax error to buffer.
	void AddSyntaxError(keyType const& rule, string const& object, string const& message);

	//Rule checker function, checks for syntax (not parsing) errors.
	void RuleSyntaxCheck(vector<rule>& userlist, rule currentRule);

	//Stores the global message.
	void StoreCurrentLine(vector<line>& lines, line currentLine);

	//Userlist grammar.
	struct userlist_grammar : qi::grammar<string::const_iterator, vector<rule>(), Skipper> {
		userlist_grammar() : userlist_grammar::base_type(ruleList, "userlist grammar") {

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

		qi::rule<string::const_iterator, vector<rule>(), Skipper> ruleList;
		qi::rule<string::const_iterator, rule(), Skipper> userlistRule;
		qi::rule<string::const_iterator, vector<line>(), Skipper> sortOrMessageLines;
		qi::rule<string::const_iterator, line(), Skipper> sortOrMessageLine;
		qi::rule<string::const_iterator, keyType(), Skipper> ruleKey, sortOrMessageKey;
		qi::rule<string::const_iterator, string(), Skipper> object, variable, file, version, andOr, keyword, metaLine;
		qi::rule<string::const_iterator, bool(), Skipper> conditional, conditionals, condition;
	
		void SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, info const& what) {
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
	};

}
#endif
