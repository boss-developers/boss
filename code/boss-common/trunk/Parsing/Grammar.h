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

#include <boost/spirit/include/qi.hpp>


namespace boss {
	namespace qi = boost::spirit::qi;

	using namespace std;

	using qi::grammar;
	using boost::spirit::info;

	///////////////////////////////
	//Skipper Grammars
	///////////////////////////////
	
	//Skipper for userlist and modlist parsers.
	struct Skipper : grammar<string::const_iterator> {

		Skipper();

		qi::rule<string::const_iterator> start, spc, eof, CComment, CPlusPlusComment, lineComment, UTF8;
	};

	//Skipper for ini parser.
	struct Ini_Skipper : grammar<string::const_iterator> {

		Ini_Skipper();

		qi::rule<string::const_iterator> start, spc, eof, comment, UTF8;
	};

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	extern bool storeItem;
	extern bool storeMessage;  //Should the current item/message be stored.
	extern vector<string> openGroups;  //Need to keep track of which groups are open to match up endings properly in MF1.

	//Stores a message, should it be appropriate.
	void StoreMessage(vector<message>& messages, const message currentMessage);

	//Stores the given item, should it be appropriate, and records any changes to open groups.
	void StoreItem(vector<item>& list, const item currentItem);

	//Defines the given masterlist variable, if appropriate.
	void StoreVar(const string var);

	//Stores the global message.
	void StoreGlobalMessage(const message currentMessage);

	//MF1 compatibility function. Evaluates the MF1 FCOM conditional. Like it says on the tin.
	void EvalOldFCOMConditional(bool& result, const char var);

	//MF1 compatibility function. Evaluates the MF1 OOO/BC conditional message symbols.
	void EvalMessKey(const keyType key);

	//Checks if a masterlist variable is defined.
	void CheckVar(bool& result, const string var);

	//Returns the true path based on what type of file or keyword it is.
	void GetPath(fs::path& file_path, string& file);

	//Checks if the given mod has a version for which the comparison holds true.
	void CheckVersion(bool& result, const string var);

	//Checks if the given mod has the given checksum.
	void CheckSum(bool& result, const unsigned int sum, string file);

	//Checks if the given file exists.
	void CheckFile(bool& result, string file);

	//Checks if a file which matches the given regex exists.
	void CheckRegex(bool& result, string reg);

	//Evaluate a single conditional.
	void EvaluateConditional(bool& result, const metaType type, const bool condition);

	//Evaluate the second half of a complex conditional.
	void EvaluateCompoundConditional(bool& result, const string andOr, const bool condition);

	//Evaluate part of a shorthand conditional message.
	void EvaluateConditionalMessage(string& message, string version, string file, const string mod);

	//Converts a hex string to an integer using BOOST's Spirit.Qi. Faster than a stringstream conversion.
	unsigned int HexStringToInt(string hex);
	
	//Turns a given string into a path. Can't be done directly because of the openGroups checks.
	void path(fs::path& p, string const itemName);

	//Modlist/Masterlist grammar.
	struct modlist_grammar : grammar<string::const_iterator, vector<item>(), Skipper> {

		modlist_grammar();

		qi::rule<string::const_iterator, vector<item>(), Skipper> modList;
		qi::rule<string::const_iterator, item(), Skipper> listItem;
		qi::rule<string::const_iterator, itemType(), Skipper> ItemType;
		qi::rule<string::const_iterator, fs::path(), Skipper> itemName;
		qi::rule<string::const_iterator, vector<message>(), Skipper> itemMessages;
		qi::rule<string::const_iterator, message(), Skipper> itemMessage, globalMessage;
		qi::rule<string::const_iterator, string(), Skipper> charString, messageString, variable, file, version, andOr, metaLine, messageVersionCRC, messageModString, messageModVariable, regexFile;
		qi::rule<string::const_iterator, keyType(), Skipper> messageKeyword;
		qi::rule<string::const_iterator, bool(), Skipper> conditional, conditionals, condition, oldConditional;
		
		void SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, boost::spirit::info const& what);
	};

	////////////////////////////
	//Ini Grammar.
	////////////////////////////

	extern string currentHeading;  //The current ini section heading.

	//Set the boolean BOSS variable values while parsing.
	void SetBoolVar(string var, const bool value);

	//Set the integer BOSS variable values while parsing.
	void SetIntVar(string var, const unsigned int value);

	//Set the string BOSS variable values while parsing.
	void SetStringVar(string var, string value);

	//Ini grammar.
	struct ini_grammar : grammar<string::const_iterator, Ini_Skipper> {

		ini_grammar();

		qi::rule<string::const_iterator, Ini_Skipper> ini, section, setting;
		qi::rule<string::const_iterator, string(), Ini_Skipper> var, stringVal, heading;
	
		void SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, info const& what);
	};

	////////////////////////////
	//Userlist Grammar.
	////////////////////////////

	extern bool storeLine;  //Should the current message/sort line be stored.

	// Error messages for rule validation
	static const string ESortLineInForRule("includes a sort line in a rule with a FOR rule keyword.");
	static const string EAddingModGroup("tries to add a group.");
	static const string ESortingGroupEsms("tries to sort the group \"ESMs\".");
	static const string ESortingMasterEsm("tries to sort the master .ESM file.");
	static const string EReferencingModAndGroup("references a mod and a group.");
	static const string ESortingGroupBeforeEsms("tries to sort a group before the group \"ESMs\".");
	static const string ESortingModBeforeGameMaster("tries to sort a mod before the master .ESM file.");
	static const string EInsertingToTopOfEsms("tries to insert a mod into the top of the group \"ESMs\", before the master .ESM file.");
	static const string EInsertingGroupOrIntoMod("tries to insert a group or insert something into a mod.");
	static const string EAttachingMessageToGroup("tries to attach a message to a group.");
	static const string EMultipleSortLines("has more than one sort line.");
	static const string EMultipleReplaceLines("has more than one REPLACE-using message line.");
	static const string EReplaceNotFirst("has a REPLACE-using message line that is not the first message line.");
	static const string ESortNotSecond("has a sort line that is not the second line of the rule.");
	static const string ESortingToItself("tries to sort a mod or group relative to itself.");

	// Used to throw as exception when signaling a userlist syntax error, in order to make the code a bit more compact.
	struct failure {
		failure(keyType const& ruleKey, string const& ruleObject, string const& message);

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
		userlist_grammar();

		qi::rule<string::const_iterator, vector<rule>(), Skipper> ruleList;
		qi::rule<string::const_iterator, rule(), Skipper> userlistRule;
		qi::rule<string::const_iterator, line(), Skipper> sortOrMessageLine;
		qi::rule<string::const_iterator, keyType(), Skipper> ruleKey, sortOrMessageKey;
		qi::rule<string::const_iterator, string(), Skipper> object;
		qi::rule<string::const_iterator, bool(), Skipper> stateKey;
	
		void SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, info const& what);
	};

}
#endif
