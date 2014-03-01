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
		SECTION_ID_SUMMARY_OPEN,
		SECTION_ID_USERLIST_OPEN,
		SECTION_ID_SE_OPEN,
		SECTION_ID_RECOGNISED_OPEN,
		SECTION_ID_UNRECOGNISED_OPEN,
		SECTION_CLOSE,
		DIV_OPEN,
		DIV_SUMMARY_BUTTON_OPEN,
		DIV_USERLIST_BUTTON_OPEN,
		DIV_SE_BUTTON_OPEN,
		DIV_RECOGNISED_BUTTON_OPEN,
		DIV_UNRECOGNISED_BUTTON_OPEN,
		DIV_CLOSE,
		LINE_BREAK,
		TABLE_OPEN,
		TABLE_CLOSE,
		TABLE_HEAD,
		TABLE_HEADING,
		TABLE_BODY,
		TABLE_ROW,
		TABLE_ROW_CLASS_SUCCESS,
		TABLE_ROW_CLASS_WARN,
		TABLE_ROW_CLASS_ERROR,
		TABLE_DATA,
		LIST_OPEN,
		LIST_CLOSE,
		LIST_ITEM,
		LIST_ITEM_CLASS_SUCCESS,
		LIST_ITEM_CLASS_WARN,
		LIST_ITEM_CLASS_ERROR,
		HEADING_OPEN,
		HEADING_CLOSE,
		PARAGRAPH,
		SPAN_ID_UNRECPLUGINSSUBMITNOTE_OPEN,
		SPAN_CLASS_MOD_OPEN,
		SPAN_CLASS_VERSION_OPEN,
		SPAN_CLASS_CRC_OPEN,
		SPAN_CLASS_ACTIVE_OPEN,
		SPAN_CLASS_MESSAGE_OPEN,
		SPAN_CLOSE,
		ITALIC_OPEN,
		ITALIC_CLOSE,
		BLOCKQUOTE_OPEN,
		BLOCKQUOTE_CLOSE,
		VAR_OPEN,
		VAR_CLOSE
	};

	class BOSS_COMMON Outputter {
	public:
		Outputter();
		Outputter(const Outputter& o);
		Outputter(const uint32_t format);
		Outputter(const uint32_t format, const ParsingError e);
		Outputter(const uint32_t format, const Rule r);
		Outputter(const uint32_t format, const logFormatting l);

		void SetFormat(const uint32_t format);	//Sets the formatting type of the output.
		void SetHTMLSpecialEscape(const bool shouldEscape);	//Set when formatting is set, generally, but this can be used to override.
		void Clear();			//Erase all current content.

		bool Empty() const;
		uint32_t GetFormat() const;
		bool GetHTMLSpecialEscape() const;

		string AsString() const;				//Outputs contents as a string.
		
		Outputter& operator= (const Outputter& o);
		Outputter& operator<< (const string s);
		Outputter& operator<< (const char * s);
		Outputter& operator<< (const char c);
		Outputter& operator<< (const logFormatting l);
		Outputter& operator<< (const int32_t i);
		Outputter& operator<< (const uint32_t i);
		Outputter& operator<< (const bool b);
		Outputter& operator<< (const fs::path p);
		Outputter& operator<< (const Message m);
		Outputter& operator<< (const ParsingError e);
		Outputter& operator<< (const Rule r);
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
		boost::unordered_map<char, uint32_t> map1252toUtf8; //1252, UTF-8. 128-255, minus a few common characters.
		boost::unordered_map<uint32_t, char> utf8toEnc;
		boost::unordered_map<char, uint32_t> encToUtf8;
		uint32_t currentEncoding;
	public:
		Transcoder();
		void SetEncoding(const uint32_t inEncoding);
		uint32_t GetEncoding();

		string Utf8ToEnc(const string inString);
		string EncToUtf8(const string inString);
	};

	class BossLog {
	public:
		BossLog();
		BossLog(const uint32_t format);

		void SetFormat(const uint32_t format);
		void Save(const fs::path file, const bool overwrite);		//Saves contents to file. Throws boss_error exception on fail.
		void Clear();

		uint32_t recognised; 
		uint32_t unrecognised;
		uint32_t inactive;
		uint32_t messages;
		uint32_t warnings;
		uint32_t errors;

		string scriptExtender;
		string gameName;

		Outputter updaterOutput;
		Outputter criticalError;
		Outputter userRules;
		Outputter sePlugins;
		Outputter recognisedPlugins;
		Outputter unrecognisedPlugins;

		vector<ParsingError> parsingErrors;
		vector<Message> globalMessages;
	private:
		uint32_t logFormat;
		bool recognisedHasChanged;

		string PrintLog();
		string PrintHeaderTop();
		string PrintHeaderBottom();
		string PrintFooter();

		bool HasRecognisedListChanged(const fs::path file);
	};
}
#endif