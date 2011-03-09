/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains functions for userlist and modlist/masterlist parsing.

#include <fstream>
#include "Parser.h"
#include "Modlist/Grammar.h"
#include "Userlist/Grammar.h"
#include "Support/Helpers.h"
#include "Common/BOSSLog.h"
#include "Common/Globals.h"
#include <boost/algorithm/string.hpp>

namespace boss {
	using namespace std;
	namespace qi = boost::spirit::qi;

	

	bool parseUserlist(fs::path file, vector<rule>& ruleList) {
		Skipper<string::const_iterator> skipper;
		userlist_grammar<string::const_iterator> grammar;
		string::const_iterator begin, end;
		string contents;

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();
		bool r = qi::phrase_parse(begin, end, grammar, skipper, ruleList);

		 if (r && begin == end)
			 return true;
		 else {
			 while (*begin == '\n') {
				begin++;
			}
			 const string & const_contents(contents);
			 string::difference_type lines = 1 + count(const_contents.begin(), begin, '\n');
			 ParsingFailed(const_contents.begin(), end, begin, lines);
			 return false;
		 }
	}

	bool parseOldMasterlist(fs::path file, vector<item>& modList) {
		Skipper<string::const_iterator> skipper;
		modlist_old_grammar<string::const_iterator> grammar;
		string::const_iterator begin, end;
		string contents;

		//Check for FCOM,OOO and BC.
		if (fs::exists(data_path / "Oblivion.esm")) {
			if (fs::exists(data_path / "FCOM_Convergence.esm"))
				setVars.insert("FCOM");
			if (fs::exists(data_path / "Oscuro's_Oblivion_Overhaul.esm"))
				setVars.insert("OOO");
			if (fs::exists(data_path / "Better Cities Resources.esm"))
				setVars.insert("BC");
		} else if (fs::exists(data_path / "Fallout3.esm")) {
			if (fs::exists(data_path / "FOOK2 - Main.esm"))
				setVars.insert("FOOK");
			if (fs::exists(data_path / "FO3 Wanderers Edition - Main File.esm"))
				setVars.insert("FWE");
		}
		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();
		bool r = qi::phrase_parse(begin, end, grammar, skipper, modList);

		 if (r && begin == end)
			 return true;
		 else {
			 while (*begin == '\n') {
				begin++;
			}
			 const string & const_contents(contents);
			 string::difference_type lines = 1 + count(const_contents.begin(), begin, '\n');
			 ParsingFailed(const_contents.begin(), end, begin, lines);
			 return false;
		 }
	}

/* NOTES: USERLIST GRAMMAR RULES.

	Below are the grammar rules that the parser must follow. Noted here until they are implemented.

S	1. Userlist must be encoded in UTF-8 or UTF-8 compatible ANSI. Use checking functions and abort the userlist if mangled line found.
S	2. All lines must contain a recognised keyword and an object. If one is missing or unrecognised, abort the rule.
T	3. If a rule object is a mod, it must be installed. If not, abort the rule.
T	4. Groups cannot be added. If a rule tries, abort it.
T	5. The 'ESMs' group cannot be sorted. If a rule tries, abort it.
T	6. The game's main master file cannot be sorted. If a rule tries, abort it.
T	7. A rule with a FOR rule keyword must not contain a sort line. If a rule tries, ignore the line and print a warning message.
T	8. A rule may not reference a mod and a group unless its sort keyword is TOP or BOTTOM and the mod is the rule object.  If a rule tries, abort it.
T	9. No group may be sorted before the 'ESMs' group. If a rule tries, abort it.
T	10. No mod may be sorted before the game's main master file. If a rule tries, abort it.
T	11. No mod may be inserted into the top of the 'ESMs' group. If a rule tries, abort it.
T	12. No rule can insert a group into anything or insert anything into a mod. If a rule tries, abort it.
T   13. No rule may attach a message to a group. If a rule tries, abort it.
S	14. The first line of a rule must be a rule line. If there is a valid line before the first rule line, ignore it and print a warning message.
*/

