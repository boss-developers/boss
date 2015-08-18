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

#ifndef PARSING_GRAMMAR_H_
#define PARSING_GRAMMAR_H_

#ifndef BOOST_SPIRIT_UNICODE
#	define BOOST_SPIRIT_UNICODE
#endif

#include <boost/version.hpp>
#if BOOST_VERSION == 105500
#	define BOOST_SPIRIT_USE_PHOENIX_V3 1
#endif

#include <cstdint>

#include <string>
#include <utility>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "common/conditional_data.h"
#include "common/error.h"
#include "common/game.h"
#include "common/rule_line.h"

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
	(std::uint32_t, key)
	(std::string, data)
)

BOOST_FUSION_ADAPT_STRUCT(
	boss::Item,
	(std::string, conditions)
	(std::uint32_t, type)
	(std::string, data)
	(std::vector<boss::Message>, messages)
)

//////////////////////////////
// RuleList Data Conversions
//////////////////////////////

BOOST_FUSION_ADAPT_STRUCT(
	boss::RuleLine,
	(std::uint32_t, key)
	(std::string, object)
)

BOOST_FUSION_ADAPT_STRUCT(
	boss::Rule,
	(bool, enabled)
	(std::uint32_t, key)
	(std::string, object)
	(std::vector<boss::RuleLine>, lines)
)


namespace boss {

namespace fs = boost::filesystem;
namespace qi = boost::spirit::qi;

using qi::grammar;
using boost::spirit::info;

//typedef boost::u8_to_u32_iterator<std::string::const_iterator> iterator_type;
//typedef iterator_type grammarIter;
typedef std::string::const_iterator grammarIter;

///////////////////////////////
// Keyword structures
///////////////////////////////

struct ruleKeys_ : qi::symbols<char, std::uint32_t> {
	ruleKeys_();
};

struct messageKeys_ : qi::symbols<char, std::uint32_t> {
	messageKeys_();
};

struct masterlistMsgKey_ : qi::symbols<char, std::uint32_t> {
	masterlistMsgKey_();
};

struct typeKey_ : qi::symbols<char, std::uint32_t> {
	typeKey_();
};


///////////////////////////////
// Skipper Grammar
///////////////////////////////

// Skipper for userlist, modlist and ini parsers.
class Skipper : public grammar<grammarIter> {
 public:
	Skipper();
	void SkipIniComments(const bool b);
 private:
	qi::rule<grammarIter> start, spc, eof, CComment, CPlusPlusComment, lineComment, iniComment, UTF8;
};

///////////////////////////////
// Modlist/Masterlist Grammar
///////////////////////////////

// Modlist/Masterlist grammar
class modlist_grammar
    : public grammar<grammarIter, std::vector<Item>(), Skipper> {
 public:
	modlist_grammar();
	void SetErrorBuffer(ParsingError *inErrorBuffer);
	void SetGlobalMessageBuffer(std::vector<Message> *inGlobalMessageBuffer);
	void SetVarStore(std::vector<MasterlistVar> *varStore);
	void SetCRCStore(boost::unordered_map<std::string, std::uint32_t> *CRCStore);
	void SetParentGame(const Game *game);
 private:
	// Parser error reporter.
	void SyntaxError(const grammarIter /*&first*/,
	                 const grammarIter &last,
	                 const grammarIter &errorpos,
	                 const boost::spirit::info &what);

	// Stores the given item and records any changes to open groups.
	void StoreItem(std::vector<Item> &list, Item currentItem);

	// Stores the masterlist variable.
	void StoreVar(const MasterlistVar var);

	// Stores the global message.
	void StoreGlobalMessage(const Message message);

	// Turns a given string into a path. Can't be done directly because of the openGroups checks.
	void ToName(std::string &p, std::string itemName);

	qi::rule<grammarIter, std::vector<Item>(), Skipper> modList;
	qi::rule<grammarIter, Item(), Skipper> listItem;
	qi::rule<grammarIter, std::uint32_t(), Skipper> ItemType;
	qi::rule<grammarIter, std::string(), Skipper> itemName;
	qi::rule<grammarIter, std::vector<Message>(), Skipper> itemMessages;
	qi::rule<grammarIter, Message(), Skipper> itemMessage, globalMessage;
	qi::rule<grammarIter, MasterlistVar(), Skipper> listVar;
	qi::rule<grammarIter, std::string(), Skipper> charString, andOr, conditional, conditionals, functCondition, shortCondition, variable, file, checksum, version, comparator, regex, language;
	qi::rule<grammarIter, std::uint32_t(), Skipper> messageKeyword;
	ParsingError *errorBuffer;
	std::vector<Message> *globalMessageBuffer;
	std::vector<MasterlistVar> *setVars;                         // Vars set by masterlist.
	boost::unordered_map<std::string, std::uint32_t> *fileCRCs;  // CRCs calculated.
	const Game *parentGame;
	std::vector<std::string> openGroups;                         // Need to keep track of which groups are open to match up endings properly in MF1.
};


////////////////////////////
// Conditional Grammar
////////////////////////////

class conditional_grammar : public grammar<grammarIter, bool(), Skipper> {
 public:
	conditional_grammar();
	void SetErrorBuffer(ParsingError *inErrorBuffer);
	void SetVarStore(boost::unordered_set<std::string> *varStore);
	void SetCRCStore(boost::unordered_map<std::string, std::uint32_t> *CRCStore);
	void SetActivePlugins(boost::unordered_set<std::string> *plugins);
	void SetLastConditionalResult(bool *result);
	void SetParentGame(const Game *game);
 private:
	// Evaluate a single conditional.
	void EvaluateConditional(bool &result, const std::string type,
	                         const bool condition);

