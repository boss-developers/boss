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

// Contains the various functions/classes required for varied BOSSlog output formattings,etc.

#ifndef OUTPUT_OUTPUT_H_
#define OUTPUT_OUTPUT_H_

#include <cstdint>

#include <sstream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "common/conditional_data.h"
#include "common/dll_def.h"
#include "common/error.h"

namespace boss {

class BOSS_COMMON Rule;

enum logFormatting : std::uint32_t {
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
	// MCP Note: Mark this as explicit?
	Outputter(const Outputter &o);
	explicit Outputter(const std::uint32_t format);
	Outputter(const std::uint32_t format, const ParsingError e);
	Outputter(const std::uint32_t format, const Rule r);
	Outputter(const std::uint32_t format, const logFormatting l);

	void SetFormat(const std::uint32_t format);          // Sets the formatting type of the output.
	void SetHTMLSpecialEscape(const bool shouldEscape);  // Set when formatting is set, generally, but this can be used to override.
	void Clear();                                        // Erase all current content.

	bool Empty() const;
	std::uint32_t GetFormat() const;
	bool GetHTMLSpecialEscape() const;

	std::string AsString() const;  // Outputs contents as a string.

	Outputter& operator= (const Outputter &o);
	Outputter& operator<< (const std::string s);
	Outputter& operator<< (const char *s);
	Outputter& operator<< (const char c);
	Outputter& operator<< (const logFormatting l);
	Outputter& operator<< (const std::int32_t i);
	Outputter& operator<< (const std::uint32_t i);
	Outputter& operator<< (const bool b);
	Outputter& operator<< (const boost::filesystem::path p);
	Outputter& operator<< (const Message m);
	Outputter& operator<< (const ParsingError e);
	Outputter& operator<< (const Rule r);

 private:
	std::string EscapeHTMLSpecial(std::string text);  // Performs the HTML escaping.
	std::string EscapeHTMLSpecial(char c);

	std::stringstream outStream;
	std::uint32_t outFormat;      // The formatting type of the output.
	bool escapeHTMLSpecialChars;  // Should special characters be escaped from non-formatting input?
};

}  // namespace boss
#endif  // OUTPUT_OUTPUT_H_
