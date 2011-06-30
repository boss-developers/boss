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
#include "Lists.h"

namespace boss {
	using namespace std;

	//Default filter options.
	extern bool UseDarkColourScheme;
	extern bool HideVersionNumbers;
	extern bool HideGhostedLabel;
	extern bool HideChecksums;
	extern bool HideMessagelessMods;
	extern bool HideGhostedMods;
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
	extern string CSSTopLevelList;
	extern string CSSLastSection;
	extern string CSSLastSectionTitle;
	extern string CSSTopLevelListItem;
	extern string CSSList;
	extern string CSSListItem;
	extern string CSSItemList;
	extern string CSSCheckbox;
	extern string CSSBlockquote;
	extern string CSSUnrecognisedList;
	extern string CSSSummaryRow;
	extern string CSSSummaryCell;
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
	

	//Prints a given message to the bosslog, using format-safe Output function below.
	void ShowMessage(string& buffer, message currentMessage);

	//Prints ouptut with formatting according to format.
	void Output(string text);

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
}
#endif