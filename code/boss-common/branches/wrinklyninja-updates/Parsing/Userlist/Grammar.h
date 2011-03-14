/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Header file for userlist grammar definition.

/* Need to:
1. Get error handling working properly (currently throws silent exceptions and doesn't specify area).
2. Make error reports useful.
*/
#ifndef __BOSS_USERLIST_GRAM_H__
#define __BOSS_USERLIST_GRAM_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include "Parsing/Data.h"
#include "Parsing/Skipper.h"
#include "Support/Helpers.h"
#include "Common/Globals.h"
#include "Common/BOSSLog.h"
#include <sstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

#include <boost/format.hpp>

namespace boss {
	namespace unicode = boost::spirit::unicode;
	namespace phoenix = boost::phoenix;
	namespace qi = boost::spirit::qi;

	using namespace std;
	using namespace qi::labels;

	using qi::skip;
	using qi::eol;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;

	using unicode::char_;
	using unicode::no_case;

	using boost::spirit::info;
	using boost::format;

	////////////////////////////
	//Userlist Grammar.
	////////////////////////////

	//Need to structure things to that it provides more meaningful error reporting.
	//Need to know what specific component failed.

	/* NOTES: USERLIST GRAMMAR RULES.

	Below are the grammar rules that the parser must follow. Noted here until they are implemented.

	1. Userlist must be encoded in UTF-8 or UTF-8 compatible ANSI.
	2. All lines must contain a recognised keyword and an object. If one is missing or unrecognised, abort the rule.
	3. If a rule object is a mod, it must be installed. If not, abort the rule.
	4. Groups cannot be added. If a rule tries, abort it.
	5. The 'ESMs' group cannot be sorted. If a rule tries, abort it.
	6. The game's main master file cannot be sorted. If a rule tries, abort it.
	7. A rule with a FOR rule keyword must not contain a sort line. If a rule tries, ignore the line and print a warning message.
	8. A rule may not reference a mod and a group unless its sort keyword is TOP or BOTTOM and the mod is the rule object.  If a rule tries, abort it.
	9. No group may be sorted before the 'ESMs' group. If a rule tries, abort it.
	10. No mod may be sorted before the game's main master file. If a rule tries, abort it.
	11. No mod may be inserted into the top of the 'ESMs' group. If a rule tries, abort it.
	12. No rule can insert a group into anything or insert anything into a mod. If a rule tries, abort it.
    13. No rule may attach a message to a group. If a rule tries, abort it.
	14. The first line of a rule must be a rule line. If there is a valid line before the first rule line, ignore it and print a warning message.
*/

/*
	if (object.empty()) {
		throw failure(skip, rule, subject, ERuleHasUndefinedObject % key);
	}
		//Line does not contain a recognised keyword. Skip it and the rule containing it. If it is a rule line, then the previous rule will also be skipped.
		throw failure(skip, rule, subject, EUnrecognisedKeyword % key % object);
	}

	//Line is not a rule line, and appears before the first rule line, so does not belong to a rule. Skip it.
	if (key=="before" || key=="after" || key=="top" || key=="bottom" || key=="append" || key=="replace") 
		AddError(EAppearsBeforeFirstRule % key % object);
	else
		AddError(EUnrecognizedKeywordBeforeFirstRule % key % object);
*/

	// Error messages for rule validation
	//Syntax errors
	static format ESortLineInForRule("includes a sort line in a rule with a FOR rule keyword.");
	static format EPluginNotInstalled("references '%1%', which is not installed.");
	static format EAddingModGroup("tries to add a group.");
	static format ESortingGroupEsms("tries to sort the group \"ESMs\".");
	static format ESortingMasterEsm("tries to sort the master .ESM file.");
	static format EReferencingModAndGroup("references a mod and a group.");
	static format ESortingGroupBeforeEsms("tries to sort a group before the group \"ESMs\".");
	static format ESortingModBeforeGameMaster("tries to sort a mod before the master .ESM file.");
	static format EInsertingToTopOfEsms("tries to insert a mod into the top of the group \"ESMs\", before the master .ESM file.");
	static format EInsertingGroupOrIntoMod("tries to insert a group or insert something into a mod.");
	static format EAttachingMessageToGroup("tries to attach a message to a group.");
	//Grammar errors.
	static format ERuleHasUndefinedObject("The line with keyword '%1%' has an undefined object.");
	static format EUnrecognisedKeyword("The line \"%1%: %2%\" does not contain a recognised keyword. If this line is the start of a rule, that rule will also be skipped.");
	static format EAppearsBeforeFirstRule("The line \"%1%: %2%\" appears before the first recognised rule line. Line skipped.");
	static format EUnrecognizedKeywordBeforeFirstRule("The line \"%1%: %2%\" does not contain a recognised keyword, and appears before the first recognised rule line. Line skipped.");

	//Syntax error formatting.
	static format SyntaxErrorFormat("<p class='%1%'>"
		"Syntax Error: The rule beginning \"%2%: %3%\" %4%"
		"</p>");

	void AddSyntaxError(keyType const& rule, string const& object, format const& message) {
		string keystring = GetKeyString(rule);
		string const msg = (SyntaxErrorFormat % "error" % keystring % object % message.str()).str();
		messageBuffer.push_back(msg);
		return;
	}

