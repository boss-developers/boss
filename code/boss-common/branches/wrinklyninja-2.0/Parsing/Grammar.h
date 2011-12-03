/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011    BOSS Development Team.

	This file is part of Better Oblivion Sorting Software.

    Better Oblivion Sorting Software is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    Better Oblivion Sorting Software is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Better Oblivion Sorting Software.  If not, see 
	<http://www.gnu.org/licenses/>.

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

namespace fs = boost::filesystem;

//////////////////////////////
// Modlist data conversions
//////////////////////////////

BOOST_FUSION_ADAPT_STRUCT(
    boss::MasterlistVar,
	(std::string, conditionals)
    (std::string, var)
)

BOOST_FUSION_ADAPT_STRUCT(
    boss::Message,
	(std::string, conditionals)
    (boss::keyType, key)
    (std::string, data)
)

BOOST_FUSION_ADAPT_STRUCT(
    boss::Item,
	(std::string, conditionals)
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

	struct ruleKeys_ : qi::symbols<char, keyType> {
		ruleKeys_();
	};

	struct messageKeys_ : qi::symbols<char, keyType> {
		messageKeys_();
	};

	struct masterlistMsgKey_ : qi::symbols<char, keyType> {
		masterlistMsgKey_();
	};

	struct typeKey_ : qi::symbols<char, itemType> {
		typeKey_();
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
		inline void SetVarStore(vector<MasterlistVar> * varStore) { setVars = varStore; }
		inline void SetCRCStore(boost::unordered_map<string,uint32_t> * CRCStore) {fileCRCs = CRCStore; }
	private:
		qi::rule<string::const_iterator, vector<Item>(), Skipper> modList;
		qi::rule<string::const_iterator, Item(), Skipper> listItem;
		qi::rule<string::const_iterator, itemType(), Skipper> ItemType;
		qi::rule<string::const_iterator, fs::path(), Skipper> itemName;
		qi::rule<string::const_iterator, vector<Message>(), Skipper> itemMessages;
		qi::rule<string::const_iterator, Message(), Skipper> itemMessage, globalMessage, oldCondItemMessage;
		qi::rule<string::const_iterator, MasterlistVar(), Skipper> listVar;
		qi::rule<string::const_iterator, string(), Skipper> charString, andOr, conditional, conditionals, oldConditional, condition, version, variable, file, regexFile;
		qi::rule<string::const_iterator, keyType(), Skipper> messageKeyword;

		void SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, boost::spirit::info const& what);
		
		ParsingError * errorBuffer;
		vector<Message> * globalMessageBuffer;
		vector<MasterlistVar> * setVars;  //Vars set by masterlist.
		boost::unordered_map<string,uint32_t> * fileCRCs;  //CRCs calculated.

		vector<string> openGroups;  //Need to keep track of which groups are open to match up endings properly in MF1.
		

		//Stores the given item, should it be appropriate, and records any changes to open groups.
		void StoreItem(vector<Item>& list, Item currentItem);

		//MF1 compatibility function. Evaluates the MF1 FCOM conditional. Like it says on the tin.
		void ConvertOldConditional(string& result, const char var);

		//Turns a given string into a path. Can't be done directly because of the openGroups checks.
		void ToPath(fs::path& p, string itemName);

		void StoreVar(const MasterlistVar var);

		void StoreGlobalMessage(const Message message);
	};


	////////////////////////////
	// Conditional Grammar
	////////////////////////////

	class conditional_grammar : public grammar<string::const_iterator, bool(), Skipper> {
	public:
		conditional_grammar();
		inline void SetVarStore(boost::unordered_set<string> * varStore) { setVars = varStore; }
		inline void SetCRCStore(boost::unordered_map<string,uint32_t> * CRCStore) {fileCRCs = CRCStore; }
	private:
		qi::rule<string::const_iterator, string(), Skipper> ifIfNot, variable, file, version, andOr, regexFile;
		qi::rule<string::const_iterator, bool(), Skipper> conditional, conditionals, condition;
		
		void SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, boost::spirit::info const& what);
		
		boost::unordered_set<string> * setVars;  //Vars set by masterlist. Also referenced by userlist parser.
		boost::unordered_map<string,uint32_t> * fileCRCs;  //CRCs calculated. Referenced by modlist and userlist parsers.

		//Checks if a masterlist variable is defined.
		void CheckVar(bool& result, const string var);

		//Returns the true path based on what type of file or keyword it is.
		void GetPath(fs::path& file_path, string& file);

		//Checks if the given mod has a version for which the comparison holds true.
		void CheckVersion(bool& result, const string var);

		//Checks if the given mod has the given checksum.
		void CheckSum(bool& result, const uint32_t sum, string file);

		//Checks if the given file exists.
		void CheckFile(bool& result, string file);

		//Checks if a file which matches the given regex exists.
		//This might not work when the regex specifies a file and a path, eg. "path/to/file.txt", because characters like '.' need to be escaped in regex
		//so the regex would be "path/to/file\.txt". boost::filesystem might interpret that as a path of "path / to / file / .txt" though.
		//In windows, the above path would be "path\to\file.txt", which would become "path\\to\\file\.txt" in regex. Basically, the extra backslashes need to
		//be removed when getting the path and filename.
		void CheckRegex(bool& result, string reg);

		//Evaluate a single conditional.
		void EvaluateConditional(bool& result, const string type, const bool condition);

		//Evaluate the second half of a complex conditional.
		void EvaluateCompoundConditional(bool& result, const string andOr, const bool condition);
	};


	///////////////////////////////////
	// Conditional Shorthand Grammar
	///////////////////////////////////

	class shorthand_grammar : public grammar<string::const_iterator, string(), Skipper> {
	public:
		shorthand_grammar();
		inline void SetMessageType(keyType type) { messageType = type; }
		inline void SetVarStore(boost::unordered_set<string> * varStore) { setVars = varStore; }
		inline void SetCRCStore(boost::unordered_map<string,uint32_t> * CRCStore) {fileCRCs = CRCStore; }
	private:
		qi::rule<string::const_iterator, string(), Skipper> charString, messageString, messageVersionCRC, messageModString, messageModVariable, file;

		void SyntaxError(string::const_iterator const& /*first*/, string::const_iterator const& last, string::const_iterator const& errorpos, boost::spirit::info const& what);
		
		boost::unordered_set<string> * setVars;  //Vars set by masterlist. Also referenced by userlist parser.
		boost::unordered_map<string,uint32_t> * fileCRCs;  //CRCs calculated. Referenced by modlist and userlist parsers.
		keyType messageType;

		//Checks if a masterlist variable is defined.
		void CheckVar(bool& result, const string var);

		//Returns the true path based on what type of file or keyword it is.
		void GetPath(fs::path& file_path, string& file);

		//Checks if the given mod has a version for which the comparison holds true.
		void CheckVersion(bool& result, const string var);

		//Checks if the given mod has the given checksum.
		void CheckSum(bool& result, const uint32_t sum, string file);

		//Checks if the given file exists.
		void CheckFile(bool& result, string file);

		//Checks if a file which matches the given regex exists.
		//This might not work when the regex specifies a file and a path, eg. "path/to/file.txt", because characters like '.' need to be escaped in regex
		//so the regex would be "path/to/file\.txt". boost::filesystem might interpret that as a path of "path / to / file / .txt" though.
		//In windows, the above path would be "path\to\file.txt", which would become "path\\to\\file\.txt" in regex. Basically, the extra backslashes need to
		//be removed when getting the path and filename.
		void CheckRegex(bool& result, string reg);

		//Converts a hex string to an integer using BOOST's Spirit.Qi. Faster than a stringstream conversion.
		uint32_t HexStringToInt(string str);

		//Evaluate part of a shorthand conditional message.
		//Most message types would make sense for the message to display if the condition evaluates to true. (eg. incompatibilities)
		//Requirement messages need the condition to eval to false.
		void EvaluateConditionalMessage(string& message, string version, string file, const string mod);
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
		void SetIntVar(string& var, const uint32_t& value);

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
