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
namespace loc = boost::locale;

////////////////////////////////
// Outputter Class Functions
////////////////////////////////

// MCP Note: Possibly condense some of these constructors using default values for paramaters? Not sure about that, though...
Outputter::Outputter()
    : outFormat(PLAINTEXT),
      escapeHTMLSpecialChars(false) {}

Outputter::Outputter(const Outputter& o) {
	outStream << o.AsString();
	outFormat = o.GetFormat();
	escapeHTMLSpecialChars = o.GetHTMLSpecialEscape();
}

Outputter::Outputter(const std::uint32_t format) : outFormat(format) {
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

Outputter& Outputter::operator= (const Outputter& o) {
	outStream << o.AsString();
	outFormat = o.GetFormat();
	escapeHTMLSpecialChars = o.GetHTMLSpecialEscape();
	return *this;
}

Outputter& Outputter::operator<< (const std::string s) {
	outStream << EscapeHTMLSpecial(s);
	return *this;
}

Outputter& Outputter::operator<< (const char c) {
	outStream << EscapeHTMLSpecial(c);
	return *this;
}

Outputter& Outputter::operator<< (const char * s) {
	outStream << EscapeHTMLSpecial(s);
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
				outStream << endl;
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
			if (outFormat == std::HTML)
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
			if (outFormat == std::HTML)
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
		while (pos1 != string::npos) {
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
			outStream << "<li class='note'>" << loc::translate("Note") << ": " << data;
		else
			outStream << std::endl << "*  " << loc::translate("Note") << ": " << data;
	} else if (m.Key() == TAG) {
		if (outFormat == HTML)
			outStream << "<li class='tag'><span class='tagPrefix'>" << loc::translate("Bash Tag suggestion(s)") << ":</span> " << data;
		else
			outStream << std::endl << "*  " << loc::translate("Bash Tag suggestion(s)") << ": " << data;
	} else if (m.Key() == REQ) {
		if (outFormat == HTML)
			outStream << "<li class='req'>" << loc::translate("Requires") << ": " << data;
		else
			outStream << std::endl << "*  " << loc::translate("Requires") << ": " << data;
	} else if (m.Key() == INC) {
		if (outFormat == HTML)
			outStream << "<li class='inc'>" << loc::translate("Incompatible with") << ": " << data;
		else
			outStream << std::endl << "*  " << loc::translate("Incompatible with") << ": " << data;
	} else if (m.Key() == WARN) {
		if (outFormat == HTML)
			outStream << "<li class='warn'>" << loc::translate("Warning") << ": " << data;
		else
			outStream << std::endl << "*  " << loc::translate("Warning") << ": " << data;
	} else if (m.Key() == ERR) {
		if (outFormat == HTML)
			outStream << "<li class='error'>" << loc::translate("Error") << ": " << data;
		else
			outStream << std::endl << "*  " << loc::translate("Error") << ": " << data;
	} else if (m.Key() == DIRTY) {
		if (outFormat == HTML)
			outStream << "<li class='dirty'>" << loc::translate("Contains dirty edits") << ": " << data;
		else
			outStream << std::endl << "*  " << loc::translate("Contains dirty edits") << ": " << data;
	} else {
		if (outFormat == HTML)
			outStream << "<li class='note'>" << loc::translate("Note") << ": " << data;
		else
			outStream << std::endl << "*  " << loc::translate("Note") << ": " << data;
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
			*this << (boost::format(loc::translate("Sort %1% before %2%")) % rObject % lObject).str();
		} else if (lines[j].Key() == AFTER) {
			*this << (boost::format(loc::translate("Sort %1% after %2%")) % rObject % lObject).str();
		} else if (lines[j].Key() == TOP) {
			*this << (boost::format(loc::translate("Insert %1% at the top of %2%")) % rObject % lObject).str();
		} else if (lines[j].Key() == BOTTOM) {
			*this << (boost::format(loc::translate("Insert %1% at the bottom of %2%")) % rObject % lObject).str();
		} else if (lines[j].Key() == APPEND) {
			if (!hasEditedMessages) {
				if (r.Key() == FOR)
					*this << (boost::format(loc::translate("Add the following messages to %1%:")) % rObject).str() << LINE_BREAK << LIST_OPEN;
				else
					*this << LINE_BREAK << loc::translate("Add the following messages:") << LINE_BREAK << LIST_OPEN;
			}
			*this << lines[j].ObjectAsMessage();
			hasEditedMessages = true;
		} else if (lines[j].Key() == REPLACE) {
			if (!hasEditedMessages) {
				if (r.Key() == FOR)
					*this << (boost::format(loc::translate("Replace the messages attached to %1% with:")) % rObject).str() << LINE_BREAK << LIST_OPEN;
				else
					*this << LINE_BREAK << loc::translate("Replace the attached messages with:") << LINE_BREAK << LIST_OPEN;
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

////////////////////////////////
// BossLog Class Functions
////////////////////////////////

/*
 * MCP Note: Condense these two constructors into one using default values for the 'format' parameter?
 * The default would be HTML. That may help reduce duplication but needs looking into.
 */
BossLog::BossLog()
    : recognised(0),
      unrecognised(0),
      inactive(0),
      messages(0),
      warnings(0),
      errors(0),
      logFormat(HTML) {
	updaterOutput.SetFormat(HTML);
	criticalError.SetFormat(HTML);
	userRules.SetFormat(HTML);
	sePlugins.SetFormat(HTML);
	recognisedPlugins.SetFormat(HTML);
	unrecognisedPlugins.SetFormat(HTML);
}

BossLog::BossLog(const std::uint32_t format)
    : recognised(0),
      unrecognised(0),
      inactive(0),
      messages(0),
      warnings(0),
      errors(0),
      logFormat(format) {
	updaterOutput.SetFormat(format);
	criticalError.SetFormat(format);
	userRules.SetFormat(format);
	sePlugins.SetFormat(format);
	recognisedPlugins.SetFormat(format);
	unrecognisedPlugins.SetFormat(format);
}

std::string BossLog::PrintHeaderTop() {
	std::stringstream out;
	if (logFormat == HTML) {
		out << "<!DOCTYPE html>"
		    << "<meta charset='utf-8'>"
		    << "<title>BOSS Log</title>"
		    << "<link rel='stylesheet' href='../resources/style.css' />"
		    << "<nav>"
		    << "<header>"
		    << "	<h1>BOSS</h1>"
		    << loc::translate("	Version ") << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH)
		    << "</header>";
	} else {
		out << "\nBOSS\n"
		    << loc::translate("Version ") << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH) << std::endl;
	}
	return out.str();
}

std::string BossLog::PrintHeaderBottom() {
	std::stringstream out;
	if (logFormat == HTML) {
		out << "<footer>"
		    << "	<div class='button' data-section='browserBox' id='supportButtonShow'>" << loc::translate("Log Feature Support") << "</div>"
		    << "	<div class='button' id='filtersButtonToggle'>" << loc::translate("Filters") << "<span id='arrow'></span></div>"
		    << "</footer>"
		    << "</nav>"
		    << "<noscript>"
		    << loc::translate("The BOSS Log requires Javascript to be enabled in order to function.")
		    << "</noscript>";
	}
	return out.str();
}

std::string BossLog::PrintFooter() {
	std::stringstream out;
	std::string colourTooltip = loc::translate("Colours must be specified using lowercase hex codes.");

	if (logFormat == HTML) {
		out << "<section id='browserBox'>"
		    << "<p>" << loc::translate("Support for the BOSS Log's more advanced features varies. Here's what your browser supports:")
		    << "<h3>" << loc::translate("Functionality") << "</h3>"
		    << "<table>"
		    << "	<tbody>"
		    << "		<tr><td id='pluginSubmitSupport'><td>" << loc::translate("In-Log Plugin Submission") << "<td>" << loc::translate("Allows unrecognised plugins to be anonymously submitted to the BOSS team directly from the BOSS Log.")
		    << "		<tr><td id='memorySupport'><td>" << loc::translate("Settings Memory") << "<td>" << loc::translate("Allows the BOSS Log to automatically restore the filter configuration last used whenever the BOSS Log is opened.")
		    // MCP Note: Look at replacing the four spaces with a tab. It looks like a mistake but will need to verify first.
		    << "	    <tr><td id='placeholderSupport'><td>" << loc::translate("Input Placeholders")
		    << "	    <tr><td id='validationSupport'><td>" << loc::translate("Form Validation")
		    << "</table>"
		    << "</section>"

		    << "<aside>"
		    << "<label><input type='checkbox' id='hideVersionNumbers' data-class='version'/>" << loc::translate("Hide Version Numbers") << "</label>"
		    << "<label><input type='checkbox' id='hideActiveLabel' data-class='active'/>" << loc::translate("Hide 'Active' Label") << "</label>"
		    << "<label><input type='checkbox' id='hideChecksums' data-class='crc'/>" << loc::translate("Hide Checksums") << "</label>"
		    << "<label><input type='checkbox' id='hideNotes'/>" << loc::translate("Hide Notes") << "</label>"
		    << "<label><input type='checkbox' id='hideBashTags'/>" << loc::translate("Hide Bash Tag Suggestions") << "</label>"
		    << "<label><input type='checkbox' id='hideRequirements'/>" << loc::translate("Hide Requirements") << "</label>"
		    << "<label><input type='checkbox' id='hideIncompatibilities'/>" << loc::translate("Hide Incompatibilities") << "</label>"
		    << "<label><input type='checkbox' id='hideDoNotCleanMessages'/>" << loc::translate("Hide 'Do Not Clean' Messages") << "</label>"
		    << "<label><input type='checkbox' id='hideAllPluginMessages'/>" << loc::translate("Hide All Plugin Messages") << "</label>"
		    << "<label><input type='checkbox' id='hideInactivePlugins'/>" << loc::translate("Hide Inactive Plugins") << "</label>"
		    << "<label><input type='checkbox' id='hideMessagelessPlugins'/>" << loc::translate("Hide Messageless Plugins") << "</label>"
		    << "<footer>"
		    << "	" << (boost::format(loc::translate("%1% of %2% recognised plugins hidden.")) % "<span id='hiddenPluginNo'>0</span>" % recognised).str()
		    << "	" << (boost::format(loc::translate("%1% of %2% messages hidden.")) % "<span id='hiddenMessageNo'>0</span>" % messages).str()
		    << "</footer>"
		    << "</aside>"

		    << "<div id='overlay'>"
		    << "<div id='submitBox'>"
		    << "<h2>" << loc::translate("Submit Plugin") << "</h2>"
		    << "<p><span id='pluginLabel'>" << loc::translate("Plugin") << ":</span><span id='plugin'></span>"
		    << "<form>"
		    << "<label>" << loc::translate("Download Location") << ":<br /><input type='url' placeholder='" << loc::translate("Label for text box. Do not use a single quote in translation, use '&#x27;' instead", "A link to the plugin&#x27;s download location.") << "' id='link'></label>"
		    << "<label>" << loc::translate("Additional Notes") << ":<br /><textarea id='notes' placeholder='" << loc::translate("Any additional information, such as recommended Bash Tags, load order suggestions, ITM/UDR counts and dirty CRCs, can be supplied here. If no download link is available, this information is crucial.") << "'></textarea></label>"
		    << "<div id='output'></div>"
		    << "<p class='last'><button>" << loc::translate("Submit") << "</button>"
		    << "<button type='reset'>" << loc::translate("Close") << "</button>"
		    << "</form>"
		    << "</div>"
		    << "</div>"

		    // Need to define some variables in code.
		    << "<script>"
		    << "var gameName = '" << gameName << "';"
		    << "var txt1 = '" << loc::translate("Checking for existing submission...") << "';"
		    << "var txt2 = '" << loc::translate("Matching submission already exists.") << "';"
		    << "var txt3 = '" << loc::translate("Plugin already submitted. Submission updated with new comment.") << "';"
		    << "var txt4 = '" << loc::translate("Plugin submitted!") << "';"
		    << "var txt5 = '" << loc::translate("Plugin submission failed! Authorisation failure. Please report this to the BOSS team.") << "';"
		    << "var txt6 = '" << loc::translate("Plugin submission failed! GitHub API rate limit exceeded. Please try again after %1%.") << "';"
		    << "var txt7 = '" << loc::translate("Plugin submission failed!") << "';"
		    << "var txt8 = '" << loc::translate("Web storage quota for this document has been exceeded.Please empty your browser\\'s cache. Note that this will delete all locally stored data.") << "';"
		    << "var txt9 = '" << loc::translate("Please supply at least a link or some notes.") << "';"
		    << "var txt10 = '" << loc::translate("Do not clean.") << "';"
		    << "</script>"
		    << "<script src='../resources/promise-1.0.0.min.js'></script>"
		    << "<script src='../resources/octokit.js'></script>"
		    << "<script src='../resources/script.js'></script>";
	}
	return out.str();
}

std::string BossLog::PrintLog() {
	std::stringstream out;
	Outputter formattedOut(logFormat);

	// Print header
	//-------------------------

	out << PrintHeaderTop();

	if (logFormat == HTML) {
		formattedOut << DIV_SUMMARY_BUTTON_OPEN << loc::translate("Summary") << DIV_CLOSE;

		if (!userRules.Empty())
			formattedOut << DIV_USERLIST_BUTTON_OPEN << loc::translate("User Rules") << DIV_CLOSE;

		if (!sePlugins.Empty())
			formattedOut << DIV_SE_BUTTON_OPEN << (boost::format(loc::translate("%1% Plugins")) % scriptExtender).str() << DIV_CLOSE;

		if (!recognisedPlugins.Empty()) {
			if (gl_revert < 1)
				formattedOut << DIV_RECOGNISED_BUTTON_OPEN << loc::translate("Recognised Plugins") << DIV_CLOSE;
			else
				formattedOut << DIV_RECOGNISED_BUTTON_OPEN << loc::translate("Restored Load Order") << DIV_CLOSE;
		}

		if (!unrecognisedPlugins.Empty())
			formattedOut << DIV_UNRECOGNISED_BUTTON_OPEN << loc::translate("Unrecognised Plugins") << DIV_CLOSE;

		out << formattedOut.AsString() << PrintHeaderBottom();
		formattedOut.Clear();  // Clear formattedOut for re-use.
	}


	// Print Summary
	//-------------------------

	formattedOut.SetHTMLSpecialEscape(false);

	if (logFormat == HTML)
		formattedOut << SECTION_ID_SUMMARY_OPEN;
	else
		formattedOut << SECTION_ID_SUMMARY_OPEN << HEADING_OPEN << loc::translate("Summary") << HEADING_CLOSE;

	if (recognised != 0 || unrecognised != 0 || messages != 0) {
		formattedOut << TABLE_OPEN << TABLE_HEAD << TABLE_ROW << TABLE_HEADING << loc::translate("Plugin Type") << TABLE_HEADING << loc::translate("Count")
		             << TABLE_BODY << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << loc::translate("Recognised (or sorted by user rules)") << TABLE_DATA << recognised;
		if (unrecognised != 0)
			formattedOut << TABLE_ROW_CLASS_WARN << TABLE_DATA << loc::translate("Unrecognised") << TABLE_DATA << unrecognised;
		else
			formattedOut << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << loc::translate("Unrecognised") << TABLE_DATA << unrecognised;
		formattedOut << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << loc::translate("Inactive") << TABLE_DATA << inactive
		             << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << loc::translate("All") << TABLE_DATA << (recognised + unrecognised)
		             << TABLE_CLOSE

		             << TABLE_OPEN << TABLE_HEAD << TABLE_ROW << TABLE_HEADING << loc::translate("Plugin Message Type") << TABLE_HEADING << loc::translate("Count")
		             << TABLE_BODY;
		if (warnings != 0)
			formattedOut << TABLE_ROW_CLASS_WARN << TABLE_DATA << loc::translate("Warning") << TABLE_DATA << warnings;
		else
			formattedOut << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << loc::translate("Warning") << TABLE_DATA << warnings;
		if (errors != 0)
			formattedOut << TABLE_ROW_CLASS_ERROR << TABLE_DATA << loc::translate("Error") << TABLE_DATA << errors;
		else
			formattedOut << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << loc::translate("Error") << TABLE_DATA << errors;
		formattedOut << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << loc::translate("All") << TABLE_DATA << messages
		             << TABLE_CLOSE;
	}

	formattedOut << LIST_OPEN;

	formattedOut << updaterOutput.AsString();  // This contains BOSS & masterlist update strings.

	if (recognisedHasChanged)
		formattedOut << LIST_ITEM_CLASS_SUCCESS << loc::translate("No change in recognised plugin list since last run.");

	std::size_t size = parsingErrors.size();  // First print parser/syntax error messages.
	for (std::size_t i = 0; i < size; i++)
		formattedOut << parsingErrors[i];

	formattedOut << criticalError.AsString();  // Print any critical errors.

	formattedOut.SetHTMLSpecialEscape(true);
	size = globalMessages.size();
	for (std::size_t i = 0; i < size; i++)
		formattedOut << globalMessages[i];  // Print global messages.
	formattedOut.SetHTMLSpecialEscape(false);

	formattedOut << LIST_CLOSE << SECTION_CLOSE;
	out << formattedOut.AsString();
	formattedOut.Clear();  // Clear formattedOut for re-use.

	if (!criticalError.Empty()) {  // Exit early.
		out << PrintFooter();
		return out.str();
	}


	// Print User Rules
	//-------------------------

	if (!userRules.Empty()) {
		if (logFormat == HTML)
			formattedOut << SECTION_ID_USERLIST_OPEN;
		else
			formattedOut << SECTION_ID_USERLIST_OPEN << HEADING_OPEN << loc::translate("User Rules") << HEADING_CLOSE;
		formattedOut << TABLE_OPEN << TABLE_HEAD << TABLE_ROW << TABLE_HEADING << loc::translate("Rule") << TABLE_HEADING << loc::translate("Applied") << TABLE_HEADING << loc::translate("Details (if applicable)")
		             << TABLE_BODY << userRules.AsString() << TABLE_CLOSE << SECTION_CLOSE;
		out << formattedOut.AsString();
		formattedOut.Clear();
	}


	// Print Script Extender Plugins
	//--------------------------------------

	if (!sePlugins.Empty()) {
		if (logFormat == HTML)
			formattedOut << SECTION_ID_SE_OPEN;
		else
			formattedOut << SECTION_ID_SE_OPEN << HEADING_OPEN << scriptExtender << loc::translate(" Plugins") << HEADING_CLOSE;
		formattedOut << LIST_OPEN
		             << sePlugins.AsString()
		             << LIST_CLOSE << SECTION_CLOSE;
		out << formattedOut.AsString();
		formattedOut.Clear();
	}


	// Print Recognised Plugins
	//-------------------------------

	if (!recognisedPlugins.Empty()) {
		if (logFormat == HTML) {
			formattedOut << SECTION_ID_RECOGNISED_OPEN;
		} else {
			if (gl_revert < 1)
				formattedOut << SECTION_ID_RECOGNISED_OPEN << HEADING_OPEN << loc::translate("Recognised Plugins") << HEADING_CLOSE;
			else if (gl_revert == 1)
				formattedOut << SECTION_ID_RECOGNISED_OPEN << HEADING_OPEN << loc::translate("Restored Load Order (Using modlist.txt)") << HEADING_CLOSE;
			else if (gl_revert == 2)
				formattedOut << SECTION_ID_RECOGNISED_OPEN << HEADING_OPEN << loc::translate("Restored Load Order (Using modlist.old)") << HEADING_CLOSE;
		}
		formattedOut << PARAGRAPH
		             << loc::translate("These plugins are recognised by BOSS and have been sorted according to its masterlist. Please read any attached messages and act on any that require action.")
		             << LIST_OPEN
		             << recognisedPlugins.AsString()
		             << LIST_CLOSE << SECTION_CLOSE;
		out << formattedOut.AsString();
		formattedOut.Clear();
	}


	// Print Unrecognised Plugins
	//--------------------------------

	if (!unrecognisedPlugins.Empty()) {
		if (logFormat == HTML)
			formattedOut << SECTION_ID_UNRECOGNISED_OPEN;
		else
			formattedOut << SECTION_ID_UNRECOGNISED_OPEN << HEADING_OPEN << loc::translate("Unrecognised Plugins") << HEADING_CLOSE;

		formattedOut << PARAGRAPH << loc::translate("The following plugins were not found in the masterlist, and must be positioned manually, using your favourite mod manager or by using BOSS's user rules functionality.")
		             << SPAN_ID_UNRECPLUGINSSUBMITNOTE_OPEN << loc::translate(" You can submit unrecognised plugins for addition to the masterlist directly from this log by clicking on a plugin and supplying a link and/or description of its contents in the panel that is displayed.") << SPAN_CLOSE << LIST_OPEN
		             << unrecognisedPlugins.AsString()
		             << LIST_CLOSE << SECTION_CLOSE;
		out << formattedOut.AsString();
		formattedOut.Clear();
	}


	// Print footer
	//-------------------------

	out << PrintFooter();

	return out.str();
}

void BossLog::SetFormat(const std::uint32_t format) {
	logFormat = format;
	updaterOutput.SetFormat(format);
	criticalError.SetFormat(format);
	userRules.SetFormat(format);
	sePlugins.SetFormat(format);
	recognisedPlugins.SetFormat(format);
	unrecognisedPlugins.SetFormat(format);
}

void BossLog::Save(const fs::path file, const bool overwrite) {
	if (fs::exists(file))
		recognisedHasChanged = HasRecognisedListChanged(file);

	std::ofstream outFile;
	if (overwrite)
		// MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
		outFile.open(file.string());
	else
		// MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
		outFile.open(file.string(), std::ios_base::out|std::ios_base::app);  // MCP Note: May need std::ofstream:: here instead
	if (outFile.fail())
		throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());

	outFile << PrintLog();
	outFile.close();
}

void BossLog::Clear() {
	recognised = 0;
	unrecognised = 0;
	inactive = 0;
	messages = 0;
	warnings = 0;
	errors = 0;

	scriptExtender.clear();
	gameName.clear();

	updaterOutput.Clear();
	criticalError.Clear();
	userRules.Clear();
	sePlugins.Clear();
	recognisedPlugins.Clear();
	unrecognisedPlugins.Clear();

	parsingErrors.clear();
	globalMessages.clear();
}

bool BossLog::HasRecognisedListChanged(const fs::path file) {
	std::size_t pos1, pos2;
	std::string result;
	fileToBuffer(file, result);
	if (logFormat == HTML) {
		pos1 = result.find("<section id='recPlugins'>");
		if (pos1 != std::string::npos)
			pos1 = result.find("<ul>", pos1);
		if (pos1 != std::string::npos)
			pos2 = result.find("</section>", pos1);
		if (pos2 != std::string::npos)
			result = result.substr(pos1 + 4, pos2 - pos1 - 9);
	} else {
		pos1 = result.find(translate("Please read any attached messages and act on any that require action."));
		if (pos1 != std::string::npos)
			pos2 = result.find("======================================", pos1);
		if (pos2 != std::string::npos)
			result = result.substr(pos1 + 69, pos2 - pos1 - 69 - 3);
	}
	return (result == recognisedPlugins.AsString());
}

}  // namespace boss
