/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/
//Header file for userlist parser functions.

#ifndef __BOSS_USERLIST_PARSER_H__
#define __BOSS_USERLIST_PARSER_H__

#include <string>
#include <vector>
#include "boost/filesystem.hpp"
#include "Common/Lists.h"
#include <boost/format.hpp>

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;
	using boost::format;

	void fileToBuffer(fs::path file, string& buffer);

	bool parseUserlist(fs::path file, vector<rule>& ruleList);

	void RuleSyntaxCheck(rule ruleToCheck);

	struct failure {
		failure(keyType const& rule, string const& object, boost::format const& message) 
			: object(object), rule(rule), message(message)
		{}

		string object;
		keyType rule;
		boost::format message;
	};

	// Error messages for rule validation
	//Syntax errors
	static boost::format ESortLineInForRule("It includes a sort line in a rule with a FOR rule keyword.");
	static boost::format EPluginNotInstalled("'%1%' is not installed.");
	static boost::format EAddingModGroup("It tries to add a group.");
	static boost::format ESortingGroupEsms("It tries to sort the group \"ESMs\".");
	static boost::format ESortingMasterEsm("It tries to sort the master .ESM file.");
	static boost::format EReferencingModAndGroup("It references a mod and a group.");
	static boost::format ESortingGroupBeforeEsms("It tries to sort a group before the group \"ESMs\".");
	static boost::format ESortingModBeforeGameMaster("It tries to sort a mod before the master .ESM file.");
	static boost::format EInsertingToTopOfEsms("It tries to insert a mod into the top of the group \"ESMs\", before the master .ESM file.");
	static boost::format EInsertingGroupToGroupOrModToMod("It tries to insert a group or insert a mod into another mod.");
	static boost::format EAttachingMessageToGroup("It tries to attach a message to a group.");
	//Grammar errors.
	static boost::format ERuleHasUndefinedObject("The line with keyword '%1%' has an undefined object.");
	static boost::format EUnrecognisedKeyword("The line \"%1%: %2%\" does not contain a recognised keyword. If this line is the start of a rule, that rule will also be skipped.");
	static boost::format EAppearsBeforeFirstRule("The line \"%1%: %2%\" appears before the first recognised rule line. Line skipped.");
	static boost::format EUnrecognizedKeywordBeforeFirstRule("The line \"%1%: %2%\" does not contain a recognised keyword, and appears before the first recognised rule line. Line skipped.");
	
	static boost::format MessageParagraphFormat(
		"<p style='margin-left:40px; text-indent:-40px;'\n"
			"	The rule beginning \" %1%: %2%\" has been skipped as it has the following problem(s):\n"
			"	<br/>\n"
			"	%3%\n"
			"	<br/>\n"
			"</p>\n"
		);

	static boost::format MessageSpanFormat(
		"	<span class='%1%'>\n"
			"		%2%\n"
			"	</span>\n"
		);


	const string FormatMesssage(string const& class_, format const& message);

	const string FormatMesssage(string const& class_, keyType const& rule, string const& object, format const& message);

	void AddMessage(keyType const& rule, string const& object, format const& message, string const& class_);

	void AddError(keyType const& rule, string const& object, format const& message);

	void AddError(format const& message);

	// Called when an error is detected while parsing the input file.
	void SyntaxError(
			string::const_iterator const& begin, 
			string::const_iterator const& end, 
			string::const_iterator const& error_pos, 
			string const& what);

	void ParsingFailed(
			string::const_iterator	const& begin, 
			string::const_iterator	const& end, 
			string::const_iterator	const& error_pos, 
			string::difference_type lineNo);
}
#endif