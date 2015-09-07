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

#include "output/output.h"

#include <cstddef>
#include <cstdint>

#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "common/conditional_data.h"
#include "common/error.h"
#include "common/globals.h"
#include "common/keywords.h"
#include "common/rule_line.h"
#include "support/helpers.h"

namespace boss {

namespace fs = boost::filesystem;
namespace bloc = boost::locale;

////////////////////////////////
// Outputter Class Functions
////////////////////////////////

// MCP Note: Possibly condense some of these constructors using default values for paramaters? Not sure about that, though...
Outputter::Outputter()
    : outFormat(PLAINTEXT),
      escapeHTMLSpecialChars(false) {}

Outputter::Outputter(const Outputter &o) {
	outStream << o.AsString();
	outFormat = o.GetFormat();
	escapeHTMLSpecialChars = o.GetHTMLSpecialEscape();
}

Outputter::Outputter(const std::uint32_t format)
    : outFormat(format) {
	if (outFormat == HTML)
		escapeHTMLSpecialChars = true;
	else
		escapeHTMLSpecialChars = false;
}

Outputter::Outputter(const std::uint32_t format, const ParsingError e)
    : outFormat(format) {
	if (outFormat == HTML)
		escapeHTMLSpecialChars = true;
	else
		escapeHTMLSpecialChars = false;

	*this << e;
}

Outputter::Outputter(const std::uint32_t format, const Rule r)
    : outFormat(format) {
	if (outFormat == HTML)
		escapeHTMLSpecialChars = true;
	else
		escapeHTMLSpecialChars = false;

	*this << r;
}

Outputter::Outputter(const std::uint32_t format, const logFormatting l)
    : outFormat(format) {
	if (outFormat == HTML)
		escapeHTMLSpecialChars = true;
	else
		escapeHTMLSpecialChars = false;

	*this << l;
}

void Outputter::SetFormat(const std::uint32_t format) {
	outFormat = format;
	if (outFormat == HTML)
		escapeHTMLSpecialChars = true;
	else
		escapeHTMLSpecialChars = false;
}

void Outputter::SetHTMLSpecialEscape(const bool shouldEscape) {
	escapeHTMLSpecialChars = shouldEscape;
}

void Outputter::Clear() {
	outStream.str(std::string());
}

bool Outputter::Empty() const {
	return outStream.str().empty();
}

std::uint32_t Outputter::GetFormat() const {
	return outFormat;
}

bool Outputter::GetHTMLSpecialEscape() const {
	return escapeHTMLSpecialChars;
}

std::string Outputter::AsString() const {
	return outStream.str();
}

Outputter& Outputter::operator= (const Outputter &o) {
	outStream << o.AsString();
	outFormat = o.GetFormat();
	escapeHTMLSpecialChars = o.GetHTMLSpecialEscape();
	return *this;
}

Outputter& Outputter::operator<< (const std::string s) {
	outStream << EscapeHTMLSpecial(s);
	return *this;
}

Outputter& Outputter::operator<< (const char *s) {
	outStream << EscapeHTMLSpecial(s);
	return *this;
}

Outputter& Outputter::operator<< (const char c) {
	outStream << EscapeHTMLSpecial(c);
	return *this;
}

Outputter& Outputter::operator<< (const logFormatting l) {
	switch (l) {
		case SECTION_ID_SUMMARY_OPEN:
			if (outFormat == HTML)
				outStream << "<section id='summary'>";
			break;
		case SECTION_ID_USERLIST_OPEN:
			if (outFormat == HTML)
				outStream << "<section id='userRules'>";
			break;
		case SECTION_ID_SE_OPEN:
			if (outFormat == HTML)
				outStream << "<section id='sePlugins'>";
			break;
		case SECTION_ID_RECOGNISED_OPEN:
			if (outFormat == HTML)
				outStream << "<section id='recPlugins'>";
			break;
		case SECTION_ID_UNRECOGNISED_OPEN:
			if (outFormat == HTML)
				outStream << "<section id='unrecPlugins'>";
			break;
		case SECTION_CLOSE:
			if (outFormat == HTML)
				outStream << "</section>";
			break;
		case DIV_OPEN:
			if (outFormat == HTML)
				outStream << "<div>";
			else
				outStream << std::endl;
			break;
		case DIV_SUMMARY_BUTTON_OPEN:
			if (outFormat == HTML)
				outStream << "<div class='button' data-section='summary'>";
			break;
		case DIV_USERLIST_BUTTON_OPEN:
			if (outFormat == HTML)
				outStream << "<div class='button' data-section='userRules'>";
			break;
		case DIV_SE_BUTTON_OPEN:
			if (outFormat == HTML)
				outStream << "<div class='button' data-section='sePlugins'>";
			break;
		case DIV_RECOGNISED_BUTTON_OPEN:
			if (outFormat == HTML)
				outStream << "<div class='button' data-section='recPlugins'>";
			break;
		case DIV_UNRECOGNISED_BUTTON_OPEN:
			if (outFormat == HTML)
				outStream << "<div class='button' data-section='unrecPlugins'>";
			break;
		case DIV_CLOSE:
			if (outFormat == HTML)
				outStream << "</div>";
			break;
		case LINE_BREAK:
			if (outFormat == HTML)
				outStream << "<br />";
			else
				outStream << std::endl;
			break;
		case TABLE_HEAD:
			if (outFormat == HTML)
				outStream << "<thead>";
			break;
		case TABLE_HEADING:
			if (outFormat == HTML)
				outStream << "<th>";
			else
				outStream << '\t';
			break;
		case TABLE_BODY:
			if (outFormat == HTML)
				outStream << "<tbody>";
			break;
		case TABLE_ROW:
			if (outFormat == HTML)
				outStream << "<tr>";
			else
				outStream << std::endl;
			break;
		case TABLE_ROW_CLASS_SUCCESS:
			if (outFormat == HTML)
				outStream << "<tr class='success'>";
			else
				outStream << std::endl;
			break;
		case TABLE_ROW_CLASS_WARN:
			if (outFormat == HTML)
				outStream << "<tr class='warn'>";
			else
				outStream << std::endl;
			break;
		case TABLE_ROW_CLASS_ERROR:
			if (outFormat == HTML)
				outStream << "<tr class='error'>";
			else
				outStream << std::endl;
			break;
		case TABLE_DATA:
			if (outFormat == HTML)
				outStream << "<td>";
			else
				outStream  << '\t';
			break;
		case TABLE_OPEN:
			if (outFormat == HTML)
				outStream << "<table>";
			break;
		case TABLE_CLOSE:
			if (outFormat == HTML)
				outStream << "</table>";
			else
				outStream << std::endl << std::endl;
			break;
		case LIST_OPEN:
			if (outFormat == HTML)
				outStream << "<ul>";
			break;
		case LIST_CLOSE:
			if (outFormat == HTML)
				outStream << "</ul>";
			else
				outStream << std::endl;
			break;
		case HEADING_OPEN:
			if (outFormat == HTML)
				outStream << "<h2>";
			else
				outStream << std::endl << std::endl << "======================================" << std::endl;
			break;
		case HEADING_CLOSE:
			if (outFormat == HTML)
				outStream << "</h2>";
			else
				outStream << std::endl << "======================================" << std::endl << std::endl;
			break;
		case PARAGRAPH:
			if (outFormat == HTML)
				outStream << "<p>";
			else
				outStream << std::endl;
			break;
		case LIST_ITEM:
			if (outFormat == HTML)
				outStream << "<li>";
			else
				outStream << std::endl << std::endl;
			break;
		case LIST_ITEM_CLASS_SUCCESS:
			if (outFormat == HTML)
				outStream << "<li class='success'>";
			else
				outStream << std::endl << "*  ";
			break;
		case LIST_ITEM_CLASS_WARN:
			if (outFormat == HTML)
				outStream << "<li class='warn'>";
			else
				outStream << std::endl << "*  ";
			break;
		case LIST_ITEM_CLASS_ERROR:
			if (outFormat == HTML)
				outStream << "<li class='error'>";
			else
				outStream << std::endl << "*  ";
			break;
		case SPAN_ID_UNRECPLUGINSSUBMITNOTE_OPEN:
			if (outFormat == HTML)
				outStream << "<span id='unrecPluginsSubmitNote'>";
			break;
		case SPAN_CLASS_MOD_OPEN:
			if (outFormat == HTML)
				outStream << "<span class='mod'>";
			break;
		case SPAN_CLASS_VERSION_OPEN:
			if (outFormat == HTML)
				outStream << "<span class='version'>&nbsp;";
			else
				outStream << " ";
			break;
		case SPAN_CLASS_CRC_OPEN:
			if (outFormat == HTML)
				outStream << "<span class='crc'>&nbsp;";
			else
				outStream << " ";
			break;
		case SPAN_CLASS_ACTIVE_OPEN:
			if (outFormat == HTML)
				outStream << "<span class='active'>&nbsp;";
			else
				outStream << " ";
			break;
		case SPAN_CLASS_MESSAGE_OPEN:
			if (outFormat == HTML)
				outStream << "<span class='message'>";
			break;
		case SPAN_CLOSE:
			if (outFormat == HTML)
				outStream << "</span>";
			break;
		case ITALIC_OPEN:
			if (outFormat == HTML)
				outStream << "<i>";
			break;
		case ITALIC_CLOSE:
			if (outFormat == HTML)
				outStream << "</i>";
			break;
		case BLOCKQUOTE_OPEN:
			if (outFormat == HTML)
				outStream << "<blockquote>";
			else
				outStream << std::endl << std::endl;
			break;
		case BLOCKQUOTE_CLOSE:
			if (outFormat == HTML)
				outStream << "</blockquote>";
			else
				outStream << std::endl << std::endl;
			break;
		case VAR_OPEN:
			if (outFormat == HTML)
				outStream << "<var>";
			else
				outStream << "\"";
			break;
		case VAR_CLOSE:
			if (outFormat == HTML)
				outStream << "</var>";
			else
				outStream << "\"";
			break;
		default:
			break;
	}
	return *this;
}

Outputter& Outputter::operator<< (const std::int32_t i) {
	outStream << i;
	return *this;
}

Outputter& Outputter::operator<< (const std::uint32_t i) {
	outStream << i;
	return *this;
}

Outputter& Outputter::operator<< (const bool b) {
	if (b)
		outStream << "true";
	else
		outStream << "false";
	return *this;
}

Outputter& Outputter::operator<< (const fs::path p) {
	*this << p.string();
	return *this;
}

Outputter& Outputter::operator<< (const Message m) {
	std::string data = EscapeHTMLSpecial(m.Data());
	// Need to handle web addresses. Recognised are those in the following formats:
	// "http:someAddress", "http:someAddress label", "https:someAddress", "https:someAddress label", "file:somelocalAddress", "file:someLocalAddress label"

	std::size_t pos1, pos2, pos3;
	std::string link, label, dq;
	std::string addressTypes[] = {"http:", "https:", "file:"};  // MCP Note: Should this be a const and should it be char**?

	if (outFormat == HTML)
		dq = "&quot;";
	else
		dq = "\"";

	// Do replacements for all addressTypes.
	for (std::uint32_t i = 0; i < 2; i++) {
		pos1 = data.find(dq + addressTypes[i]);
		while (pos1 != std::string::npos) {
			pos1 += dq.length();
			pos3 = data.find(dq, pos1);  // End of quoted string.
			// Check if there is a label in the quoted string.
			pos2 = data.find(' ', pos1);
			if (pos2 < pos3) {  // Label present.
				link = data.substr(pos1, pos2 - pos1);
				label = data.substr(pos2 + 1, pos3 - pos2 - 1);
			} else {  // Label not present.
				link = data.substr(pos1, pos3 - pos1);
				label = link;
			}
			if (outFormat == HTML)
				link = "<a href=\"" + link + "\">" + label + "</a>";
			else if (pos2 > pos3)
				link = '"' + link + '"';
			else
				link = label + " (\"" + link + "\")";
			data.replace(pos1 - dq.length(), pos3 - pos1 + (2 * dq.length()), link);
			pos1 = data.find(dq + addressTypes[i], pos1);
		}
	}
	// TODO(MCP): Look at converting this to a switch-statement.
	// Select message formatting.
	if (m.Key() == SAY) {
		if (outFormat == HTML)
			outStream << "<li class='note'>" << bloc::translate("Note") << ": " << data;
		else
			outStream << std::endl << "*  " << bloc::translate("Note") << ": " << data;
	} else if (m.Key() == TAG) {
		if (outFormat == HTML)
			outStream << "<li class='tag'><span class='tagPrefix'>" << bloc::translate("Bash Tag suggestion(s)") << ":</span> " << data;
		else
			outStream << std::endl << "*  " << bloc::translate("Bash Tag suggestion(s)") << ": " << data;
	} else if (m.Key() == REQ) {
		if (outFormat == HTML)
			outStream << "<li class='req'>" << bloc::translate("Requires") << ": " << data;
		else
			outStream << std::endl << "*  " << bloc::translate("Requires") << ": " << data;
	} else if (m.Key() == INC) {
		if (outFormat == HTML)
			outStream << "<li class='inc'>" << bloc::translate("Incompatible with") << ": " << data;
		else
			outStream << std::endl << "*  " << bloc::translate("Incompatible with") << ": " << data;
	} else if (m.Key() == WARN) {
		if (outFormat == HTML)
			outStream << "<li class='warn'>" << bloc::translate("Warning") << ": " << data;
		else
			outStream << std::endl << "*  " << bloc::translate("Warning") << ": " << data;
	} else if (m.Key() == ERR) {
		if (outFormat == HTML)
			outStream << "<li class='error'>" << bloc::translate("Error") << ": " << data;
		else
			outStream << std::endl << "*  " << bloc::translate("Error") << ": " << data;
	} else if (m.Key() == DIRTY) {
		if (outFormat == HTML)
			outStream << "<li class='dirty'>" << bloc::translate("Contains dirty edits") << ": " << data;
		else
			outStream << std::endl << "*  " << bloc::translate("Contains dirty edits") << ": " << data;
	} else {
		if (outFormat == HTML)
			outStream << "<li class='note'>" << bloc::translate("Note") << ": " << data;
		else
			outStream << std::endl << "*  " << bloc::translate("Note") << ": " << data;
	}
	return *this;
}

Outputter& Outputter::operator<< (const ParsingError e) {
	if (e.Empty()) {
		return *this;
	} else if (!e.WholeMessage().empty()) {
		*this << LIST_ITEM_CLASS_ERROR << e.WholeMessage();
	} else {
		bool htmlEscape = escapeHTMLSpecialChars;
		escapeHTMLSpecialChars = true;
		*this << LIST_ITEM_CLASS_ERROR << e.Header()
		      << BLOCKQUOTE_OPEN << e.Detail() << BLOCKQUOTE_CLOSE
		      << e.Footer();
		escapeHTMLSpecialChars = htmlEscape;
	}
	return *this;
}

Outputter& Outputter::operator<< (const Rule r) {
	bool hasEditedMessages = false;
	std::vector<RuleLine> lines = r.Lines();
	std::size_t linesSize = lines.size();
	std::string varOpen = Outputter(outFormat, VAR_OPEN).AsString();
	std::string varClose = Outputter(outFormat, VAR_CLOSE).AsString();

	// Need to temporarily turn off escaping of special characters so that <var> and </var> are printed correctly.
	bool wasEscaped = escapeHTMLSpecialChars;

	for (std::size_t j = 0; j < linesSize; j++) {
		std::string rObject = varOpen + EscapeHTMLSpecial(r.Object()) + varClose;
		std::string lObject = varOpen + EscapeHTMLSpecial(lines[j].Object()) + varClose;

		escapeHTMLSpecialChars = false;

		// TODO(MCP): Look at converting this to a switch-statement
		if (lines[j].Key() == BEFORE) {
			*this << (boost::format(bloc::translate("Sort %1% before %2%")) % rObject % lObject).str();
		} else if (lines[j].Key() == AFTER) {
			*this << (boost::format(bloc::translate("Sort %1% after %2%")) % rObject % lObject).str();
		} else if (lines[j].Key() == TOP) {
			*this << (boost::format(bloc::translate("Insert %1% at the top of %2%")) % rObject % lObject).str();
		} else if (lines[j].Key() == BOTTOM) {
			*this << (boost::format(bloc::translate("Insert %1% at the bottom of %2%")) % rObject % lObject).str();
		} else if (lines[j].Key() == APPEND) {
			if (!hasEditedMessages) {
				if (r.Key() == FOR)
					*this << (boost::format(bloc::translate("Add the following messages to %1%:")) % rObject).str() << LINE_BREAK << LIST_OPEN;
				else
					*this << LINE_BREAK << bloc::translate("Add the following messages:") << LINE_BREAK << LIST_OPEN;
			}
			*this << lines[j].ObjectAsMessage();
			hasEditedMessages = true;
		} else if (lines[j].Key() == REPLACE) {
			if (!hasEditedMessages) {
				if (r.Key() == FOR)
					*this << (boost::format(bloc::translate("Replace the messages attached to %1% with:")) % rObject).str() << LINE_BREAK << LIST_OPEN;
				else
					*this << LINE_BREAK << bloc::translate("Replace the attached messages with:") << LINE_BREAK << LIST_OPEN;
			}
			*this << lines[j].ObjectAsMessage();
			hasEditedMessages = true;
		}

		escapeHTMLSpecialChars = wasEscaped;
	}
	if (hasEditedMessages)
		*this << LIST_CLOSE;

	escapeHTMLSpecialChars = wasEscaped;

	return *this;
}

std::string Outputter::EscapeHTMLSpecial(std::string text) {
	if (escapeHTMLSpecialChars && outFormat == HTML) {
		boost::replace_all(text, "&", "&amp;");
		boost::replace_all(text, "\"", "&quot;");
		boost::replace_all(text, "'", "&#039;");
		boost::replace_all(text, "<", "&lt;");
		boost::replace_all(text, ">", "&gt;");
		boost::replace_all(text, "©", "&copy;");
		boost::replace_all(text, "✗", "&#x2717;");
		boost::replace_all(text, "✓", "&#x2713;");
		boost::replace_all(text, "\n", "<br />");  // Not an HTML special char escape, but this needs to happen here to get the details of parser errors formatted correctly.
	}
	return text;
}

std::string Outputter::EscapeHTMLSpecial(char c) {
	if (escapeHTMLSpecialChars && outFormat == HTML) {
		/*
		 * MCP Note: GCC squalls at these due to the copyright character not being ASCII. Look at trying to fix this.
		 * Maybe changing it to a wchar_t would fix it? Need to look into that. Or maybe use ints?
		 * MCP Note 2: For the default, would it be better to leave as-is or change it to a break-statement,
		 * remove the else, and simply return string(1, c)?
		 */
		switch (c) {
			case '&':
				return "&amp;";
			case '"':
				return "&quot;";
			case '\'':
				return "&#039;";
			case '<':
				return "&lt;";
			case '>':
				return "&gt;";
			case '©':
				return "&copy;";
			default:
				return std::string(1, c);
		}
	}
	return std::string(1, c);
}

}  // namespace boss
