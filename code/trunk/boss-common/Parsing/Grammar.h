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
	(std::string, conditions)
    (std::string, data)
)

BOOST_FUSION_ADAPT_STRUCT(
    boss::Message,
	(std::string, conditions)
    (uint32_t, key)
    (std::string, data)
)

BOOST_FUSION_ADAPT_STRUCT(
    boss::Item,
	(std::string, conditions)
	(boss::itemType, type)
    (std::string, data)
    (std::vector<boss::Message>, messages)
)

//////////////////////////////
//RuleList Data Conversions
//////////////////////////////

BOOST_FUSION_ADAPT_STRUCT(
	boss::RuleLine,
	(uint32_t, key)
	(std::string, object)
)

BOOST_FUSION_ADAPT_STRUCT(
	boss::Rule,
	(bool, enabled)
	(uint32_t, key)
	(std::string, object)
	(std::vector<boss::RuleLine>, lines)
)

//////////////////////////////
//Ini Data Conversions
//////////////////////////////

BOOST_FUSION_ADAPT_STRUCT(
	boss::IniPair,
	(std::string, key)
	(std::string, value)
)


namespace boss {
	namespace qi = boost::spirit::qi;

	using namespace std;

	using qi::grammar;
	using boost::spirit::info;

	//typedef boost::u8_to_u32_iterator<std::string::const_iterator> iterator_type;
	//typedef iterator_type grammarIter;
	typedef string::const_iterator grammarIter;

	///////////////////////////////
	// Keyword structures
	///////////////////////////////

	struct ruleKeys_ : qi::symbols<char, uint32_t> {
		ruleKeys_();
	};

	struct messageKeys_ : qi::symbols<char, uint32_t> {
		messageKeys_();
	};

	struct masterlistMsgKey_ : qi::symbols<char, uint32_t> {
		masterlistMsgKey_();
	};

	struct oldMasterlistMsgKey_ : qi::symbols<char, uint32_t> {
		oldMasterlistMsgKey_();
	};

	struct typeKey_ : qi::symbols<char, itemType> {
		typeKey_();
	};


	///////////////////////////////
	//Skipper Grammar
	///////////////////////////////
	
	//Skipper for userlist, modlist and ini parsers.
	class Skipper : public grammar<grammarIter> {
	public:
		Skipper();
		void SkipIniComments(bool b);
	private:
		qi::rule<grammarIter> start, spc, eof, CComment, CPlusPlusComment, lineComment, iniComment, UTF8;
	};

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	//Modlist/Masterlist grammar.
	class modlist_grammar : public grammar<grammarIter, vector<Item>(), Skipper> {
	public:
		modlist_grammar();
		inline void SetErrorBuffer(ParsingError * inErrorBuffer) { errorBuffer = inErrorBuffer; }
		inline void SetGlobalMessageBuffer(vector<Message> * inGlobalMessageBuffer) { globalMessageBuffer = inGlobalMessageBuffer; }
		inline void SetVarStore(vector<MasterlistVar> * varStore) { setVars = varStore; }
		inline void SetCRCStore(boost::unordered_map<string,uint32_t> * CRCStore) {fileCRCs = CRCStore; }
	private:
		qi::rule<grammarIter, vector<Item>(), Skipper> modList;
		qi::rule<grammarIter, Item(), Skipper> listItem;
		qi::rule<grammarIter, itemType(), Skipper> ItemType;
		qi::rule<grammarIter, string(), Skipper> itemName;
		qi::rule<grammarIter, vector<Message>(), Skipper> itemMessages;
		qi::rule<grammarIter, Message(), Skipper> itemMessage, globalMessage, oldCondItemMessage;
		qi::rule<grammarIter, MasterlistVar(), Skipper> listVar;
		qi::rule<grammarIter, string(), Skipper> charString, andOr, conditional, conditionals, oldConditional, condition, version, variable, file, regexFile;
		qi::rule<grammarIter, uint32_t(), Skipper> messageKeyword, messageSymbol;
		ParsingError * errorBuffer;
		vector<Message> * globalMessageBuffer;
		vector<MasterlistVar> * setVars;					//Vars set by masterlist.
		boost::unordered_map<string,uint32_t> * fileCRCs;	//CRCs calculated.
		vector<string> openGroups;  //Need to keep track of which groups are open to match up endings properly in MF1.

		//Parser error reporter.
		void SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, boost::spirit::info const& what);

		//Stores the given item and records any changes to open groups.
		void StoreItem(vector<Item>& list, Item currentItem);

		//Stores the masterlist variable.
		void StoreVar(const MasterlistVar var);

		//Stores the global message.
		void StoreGlobalMessage(const Message message);

		//MF1 compatibility function. Evaluates the MF1 FCOM conditional. Like it says on the tin.
		void ConvertOldConditional(string& result, const char var);