	//Rule checker function, checks for syntax (not parsing) errors.
	void RuleSyntaxCheck(vector<rule>& userlist, rule currentRule) {
		bool skip = false;
		keyType ruleKey = currentRule.ruleKey;
		string ruleObject = currentRule.ruleObject;
		if (IsPlugin(ruleObject)) {
			if (!Exists(data_path / ruleObject)) {
				AddSyntaxError(ruleKey, ruleObject, EPluginNotInstalled % ruleObject);
				skip = true;
			}
			if (IsMasterFile(ruleObject)) {
				AddSyntaxError(ruleKey, ruleObject, ESortingMasterEsm);
				skip = true;
			}
		} else {
			if (Tidy(ruleObject) == "esms") {
				AddSyntaxError(ruleKey, ruleObject, ESortingGroupEsms);
				skip = true;
			}
			if (ruleKey == ADD && !IsPlugin(ruleObject)) {
				AddSyntaxError(ruleKey, ruleObject, EAddingModGroup);
				skip = true;
			} else if (ruleKey == FOR) {
				AddSyntaxError(ruleKey, ruleObject, EAttachingMessageToGroup);
				skip = true;
			}
		}
		for (size_t i=0; i<currentRule.lines.size(); i++) {
			keyType key = currentRule.lines[i].key;
			string subject = currentRule.lines[i].object;
			if (key == BEFORE || key == AFTER) {
				if (currentRule.ruleKey == FOR) {
					AddSyntaxError(ruleKey, ruleObject, ESortLineInForRule);
					skip = true;
				}
				if ((IsPlugin(ruleObject) && !IsPlugin(subject)) || (!IsPlugin(ruleObject) && IsPlugin(subject))) {
					AddSyntaxError(ruleKey, ruleObject, EReferencingModAndGroup);
					skip = true;
				}
				if (key == BEFORE) {
					if (Tidy(subject) == "esms") {
						AddSyntaxError(ruleKey, ruleObject, ESortingGroupBeforeEsms);
						skip = true;
					} else if (IsMasterFile(subject)) {
						AddSyntaxError(ruleKey, ruleObject, ESortingModBeforeGameMaster);
						skip = true;
					}
				}
			} else if (key == TOP || key == BOTTOM) {
				if (ruleKey == FOR) {
					AddSyntaxError(ruleKey, ruleObject, ESortLineInForRule);
					skip = true;
				}
				if (key == TOP && Tidy(subject)=="esms") {
					AddSyntaxError(ruleKey, ruleObject, EInsertingToTopOfEsms);
					skip = true;
				}
				if (!IsPlugin(ruleObject) || IsPlugin(subject)) {
					AddSyntaxError(ruleKey, ruleObject, EInsertingGroupOrIntoMod);
					skip = true;
				}
			} else if (key == APPEND || key == REPLACE) {
				if (!IsPlugin(ruleObject)) {
					AddSyntaxError(ruleKey, ruleObject, EAttachingMessageToGroup);
					skip = true;
				}
			}
		}
		if (skip)
			skip = false;
		else
			userlist.push_back(currentRule);
		return;
	}

	template <typename Iterator>
	struct userlist_grammar : qi::grammar<Iterator, vector<rule>(), Skipper<Iterator> > {
		userlist_grammar() : userlist_grammar::base_type(list, "userlist grammar") {

			//A list is a vector of rules. Rules are separated by line endings.
			list = userlistRule[phoenix::bind(&RuleSyntaxCheck, _val, _1)] % eol; 

			//A rule consists of a rule line containing a rule keyword and a rule object, followed by one or more message or sort lines.
			userlistRule %=
				*eol			//Soak up excess empty lines.
				> ((ruleKey > ':' > object)
				> +eol
				> ((sortLine | messageLine) % +eol));

			sortLine %=
				sortKey
				> ':'
				> object;

			messageLine %=
				messageKey
				> ':'
				> object;

			object %= lexeme[skip("//" >> *(char_ - eol))[+(char_ - eol)]]; //String, but skip comment if present.

			ruleKey %= no_case[ruleKeys];

			sortKey %= no_case[sortKeys];

			messageKey %= no_case[messageKeys];

			//Give each rule names.
			list.name("list");
			userlistRule.name("rule");
			sortLine.name("sort line");
			messageLine.name("message line");
			object.name("object");
			ruleKey.name("rule keyword");
			sortKey.name("sort keyword");
			messageKey.name("message keyword");
		
			on_error<fail>(list,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(userlistRule,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(sortLine,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(messageLine,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(object,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(ruleKey,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(sortKey,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(messageKey,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
		}

		typedef Skipper<Iterator> skipper;

		qi::rule<Iterator, vector<rule>(), skipper> list;
		qi::rule<Iterator, rule(), skipper> userlistRule;
		qi::rule<Iterator, line(), skipper> sortLine, messageLine;
		qi::rule<Iterator, keyType(), skipper> ruleKey, sortKey, messageKey;
		qi::rule<Iterator, string(), skipper> object;
	
		void SyntaxError(Iterator const& first, Iterator const& last, Iterator const& errorpos, info const& what) {
			cout << what;
		}
	};
}


#endif