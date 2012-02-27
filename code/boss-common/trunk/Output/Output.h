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

//Contains the various functions/classes required for varied BOSSlog output formattings,etc.

#ifndef __BOSS_BOSSLOG_H__
#define __BOSS_BOSSLOG_H__

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <boost/format.hpp>

#include "Common/Classes.h"
#include "Common/DllDef.h"

namespace boss {
	using namespace std;

	//Default filter options.
	BOSS_COMMON extern bool UseDarkColourScheme;
	BOSS_COMMON extern bool HideVersionNumbers;
	BOSS_COMMON extern bool HideGhostedLabel;
	BOSS_COMMON extern bool HideActiveLabel;
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
	BOSS_COMMON extern bool HideInactivePlugins;

	//Default CSS.
	BOSS_COMMON extern string CSSBody;
	BOSS_COMMON extern string CSSDarkBody;
	BOSS_COMMON extern string CSSDarkLink;
	BOSS_COMMON extern string CSSDarkLinkVisited;
	BOSS_COMMON extern string CSSFilters;
	BOSS_COMMON extern string CSSFiltersList;
	BOSS_COMMON extern string CSSDarkFilters;
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
	BOSS_COMMON extern string CSSSubmit;
	BOSS_COMMON extern string CSSPopupBox;
	BOSS_COMMON extern string CSSPopupBoxTitle;
	BOSS_COMMON extern string CSSPopupBoxLink;
	BOSS_COMMON extern string CSSPopupBoxNotes;
	BOSS_COMMON extern string CSSPopupBoxClose;
	BOSS_COMMON extern string CSSPopupBoxSubmit;
	BOSS_COMMON extern string CSSMask;
	BOSS_COMMON extern string CSSActive;

	enum logFormatting : uint32_t {
		DIV_OPEN,
		DIV_CLOSE,
		LINE_BREAK,
		TABLE_OPEN,
		TABLE_CLOSE,
		TABLE_ROW,
		TABLE_DATA,
		LIST_ID_RECOGNISED_OPEN,
		LIST_ID_USERLIST_MESSAGES_OPEN,
		LIST_OPEN,
		LIST_CLOSE,
		LIST_ITEM,
		LIST_ITEM_CLASS_SUCCESS,
		LIST_ITEM_CLASS_WARN,
		LIST_ITEM_CLASS_ERROR,
		LIST_ITEM_SPAN_CLASS_MOD_OPEN,
		HEADING_ID_END_OPEN,
		HEADING_OPEN,
		HEADING_CLOSE,
		PARAGRAPH,
		SPAN_CLASS_VERSION_OPEN,
		SPAN_CLASS_GHOSTED_OPEN,
		SPAN_CLASS_CRC_OPEN,
		SPAN_CLASS_ACTIVE_OPEN,
		SPAN_CLASS_ERROR_OPEN,
		SPAN_CLASS_MESSAGE_OPEN,
		SPAN_CLOSE,
		ITALIC_OPEN,
		ITALIC_CLOSE,
		BLOCKQUOTE_OPEN,
		BLOCKQUOTE_CLOSE,
		BUTTON_SUBMIT_PLUGIN
	};

	class BOSS_COMMON Outputter {
	public:
		Outputter();
		Outputter(uint32_t format);

		void SetFormat(uint32_t format);	//Sets the formatting type of the output.
		void SetHTMLSpecialEscape(bool shouldEscape);	//Set when formatting is set, generally, but this can be used to override.
		void Clear();			//Erase all current content.

		void PrintHeader();		//Prints BOSS Log header.
		void PrintFooter();		//Prints BOSS Log footer.

		void Save(fs::path file, bool overwrite);		//Saves contents to file. 
														//Throws boss_error exception on fail.
		string AsString();				//Outputs contents as a string.
		
		Outputter& operator<< (const string s);
		Outputter& operator<< (const char * s);
		Outputter& operator<< (const char c);
		Outputter& operator<< (const logFormatting l);
		Outputter& operator<< (const int32_t i);
		Outputter& operator<< (const uint32_t i);
		Outputter& operator<< (const bool b);
		Outputter& operator<< (const fs::path p);
		Outputter& operator<< (const Message m);
	private:
		stringstream outStream;
		uint32_t outFormat;			//The formatting type of the output.
		bool escapeHTMLSpecialChars;	//Should special characters be escaped from non-formatting input?
	
		string EscapeHTMLSpecial(string text);	//Performs the HTML escaping.
		string EscapeHTMLSpecial(char c);
	};
}
#endif