		//Turns a given string into a path. Can't be done directly because of the openGroups checks.
		void ToName(string& p, string itemName);

		
	};


	////////////////////////////
	// Conditional Grammar
	////////////////////////////

	class conditional_grammar : public grammar<grammarIter, bool(), Skipper> {
	public:
		conditional_grammar();
		inline void SetErrorBuffer(ParsingError * inErrorBuffer) { errorBuffer = inErrorBuffer; }
		inline void SetVarStore(boost::unordered_set<string> * varStore) { setVars = varStore; }
		inline void SetCRCStore(boost::unordered_map<string,uint32_t> * CRCStore) {fileCRCs = CRCStore; }
	private:
		qi::rule<grammarIter, string(), Skipper> ifIfNot, variable, file, version, andOr, regexFile;
		qi::rule<grammarIter, bool(), Skipper> conditional, conditionals, condition;
		ParsingError * errorBuffer;
		boost::unordered_set<string> * setVars;				//Vars set by masterlist.
		boost::unordered_map<string,uint32_t> * fileCRCs;	//CRCs calculated.
		
		//Parser error reporter.
		void SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, boost::spirit::info const& what);

		//Checks if a masterlist variable is defined.
		void CheckVar(bool& result, const string var);

		//Checks if the given mod has the given checksum.
		void CheckSum(bool& result, const uint32_t sum, string file);

		//Evaluate a single conditional.
		void EvaluateConditional(bool& result, const string type, const bool condition);

		//Evaluate the second half of a complex conditional.
		void EvaluateCompoundConditional(bool& lhsCondition, const string andOr, const bool rhsCondition);
	};


	///////////////////////////////////
	// Conditional Shorthand Grammar
	///////////////////////////////////

	class shorthand_grammar : public grammar<grammarIter, string(), Skipper> {
	public:
		shorthand_grammar();
		inline void SetErrorBuffer(ParsingError * inErrorBuffer) { errorBuffer = inErrorBuffer; }
		inline void SetMessageType(uint32_t type) { messageType = type; }
		inline void SetVarStore(boost::unordered_set<string> * varStore) { setVars = varStore; }
		inline void SetCRCStore(boost::unordered_map<string,uint32_t> * CRCStore) {fileCRCs = CRCStore; }
	private:
		qi::rule<grammarIter, string(), Skipper> charString, messageItem, messageString, messageVersionCRC, messageModString, messageModVariable, file;
		qi::rule<grammarIter, Skipper> messageItemDelimiter;
		ParsingError * errorBuffer;
		boost::unordered_set<string> * setVars;				//Vars set by masterlist.
		boost::unordered_map<string,uint32_t> * fileCRCs;	//CRCs calculated.
		uint32_t messageType;

		//Parser error reporter.
		void SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, boost::spirit::info const& what);
		
		//Checks if a masterlist variable is defined.
		void CheckVar(bool& result, const string var);

		//Checks if the given mod has the given checksum.
		void CheckSum(bool& result, const uint32_t sum, string file);

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
	class ini_grammar : public grammar<grammarIter, vector<IniPair>(), Skipper> {
	public:
		ini_grammar();
		inline void SetErrorBuffer(ParsingError * inErrorBuffer) { errorBuffer = inErrorBuffer; }
	private:
		
		qi::rule<grammarIter, Skipper> heading;
		qi::rule<grammarIter, vector<IniPair>(), Skipper> ini;
		qi::rule<grammarIter, IniPair(), Skipper> setting;
		qi::rule<grammarIter, string(), Skipper> var, stringVal;
	
		void SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, info const& what);

		ParsingError * errorBuffer;
	};

	////////////////////////////
	//RuleList Grammar.
	////////////////////////////

	//RuleList grammar.
	class userlist_grammar : public qi::grammar<grammarIter, vector<Rule>(), Skipper> {
	public:
		userlist_grammar();
		inline void SetParsingErrorBuffer(ParsingError * inErrorBuffer) { parsingErrorBuffer = inErrorBuffer; }
		inline void SetSyntaxErrorBuffer(vector<ParsingError> * inErrorBuffer) { syntaxErrorBuffer = inErrorBuffer; }
	private:
		qi::rule<grammarIter, vector<Rule>(), Skipper> ruleList;
		qi::rule<grammarIter, Rule(), Skipper> userlistRule;
		qi::rule<grammarIter, RuleLine(), Skipper> sortOrMessageLine;
		qi::rule<grammarIter, uint32_t(), Skipper> ruleKey, sortOrMessageKey;
		qi::rule<grammarIter, string(), Skipper> object;
		qi::rule<grammarIter, bool(), Skipper> stateKey;
	
		void SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, info const& what);

		void RuleSyntaxCheck(vector<Rule>& userlist, Rule currentRule);

		ParsingError * parsingErrorBuffer;
		vector<ParsingError> * syntaxErrorBuffer;
	};

}
#endif
