/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains the various functions/classes required for varied BOSSlog output formattings,etc.
//Still at the brainstorming stage.
//The idea is to separate the unformatted text from the formatting. Unformatted text is generated in main() and passed through something found
//here to be formatted appropriately.

#ifndef __BOSS_BOSSLOG_H__
#define __BOSS_BOSSLOG_H__

#include <fstream>
#include <string>
#include "Common/Lists.h"
#include "Common/DllDef.h"
#include <boost/format.hpp>

namespace boss {
	using namespace std;

	using boost::format;

	//Default filter options.
	BOSS_COMMON extern bool UseDarkColourScheme;
	BOSS_COMMON extern bool HideVersionNumbers;
	BOSS_COMMON extern bool HideGhostedLabel;
	BOSS_COMMON extern bool HideChecksums;
	BOSS_COMMON extern bool HideMessagelessMods;
	BOSS_COMMON extern bool HideGhostedMods;
	BOSS_COMMON extern bool HideCleanMods;
	BOSS_COMMON extern bool HideRuleWarnings;
	BOSS_COMMON extern bool HideAllModMessages;
	BOSS_COMMON extern bool HideNotes;
	BOSS_COMMON extern bool HideBashTagSuggestions;
	BOSS_COMMON extern bool HideRequirements;
	BOSS_COMMON extern bool HideIncompatibilities;
	BOSS_COMMON extern bool HideDoNotCleanMessages;

	//Default CSS.
	BOSS_COMMON extern string CSSBody;
	BOSS_COMMON extern string CSSFilters;
	BOSS_COMMON extern string CSSFiltersList;
	BOSS_COMMON extern string CSSTitle;
	BOSS_COMMON extern string CSSCopyright;
	BOSS_COMMON extern string CSSSections;
	BOSS_COMMON extern string CSSSectionTitle;
	BOSS_COMMON extern string CSSSectionPlusMinus;
	BOSS_COMMON extern string CSSLastSection;
	BOSS_COMMON extern string CSSTable;
	BOSS_COMMON extern string CSSList;
	BOSS_COMMON extern string CSSListItem;
	BOSS_COMMON extern string CSSSubList;
	BOSS_COMMON extern string CSSCheckbox;
	BOSS_COMMON extern string CSSBlockquote;
	BOSS_COMMON extern string CSSError;
	BOSS_COMMON extern string CSSWarning;
	BOSS_COMMON extern string CSSSuccess;
	BOSS_COMMON extern string CSSVersion;
	BOSS_COMMON extern string CSSGhost;
	BOSS_COMMON extern string CSSCRC;
	BOSS_COMMON extern string CSSTagPrefix;
	BOSS_COMMON extern string CSSDirty;
	BOSS_COMMON extern string CSSQuotedMessage;
	BOSS_COMMON extern string CSSMod;
	BOSS_COMMON extern string CSSTag;
	BOSS_COMMON extern string CSSNote;
	BOSS_COMMON extern string CSSRequirement;
	BOSS_COMMON extern string CSSIncompatibility;
	
	//Parsing error message format.
	static format MasterlistParsingErrorFormat("<p><span class='error'>Masterlist Parsing Error: Expected a %1% at:</span>"
		"<blockquote>%2%</blockquote>"
		"<span class='error'>Masterlist parsing aborted. Utility will end now.</span>");
	
	//Parsing error formatting.
	static format IniParsingErrorFormat("<li><span class='error'>Ini Parsing Error: Expected a %1% at:</span>"
		"<blockquote>%2%</blockquote>"
		"<span class='error'>Ini parsing aborted. Some or all of the options may not have been set correctly.</span>");

	//Syntax error formatting.
	static format SyntaxErrorFormat("<li class='error'>"
		"Userlist Syntax Error: The rule beginning \"%1%: %2%\" %3%"
		"");

	//Parsing error formatting.
	static format UserlistParsingErrorFormat("<li><span class='error'>Userlist Parsing Error: Expected a %1% at:</span>"
		"<blockquote>%2%</blockquote>"
		"<span class='error'>Userlist parsing aborted. No rules will be applied.</span>");


	//Prints a given message to the bosslog, using format-safe Output function below.
	void ShowMessage(string& buffer, message currentMessage);

	//Prints ouptut with formatting according to format.
	BOSS_COMMON void Output(string text);
	
	//Escapes HTML special characters.
	BOSS_COMMON string EscapeHTMLSpecial(string text);

	//Prints HTML header.
	BOSS_COMMON void OutputHeader();

	//Prints HTML footer (ie. </body> and </html> tags).
	BOSS_COMMON void OutputFooter();

	//Generate a default BOSS.ini
	BOSS_COMMON bool GenerateIni();

	//Converts an integer to a string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
	BOSS_COMMON string IntToString(const unsigned int n);

	//Converts an integer to a hex string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
	string IntToHexString(const unsigned int n);

	//Converts a boolean to a string representation (0/1)
	string BoolToString(bool b);
}
#endif