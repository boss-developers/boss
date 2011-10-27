/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011  Random/Random007/jpearce, WrinklyNinja & the BOSS 
	development team. Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __BOSS_GRAMMAR_H__
#define __BOSS_GRAMMAR_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include "Common/Classes.h"
#include <string>
#include <vector>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

//////////////////////////////
// Modlist data conversions
//////////////////////////////

BOOST_FUSION_ADAPT_STRUCT(
    boss::Message,
    (boss::keyType, key)
    (std::string, data)
)

BOOST_FUSION_ADAPT_STRUCT(
    boss::Item,
	(boss::itemType, type)
    (fs::path, name)
    (std::vector<boss::Message>, messages)
)

//////////////////////////////
//RuleList Data Conversions
//////////////////////////////

BOOST_FUSION_ADAPT_STRUCT(
	boss::RuleLine,
	(boss::keyType, key)
	(std::string, object)
)

BOOST_FUSION_ADAPT_STRUCT(
	boss::Rule,
	(bool, enabled)
	(boss::keyType, ruleKey)
	(std::string, ruleObject)
	(std::vector<boss::RuleLine>, lines)
)


namespace boss {
	namespace qi = boost::spirit::qi;

	using namespace std;

	using qi::grammar;
	using boost::spirit::info;

	///////////////////////////////
	// Keyword structures
	///////////////////////////////

	enum metaType {
		IF,
		IFNOT
	};

	struct ruleKeys_ : qi::symbols<char, keyType> {
		inline ruleKeys_();
	};

	struct messageKeys_ : qi::symbols<char, keyType> {
		messageKeys_();
	};

	struct masterlistMsgKey_ : qi::symbols<char, keyType> {
		inline masterlistMsgKey_();
	};

	struct typeKey_ : qi::symbols<char, itemType> {
		inline typeKey_();
	};

	struct metaKey_ : qi::symbols<char, metaType> {
		inline metaKey_();
	};

	///////////////////////////////
	//Skipper Grammar
	///////////////////////////////
	
	//Skipper for userlist, modlist and ini parsers.
	class Skipper : public grammar<string::const_iterator> {
	public:
		Skipper(bool skipIniComments);
	private:
		qi::rule<string::const_iterator> start, spc, eof, CComment, CPlusPlusComment, lineComment, iniComment, UTF8;
	};

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	//Modlist/Masterlist grammar.
	class modlist_grammar : public grammar<string::const_iterator, vector<Item>(), Skipper> {
	public:
		modlist_grammar();
		inline void SetErrorBuffer(ParsingError * inErrorBuffer) { errorBuffer = inErrorBuffer; }
		inline void SetGlobalMessageBuffer(vector<Message> * inGlobalMessageBuffer) { globalMessageBuffer = inGlobalMessageBuffer; }
	private:
		qi::rule<string::const_iterator, vector<Item>(), Skipper> modList;
		qi::rule<string::const_iterator, Item(), Skipper> listItem;
		qi::rule<string::const_iterator, itemType(), Skipper> ItemType;
		qi::rule<string::const_iterator, fs::path(), Skipper> itemName;
		qi::rule<string::const_iterator, vector<Message>(), Skipper> itemMessages;
		qi::rule<string::const_iterator, Message(), Skipper> itemMessage, globalMessage;
		qi::rule<string::const_iterator, string(), Skipper> charString, messageString, variable, file, version, andOr, metaLine, messageVersionCRC, messageModString, messageModVariable, regexFile;
		qi::rule<string::const_iterator, keyType(), Skipper> messageKeyword;
		qi::rule<string::const_iterator, bool(), Skipper> conditional, conditionals, condition, oldConditional;
		
		void SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, boost::spirit::info const& what);
		
		ParsingError * errorBuffer;
		vector<Message> * globalMessageBuffer;
		const vector<Message> noMessages;  //An empty set of messages.
		bool storeItem;
		bool storeMessage;  //Should the current item/message be stored.
		keyType currentMessageType;
		vector<string> openGroups;  //Need to keep track of which groups are open to match up endings properly in MF1.
		boost::unordered_set<string> setVars;  //Vars set by masterlist. Also referenced by userlist parser.
		boost::unordered_map<string,unsigned int> fileCRCs;  //CRCs calculated. Referenced by modlist and userlist parsers.

