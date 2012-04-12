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
#include <boost/unordered_set.hpp>

#include "Common/Classes.h"
#include "Common/DllDef.h"

namespace boss {
	using namespace std;

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
		HEADING_ID_SUMMARY_OPEN,
		HEADING_ID_GENERAL_OPEN,
		HEADING_ID_USERLIST_OPEN,
		HEADING_ID_SE_OPEN,
		HEADING_ID_RECOGNISEDSEC_OPEN,
		HEADING_ID_UNRECOGNISED_OPEN,
		HEADING_CLOSE,
		PARAGRAPH,
		SPAN_CLASS_MOD_OPEN,
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

	enum langString : uint32_t {
		LOG_UseDarkColourScheme,
		LOG_HideRuleWarnings,
		LOG_HideVersionNumbers,
		LOG_HideGhostedLabel,
		LOG_HideActiveLabel,
		LOG_HideChecksums,
		LOG_HideInactiveMods,
		LOG_HideMessagelessMods,
		LOG_HideGhostedMods,
		LOG_HideCleanMods,
		LOG_HideAllModMessages,
		LOG_HideNotes,
		LOG_HideBashTagSuggestions,
		LOG_HideRequirements,
		LOG_HideIncompatibilities,
		LOG_HideDoNotCleanMessages,
	//	# of # plugins hidden
	//	# of # messages hidden
		LOG_SummaryHeading,
		LOG_MasterlistUpToDate,
		LOG_MasterlistUpdated,
		LOG_NoUpdateNecessary,
		LOG_RecognisedCount,
		LOG_WarningsCount,
		LOG_UnrecognisedCount,
		LOG_ErrorsCount,
		LOG_GhostedPluginsCount,
		LOG_TotalMessageNumber,
		LOG_TotalPluginNumber,
		LOG_UserRuleCountNote,
		LOG_SEHeading,
		LOG_RecognisedHeading,
		LOG_UnrecognisedHeading,
		LOG_UnrecognisedNote,
		LOG_MessageNote,
		LOG_MessageRequires,
		LOG_MessageIncompatible,
		LOG_MessageBashTag,
		LOG_MessageDirty,
		LOG_MessageWarning,
		LOG_MessageError
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
		Outputter& operator<< (const langString l);
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

	class BOSS_COMMON Transcoder {
	private:
		//0x81, 0x8D, 0x8F, 0x90, 0x9D in 1252 are undefined in UTF-8.
		boost::unordered_map<char, uint32_t> commonMap;  //1251/1252, UTF-8. 0-127, plus some more.
	//	boost::unordered_map<char, uint32_t> map1251toUtf8; //1251, UTF-8. 128-255, minus a few common characters.
		boost::unordered_map<char, uint32_t> map1252toUtf8; //1252, UTF-8. 128-255, minus a few common characters.
		boost::unordered_map<uint32_t, char> utf8toEnc;
		boost::unordered_map<char, uint32_t> encToUtf8;
		uint32_t currentEncoding;
	public:
		Transcoder();
		void SetEncoding(uint32_t inEncoding);
		uint32_t GetEncoding();

		string Utf8ToEnc(string inString);
		string EncToUtf8(string inString);
	};
}
#endif