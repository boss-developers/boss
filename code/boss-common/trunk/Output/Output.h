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
#include <boost/format.hpp>

namespace boss {
	using namespace std;

	using boost::format;

	//Default filter options.
	extern bool UseDarkColourScheme;
	extern bool HideVersionNumbers;
	extern bool HideGhostedLabel;
	extern bool HideChecksums;
	extern bool HideMessagelessMods;
	extern bool HideGhostedMods;
	extern bool HideCleanMods;
	extern bool HideRuleWarnings;
	extern bool HideAllModMessages;
	extern bool HideNotes;
	extern bool HideBashTagSuggestions;
	extern bool HideRequirements;
	extern bool HideIncompatibilities;

	//Default CSS.
	extern string CSSBody;
	extern string CSSFilters;
	extern string CSSFiltersList;
	extern string CSSTitle;
	extern string CSSCopyright;
	extern string CSSSections;
	extern string CSSSectionTitle;
	extern string CSSSectionPlusMinus;
	extern string CSSLastSection;
	extern string CSSTable;
	extern string CSSList;
	extern string CSSListItem;
	extern string CSSSubList;
	extern string CSSCheckbox;
	extern string CSSBlockquote;
	extern string CSSError;
	extern string CSSWarning;
	extern string CSSSuccess;
	extern string CSSVersion;
	extern string CSSGhost;
	extern string CSSCRC;
	extern string CSSTagPrefix;
	extern string CSSDirty;
	extern string CSSQuotedMessage;
	extern string CSSMod;
	extern string CSSTag;
	extern string CSSNote;
	extern string CSSRequirement;
	extern string CSSIncompatibility;
	
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
	void Output(string text);
	
	//Escapes HTML special characters.
	string EscapeHTMLSpecial(string text);

	//Prints HTML header.
	void OutputHeader();

	//Prints HTML footer (ie. </body> and </html> tags).
	void OutputFooter();

	//Generate a default BOSS.ini
	bool GenerateIni();

	//Converts an integer to a string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
	string IntToString(unsigned int n);

	//Converts an integer to a hex string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
	string IntToHexString(unsigned int n);

	//Converts a boolean to a string representation (0/1)
	string BoolToString(bool b);
}
#endif