		//Stores a message, should it be appropriate.
		void StoreMessage(vector<Message>& messages, Message currentMessage);

		//Stores the given item, should it be appropriate, and records any changes to open groups.
		void StoreItem(vector<Item>& list, Item currentItem);

		//Defines the given masterlist variable, if appropriate.
		void StoreVar(const string var);

		//Stores the global message.
		void StoreGlobalMessage(const Message currentMessage);

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
		//This might not work when the regex specifies a file and a path, eg. "path/to/file.txt", because characters like '.' need to be escaped in regex
		//so the regex would be "path/to/file\.txt". boost::filesystem might interpret that as a path of "path / to / file / .txt" though.
		//In windows, the above path would be "path\to\file.txt", which would become "path\\to\\file\.txt" in regex. Basically, the extra backslashes need to
		//be removed when getting the path and filename.
		void CheckRegex(bool& result, string reg);

		//Evaluate a single conditional.
		void EvaluateConditional(bool& result, const metaType type, const bool condition);

		//Evaluate the second half of a complex conditional.
		void EvaluateCompoundConditional(bool& result, const string andOr, const bool condition);

		//Converts a hex string to an integer using BOOST's Spirit.Qi. Faster than a stringstream conversion.
		unsigned int HexStringToInt(string str);

		//Evaluate part of a shorthand conditional message.
		//Most message types would make sense for the message to display if the condition evaluates to true. (eg. incompatibilities)
		//Requirement messages need the condition to eval to false.
		void EvaluateConditionalMessage(string& message, string version, string file, const string mod);

		//Turns a given string into a path. Can't be done directly because of the openGroups checks.
		void path(fs::path& p, string itemName);
	};

	////////////////////////////
	//Ini Grammar.
	////////////////////////////

	//Ini grammar.
	class ini_grammar : public grammar<string::const_iterator, Skipper> {
	public:
		ini_grammar();
		inline void SetErrorBuffer(ParsingError * inErrorBuffer) { errorBuffer = inErrorBuffer; }
	private:
		qi::rule<string::const_iterator, Skipper> ini, section, setting;
		qi::rule<string::const_iterator, string(), Skipper> var, stringVal, heading;
	
		void SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, info const& what);

		//Set the boolean BOSS variable values while parsing.
		void SetBoolVar(string& var, const bool& value);

		//Set the integer BOSS variable values while parsing.
		void SetIntVar(string& var, const unsigned int& value);

		//Set the BOSS variable values while parsing.
		void SetStringVar(string& var, string& value);

		ParsingError * errorBuffer;
	};

	////////////////////////////
	//RuleList Grammar.
	////////////////////////////

	//RuleList grammar.
	class userlist_grammar : public qi::grammar<string::const_iterator, vector<Rule>(), Skipper> {
	public:
		userlist_grammar();
		inline void SetParsingErrorBuffer(ParsingError * inErrorBuffer) { parsingErrorBuffer = inErrorBuffer; }
		inline void SetSyntaxErrorBuffer(vector<ParsingError> * inErrorBuffer) { syntaxErrorBuffer = inErrorBuffer; }
	private:
		qi::rule<string::const_iterator, vector<Rule>(), Skipper> ruleList;
		qi::rule<string::const_iterator, Rule(), Skipper> userlistRule;
		qi::rule<string::const_iterator, RuleLine(), Skipper> sortOrMessageLine;
		qi::rule<string::const_iterator, keyType(), Skipper> ruleKey, sortOrMessageKey;
		qi::rule<string::const_iterator, string(), Skipper> object;
		qi::rule<string::const_iterator, bool(), Skipper> stateKey;
	
		void SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, info const& what);

		void RuleSyntaxCheck(vector<Rule>& userlist, Rule currentRule);

		ParsingError * parsingErrorBuffer;
		vector<ParsingError> * syntaxErrorBuffer;
	};

}
#endif