	//Rule checker function, checks for syntax (not parsing) errors.
	void RuleSyntaxCheck(rule currentRule) {

		keyType ruleKey = currentRule.ruleKey;
		string subject = currentRule.ruleObject;

		try {
			if (IsPlugin(subject)) {

				if (!Exists(subject))
					throw failure(ruleKey, subject, EPluginNotInstalled % subject);

				if (IsMasterFile(subject))
					throw failure(ruleKey, subject, ESortingMasterEsm);

			} else {

				if (Tidy(subject) == "esms")
					throw failure(ruleKey, subject, ESortingGroupEsms);

				if (ruleKey == ADD && !IsPlugin(subject))
					throw failure(ruleKey, subject, EAddingModGroup);

				else if (ruleKey == FOR)
					throw failure(ruleKey, subject, EAttachingMessageToGroup);

			}
			for (size_t i=0; i<currentRule.lines.size(); i++) {

				keyType key = currentRule.lines[i].key;
				subject = currentRule.lines[i].object;

				if (key == BEFORE || key == AFTER) {

					if (currentRule.ruleKey == FOR)
						throw failure(ruleKey, subject, ESortLineInForRule);

					if ((IsPlugin(currentRule.ruleObject) && !IsPlugin(subject)) || (!IsPlugin(currentRule.ruleObject) && IsPlugin(subject))) {
						throw failure(ruleKey, subject, EReferencingModAndGroup);
					}

					if (key == BEFORE) {

						if (Tidy(subject) == "esms")
							throw failure(ruleKey, subject, ESortingGroupBeforeEsms);

						else if (IsMasterFile(subject))
							throw failure(ruleKey, subject, ESortingModBeforeGameMaster);

					}

				} else if (key == TOP || key == BOTTOM) {

					if (ruleKey == FOR)
						throw failure(ruleKey, subject, ESortLineInForRule);

					if (key == TOP && Tidy(subject)=="esms")
						throw failure(ruleKey, subject, EInsertingToTopOfEsms);
						
					if (!IsPlugin(currentRule.ruleObject) || IsPlugin(subject)) {
						throw failure(ruleKey, subject, EInsertingGroupToGroupOrModToMod);
					}
				} else if (key == APPEND || key == REPLACE) {
					if (!IsPlugin(currentRule.ruleObject))
						throw failure(ruleKey, subject, EAttachingMessageToGroup);
				}
			}
		} catch(failure const& e) {
			AddError(e.rule, e.object, e.message);
		}
	}

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

	const string FormatMesssage(string const& class_, format const& message)
	{
		return (MessageSpanFormat % class_ % message.str()).str();
	}

	const string FormatMesssage(string const& class_, keyType const& rule, string const& object, format const& message)
	{
		string const span = FormatMesssage(class_ , message);
		return (MessageParagraphFormat % rule % object % span).str();
	}

	

	void AddMessage(keyType const& rule, string const& object, format const& message, string const& class_)
	{
		string msg = FormatMesssage(class_, rule, object, message);
		//Output message to BOSSlog using output function.
		Output(bosslog,HTML,msg);
	}

	void AddError(keyType const& rule, string const& object, format const& message)
	{
		AddMessage(rule, object, message, "error");
	}

	void AddError(format const& message)
	{
		string msg = FormatMesssage("error", message);
		//Output message to BOSSlog using output function.
		Output(bosslog,HTML,msg);
	}

	// Called when an error is detected while parsing the input file.
	void SyntaxError(
			string::const_iterator const& begin, 
			string::const_iterator const& end, 
			string::const_iterator const& error_pos, 
			string const& what) 
	{
		std::string context(error_pos, std::min(error_pos + 50, end));
		boost::trim_left(context);
		boost::replace_all(context, "\n", "<eol>");

		std::cerr << "Syntax error while trying to parse Userlist.txt: '" << what << "' near this input: '" << context << "'." << std::endl;
	}

	void ParsingFailed(
			string::const_iterator	const& begin, 
			string::const_iterator	const& end, 
			string::const_iterator	const& error_pos, 
			string::difference_type lineNo)
	{
		string context(error_pos, std::min(error_pos + 50, end));
		boost::trim_left(context);
		boost::replace_all(context, "\n", "<eol>");

		std::cerr << "Userlist.txt parsing error at line #" << lineNo << " while reading near this input: '" << context << "'." << std::endl;
	}
}