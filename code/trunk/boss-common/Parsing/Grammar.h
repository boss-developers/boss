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
#include "Common/Game.h"
#include <string>
#include <vector>
#include <utility>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/std_pair.hpp>
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
	(uint32_t, type)
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

	struct typeKey_ : qi::symbols<char, uint32_t> {
		typeKey_();
	};


	///////////////////////////////
	//Skipper Grammar
	///////////////////////////////
	
	//Skipper for userlist, modlist and ini parsers.
	class Skipper : public grammar<grammarIter> {
	public:
		Skipper();
		void SkipIniComments(const bool b);
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
		void SetErrorBuffer(ParsingError * inErrorBuffer);
		void SetGlobalMessageBuffer(vector<Message> * inGlobalMessageBuffer);
		void SetVarStore(vector<MasterlistVar> * varStore);
		void SetCRCStore(boost::unordered_map<string,uint32_t> * CRCStore);
		void SetParentGame(const Game * game);
	private:
		qi::rule<grammarIter, vector<Item>(), Skipper> modList;
		qi::rule<grammarIter, Item(), Skipper> listItem;
		qi::rule<grammarIter, uint32_t(), Skipper> ItemType;
		qi::rule<grammarIter, string(), Skipper> itemName;
		qi::rule<grammarIter, vector<Message>(), Skipper> itemMessages;
		qi::rule<grammarIter, Message(), Skipper> itemMessage, globalMessage;
		qi::rule<grammarIter, MasterlistVar(), Skipper> listVar;
		qi::rule<grammarIter, string(), Skipper> charString, andOr, conditional, conditionals, functCondition, shortCondition, variable, file, checksum, version, comparator, regex, language;
		qi::rule<grammarIter, uint32_t(), Skipper> messageKeyword;
		ParsingError * errorBuffer;
		vector<Message> * globalMessageBuffer;
		vector<MasterlistVar> * setVars;					//Vars set by masterlist.
		boost::unordered_map<string,uint32_t> * fileCRCs;	//CRCs calculated.
		const Game * parentGame;
		vector<string> openGroups;  //Need to keep track of which groups are open to match up endings properly in MF1.

		//Parser error reporter.
		void SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, boost::spirit::info const& what);

		//Stores the given item and records any changes to open groups.
		void StoreItem(vector<Item>& list, Item currentItem);

		//Stores the masterlist variable.
		void StoreVar(const MasterlistVar var);

		//Stores the global message.
		void StoreGlobalMessage(const Message message);

		//Turns a given string into a path. Can't be done directly because of the openGroups checks.
		void ToName(string& p, string itemName);
	};


	////////////////////////////
	// Conditional Grammar
	////////////////////////////

	class conditional_grammar : public grammar<grammarIter, bool(), Skipper> {
	public:
		conditional_grammar();
		void SetErrorBuffer(ParsingError * inErrorBuffer);
		void SetVarStore(boost::unordered_set<string> * varStore);
		void SetCRCStore(boost::unordered_map<string,uint32_t> * CRCStore);
		void SetActivePlugins(boost::unordered_set<string> * plugins);
		void SetLastConditionalResult(bool * result);
		void SetParentGame(const Game * game);
	private:
		qi::rule<grammarIter, string(), Skipper> ifIfNot, andOr, variable, file, version, regex, language;
		qi::rule<grammarIter, bool(), Skipper> conditional, conditionals, condition, shortCondition, functCondition;
		qi::rule<grammarIter, uint32_t(), Skipper> checksum;
		qi::rule<grammarIter, char(), Skipper> comparator;
		ParsingError * errorBuffer;
		boost::unordered_set<string> * setVars;				//Vars set by masterlist.
		boost::unordered_set<string> * activePlugins;		//Active plugins, with lowercase filenames.
		boost::unordered_map<string,uint32_t> * fileCRCs;	//CRCs calculated.
		const Game * parentGame;
		bool * lastResult;

		//Evaluate a single conditional.
		void EvaluateConditional(bool& result, const string type, const bool condition);

		//Evaluate the second half of a complex conditional.
		void EvaluateCompoundConditional(bool& lhsCondition, const string andOr, const bool rhsCondition);

		void EvalElseConditional(bool& result, bool& ok);

		//Returns the true path based on what type of file or keyword it is.
		fs::path GetPath(const string file);

		//Checks if the given file (plugin or dll/exe) has a version for which the comparison holds true.
		void CheckVersion(bool& result, const string file, const string version, const char comparator);

		//Checks if the given file exists.
		void CheckFile(bool& result, string file);

		//Checks if a file which matches the given regex exists.
		//This might not work when the regex specifies a file and a path, eg. "path/to/file.txt", because characters like '.' need to be escaped in regex
		//so the regex would be "path/to/file\.txt". boost::filesystem might interpret that as a path of "path / to / file / .txt" though.
		//In windows, the above path would be "path\to\file.txt", which would become "path\\to\\file\.txt" in regex. Basically, the extra backslashes need to
		//be removed when getting the path and filename.
		void CheckRegex(bool& result, string reg);
		
		//Checks if a masterlist variable is defined.
		void CheckVar(bool& result, const string var);

		//Checks if the given plugin is active.
		void CheckActive(bool& result, const string plugin);

		//Checks if the given language is the current language.
		void CheckLanguage(bool& result, const string language);

		//Checks if the given mod has the given checksum.
		void CheckSum(bool& result, string file, const uint32_t sum);

		//Parser error reporter.
		void SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, boost::spirit::info const& what);
	};


	////////////////////////////
	//Ini Grammar.
	////////////////////////////

	//Ini grammar.
	class ini_grammar : public grammar<grammarIter, boost::unordered_map<string, string>(), Skipper> {
	public:
		ini_grammar();
		void SetErrorBuffer(ParsingError * inErrorBuffer);
	private:
		qi::rule<grammarIter, Skipper> heading;
		qi::rule<grammarIter, boost::unordered_map<string, string>(), Skipper> ini;
		qi::rule<grammarIter, pair<string, string>(), Skipper> setting;
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
		void SetErrorBuffer(vector<ParsingError> * inErrorBuffer);
	private:
		qi::rule<grammarIter, vector<Rule>(), Skipper> ruleList;
		qi::rule<grammarIter, Rule(), Skipper> userlistRule;
		qi::rule<grammarIter, RuleLine(), Skipper> sortOrMessageLine;
		qi::rule<grammarIter, uint32_t(), Skipper> ruleKey, sortOrMessageKey;
		qi::rule<grammarIter, string(), Skipper> object;
		qi::rule<grammarIter, bool(), Skipper> stateKey;
	
		void SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, info const& what);

		vector<ParsingError> * errorBuffer;
	};

}
#endif