	// Evaluate the second half of a complex conditional.
	void EvaluateCompoundConditional(bool &lhsCondition,
	                                 const std::string andOr,
	                                 const bool rhsCondition);

	void EvalElseConditional(bool &result, bool &ok);

	// Returns the true path based on what type of file or keyword it is.
	fs::path GetPath(const std::string file);

	// Checks if the given file (plugin or dll/exe) has a version for which the comparison holds true.
	void CheckVersion(bool &result, const std::string file,
	                  const std::string version, const char comparator);

	// Checks if the given file exists.
	void CheckFile(bool &result, std::string file);

	/*
	 * Checks if a file which matches the given regex exists.
	 * This might not work when the regex specifies a file and a path, eg. "path/to/file.txt", because characters like '.' need to be escaped in regex
	 * so the regex would be "path/to/file\.txt". boost::filesystem might interpret that as a path of "path / to / file / .txt" though.
	 * In windows, the above path would be "path\to\file.txt", which would become "path\\to\\file\.txt" in regex. Basically, the extra backslashes need to
	 * be removed when getting the path and filename.
	 */
	void CheckRegex(bool &result, std::string reg);

	// Checks if a masterlist variable is defined.
	void CheckVar(bool &result, const std::string var);

	// Checks if the given plugin is active.
	void CheckActive(bool &result, const std::string plugin);

	// Checks if the given language is the current language.
	void CheckLanguage(bool &result, const std::string language);

	// Checks if the given mod has the given checksum.
	void CheckSum(bool &result, std::string file, const std::uint32_t sum);

	// Parser error reporter.
	void SyntaxError(const grammarIter /*&first*/,
	                 const grammarIter &last,
	                 const grammarIter &errorpos,
	                 const boost::spirit::info &what);

	qi::rule<grammarIter, std::string(), Skipper> ifIfNot, andOr, variable, file, version, regex, language;
	qi::rule<grammarIter, bool(), Skipper> conditional, conditionals, condition, shortCondition, functCondition;
	qi::rule<grammarIter, std::uint32_t(), Skipper> checksum;
	qi::rule<grammarIter, char(), Skipper> comparator;
	ParsingError *errorBuffer;
	boost::unordered_set<std::string> *setVars;                  // Vars set by masterlist.
	boost::unordered_set<std::string> *activePlugins;            // Active plugins, with lowercase filenames.
	boost::unordered_map<std::string, std::uint32_t> *fileCRCs;  // CRCs calculated.
	const Game *parentGame;
	bool *lastResult;
};


////////////////////////////
// Ini Grammar
////////////////////////////

// Ini grammar
class ini_grammar : public grammar<grammarIter, boost::unordered_map<std::string, std::string>(), Skipper> {
 public:
	ini_grammar();
	void SetErrorBuffer(ParsingError *inErrorBuffer);
 private:
	void SyntaxError(const grammarIter /*&first*/,
	                 const grammarIter &last,
	                 const grammarIter &errorpos,
	                 const boost::spirit::info &what);

	qi::rule<grammarIter, Skipper> heading;
	qi::rule<grammarIter, boost::unordered_map<std::string, std::string>(), Skipper> ini;
	qi::rule<grammarIter, std::pair<std::string, std::string>(), Skipper> setting;
	qi::rule<grammarIter, std::string(), Skipper> var, stringVal;

	ParsingError *errorBuffer;
};

////////////////////////////
// RuleList Grammar
////////////////////////////

// RuleList grammar
class userlist_grammar : public qi::grammar<grammarIter, std::vector<Rule>(), Skipper> {
 public:
	userlist_grammar();
	void SetErrorBuffer(std::vector<ParsingError> *inErrorBuffer);
 private:
	void SyntaxError(const grammarIter /*&first*/,
	                 const grammarIter &last,
	                 const grammarIter &errorpos,
	                 const boost::spirit::info &what);

	qi::rule<grammarIter, std::vector<Rule>(), Skipper> ruleList;
	qi::rule<grammarIter, Rule(), Skipper> userlistRule;
	qi::rule<grammarIter, RuleLine(), Skipper> sortOrMessageLine;
	qi::rule<grammarIter, std::uint32_t(), Skipper> ruleKey, sortOrMessageKey;
	qi::rule<grammarIter, std::string(), Skipper> object;
	qi::rule<grammarIter, bool(), Skipper> stateKey;

	std::vector<ParsingError> *errorBuffer;
};

}  // namespace boss
#endif  // PARSING_GRAMMAR_H_
