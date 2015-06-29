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

#include "Output/Output.h"
#include "Common/Error.h"
#include "Common/Globals.h"
#include "Support/Helpers.h"
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

namespace boss {
	using namespace std;
	using boost::algorithm::replace_all;
	using boost::locale::translate;

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

	Outputter::Outputter(const uint32_t format) : outFormat(format) {
		if (outFormat == HTML)
			escapeHTMLSpecialChars = true;
		else
			escapeHTMLSpecialChars = false;
	}

	Outputter::Outputter(const uint32_t format, const ParsingError e)
	    : outFormat(format) {
		if (outFormat == HTML)
			escapeHTMLSpecialChars = true;
		else
			escapeHTMLSpecialChars = false;

		*this << e;
	}

	Outputter::Outputter(const uint32_t format, const Rule r)
	    : outFormat(format) {
		if (outFormat == HTML)
			escapeHTMLSpecialChars = true;
		else
			escapeHTMLSpecialChars = false;

		*this << r;
	}

	Outputter::Outputter(const uint32_t format, const logFormatting l)
	    : outFormat(format) {
		if (outFormat == HTML)
			escapeHTMLSpecialChars = true;
		else
			escapeHTMLSpecialChars = false;

		*this << l;
	}

	void Outputter::SetFormat(const uint32_t format) {
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

	uint32_t Outputter::GetFormat() const {
		return outFormat;
	}

	bool Outputter::GetHTMLSpecialEscape() const {
		return escapeHTMLSpecialChars;
	}

	string Outputter::AsString() const {
		return outStream.str();
	}

	string Outputter::EscapeHTMLSpecial(string text) {
		if (escapeHTMLSpecialChars && outFormat == HTML) {
			replace_all(text, "&", "&amp;");
			replace_all(text, "\"", "&quot;");
			replace_all(text, "'", "&#039;");
			replace_all(text, "<", "&lt;");
			replace_all(text, ">", "&gt;");
			replace_all(text, "©", "&copy;");
			replace_all(text, "✗", "&#x2717;");
			replace_all(text, "✓", "&#x2713;");
			replace_all(text, "\n", "<br />");  // Not an HTML special char escape, but this needs to happen here to get the details of parser errors formatted correctly.
		}
		return text;
	}

	string Outputter::EscapeHTMLSpecial(char c) {
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
					return string(1, c);
			}
		} else
			return string(1, c);
	}

	Outputter& Outputter::operator= (const Outputter& o) {
		outStream << o.AsString();
		outFormat = o.GetFormat();
		escapeHTMLSpecialChars = o.GetHTMLSpecialEscape();
		return *this;
	}

	Outputter& Outputter::operator<< (const string s) {
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
					outStream << endl;
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
					outStream << endl;
				break;
			case TABLE_ROW_CLASS_SUCCESS:
				if (outFormat == HTML)
					outStream << "<tr class='success'>";
				else
					outStream << endl;
				break;
			case TABLE_ROW_CLASS_WARN:
				if (outFormat == HTML)
					outStream << "<tr class='warn'>";
				else
					outStream << endl;
				break;
			case TABLE_ROW_CLASS_ERROR:
				if (outFormat == HTML)
					outStream << "<tr class='error'>";
				else
					outStream << endl;
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
					outStream << endl << endl;
				break;
			case LIST_OPEN:
				if (outFormat == HTML)
					outStream << "<ul>";
				break;
			case LIST_CLOSE:
				if (outFormat == HTML)
					outStream << "</ul>";
				else
					outStream << endl;
				break;
			case HEADING_OPEN:
				if (outFormat == HTML)
					outStream << "<h2>";
				else
					outStream << endl << endl << "======================================" << endl;
				break;
			case HEADING_CLOSE:
				if (outFormat == HTML)
					outStream << "</h2>";
				else
					outStream << endl << "======================================" << endl << endl;
				break;
			case PARAGRAPH:
				if (outFormat == HTML)
					outStream << "<p>";
				else
					outStream << endl;
				break;
			case LIST_ITEM:
				if (outFormat == HTML)
					outStream << "<li>";
				else
					outStream << endl << endl;
				break;
			case LIST_ITEM_CLASS_SUCCESS:
				if (outFormat == HTML)
					outStream << "<li class='success'>";
				else
					outStream << endl << "*  ";
				break;
			case LIST_ITEM_CLASS_WARN:
				if (outFormat == HTML)
					outStream << "<li class='warn'>";
				else
					outStream << endl << "*  ";
				break;
			case LIST_ITEM_CLASS_ERROR:
				if (outFormat == HTML)
					outStream << "<li class='error'>";
				else
					outStream << endl << "*  ";
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
					outStream << endl << endl;
				break;
			case BLOCKQUOTE_CLOSE:
				if (outFormat == HTML)
					outStream << "</blockquote>";
				else
					outStream << endl << endl;
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

	Outputter& Outputter::operator<< (const int32_t i) {
		outStream << i;
		return *this;
	}

	Outputter& Outputter::operator<< (const uint32_t i) {
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
		string data = EscapeHTMLSpecial(m.Data());
		// Need to handle web addresses. Recognised are those in the following formats:
		// "http:someAddress", "http:someAddress label", "https:someAddress", "https:someAddress label", "file:somelocalAddress", "file:someLocalAddress label"

		size_t pos1, pos2, pos3;
		string link, label, dq;
		string addressTypes[] = {"http:", "https:", "file:"};

		if (outFormat == HTML)
			dq = "&quot;";
		else
			dq = "\"";

		// Do replacements for all addressTypes.
		for (uint32_t i = 0; i < 2; i++) {
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
				outStream << "<li class='note'>" << translate("Note") << ": " << data;
			else
				outStream << endl << "*  " << translate("Note") << ": " << data;
		} else if (m.Key() == TAG) {
			if (outFormat == HTML)
				outStream << "<li class='tag'><span class='tagPrefix'>" << translate("Bash Tag suggestion(s)") << ":</span> " << data;
			else
				outStream << endl << "*  " << translate("Bash Tag suggestion(s)") << ": " << data;
		} else if (m.Key() == REQ) {
			if (outFormat == HTML)
				outStream << "<li class='req'>" << translate("Requires") << ": " << data;
			else
				outStream << endl << "*  " << translate("Requires") << ": " << data;
		} else if (m.Key() == INC) {
			if (outFormat == HTML)
				outStream << "<li class='inc'>" << translate("Incompatible with") << ": " << data;
			else
				outStream << endl << "*  " << translate("Incompatible with") << ": " << data;
		} else if (m.Key() == WARN) {
			if (outFormat == HTML)
				outStream << "<li class='warn'>" << translate("Warning") << ": " << data;
			else
				outStream << endl << "*  " << translate("Warning") << ": " << data;
		} else if (m.Key() == ERR) {
			if (outFormat == HTML)
				outStream << "<li class='error'>" << translate("Error") << ": " << data;
			else
				outStream << endl << "*  " << translate("Error") << ": " << data;
		} else if (m.Key() == DIRTY) {
			if (outFormat == HTML)
				outStream << "<li class='dirty'>" << translate("Contains dirty edits") << ": " << data;
			else
				outStream << endl << "*  " << translate("Contains dirty edits") << ": " << data;
		} else {
			if (outFormat == HTML)
				outStream << "<li class='note'>" << translate("Note") << ": " << data;
			else
				outStream << endl << "*  " << translate("Note") << ": " << data;
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
		vector<RuleLine> lines = r.Lines();
		size_t linesSize = lines.size();
		string varOpen = Outputter(outFormat, VAR_OPEN).AsString();
		string varClose = Outputter(outFormat, VAR_CLOSE).AsString();

		// Need to temporarily turn off escaping of special characters so that <var> and </var> are printed correctly.
		bool wasEscaped = escapeHTMLSpecialChars;

		for (size_t j = 0; j < linesSize; j++) {
			string rObject = varOpen + EscapeHTMLSpecial(r.Object()) + varClose;
			string lObject = varOpen + EscapeHTMLSpecial(lines[j].Object()) + varClose;

			escapeHTMLSpecialChars = false;

			// TODO(MCP): Look at converting this to a switch-statement
			if (lines[j].Key() == BEFORE) {
				*this << (boost::format(translate("Sort %1% before %2%")) % rObject % lObject).str();
			} else if (lines[j].Key() == AFTER) {
				*this << (boost::format(translate("Sort %1% after %2%")) % rObject % lObject).str();
			} else if (lines[j].Key() == TOP) {
				*this << (boost::format(translate("Insert %1% at the top of %2%")) % rObject % lObject).str();
			} else if (lines[j].Key() == BOTTOM) {
				*this << (boost::format(translate("Insert %1% at the bottom of %2%")) % rObject % lObject).str();
			} else if (lines[j].Key() == APPEND) {
				if (!hasEditedMessages) {
					if (r.Key() == FOR)
						*this << (boost::format(translate("Add the following messages to %1%:")) % rObject).str() << LINE_BREAK << LIST_OPEN;
					else
						*this << LINE_BREAK << translate("Add the following messages:") << LINE_BREAK << LIST_OPEN;
				}
				*this << lines[j].ObjectAsMessage();
				hasEditedMessages = true;
			} else if (lines[j].Key() == REPLACE) {
				if (!hasEditedMessages) {
					if (r.Key() == FOR)
						*this << (boost::format(translate("Replace the messages attached to %1% with:")) % rObject).str() << LINE_BREAK << LIST_OPEN;
					else
						*this << LINE_BREAK << translate("Replace the attached messages with:") << LINE_BREAK << LIST_OPEN;
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

	BossLog::BossLog(const uint32_t format)
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

	string BossLog::PrintHeaderTop() {
		stringstream out;
		if (logFormat == HTML) {
			out << "<!DOCTYPE html>"
			    << "<meta charset='utf-8'>"
			    << "<title>BOSS Log</title>"
			    << "<link rel='stylesheet' href='../resources/style.css' />"
			    << "<nav>"
			    << "<header>"
			    << "	<h1>BOSS</h1>"
			    << translate("	Version ") << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH)
			    << "</header>";
		} else {
			out << "\nBOSS\n"
			    << translate("Version ") << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH) << endl;
		}
		return out.str();
	}

	string BossLog::PrintHeaderBottom() {
		stringstream out;
		if (logFormat == HTML) {
			out << "<footer>"
			    << "	<div class='button' data-section='browserBox' id='supportButtonShow'>" << translate("Log Feature Support") << "</div>"
			    << "	<div class='button' id='filtersButtonToggle'>" << translate("Filters") << "<span id='arrow'></span></div>"
			    << "</footer>"
			    << "</nav>"
			    << "<noscript>"
			    << translate("The BOSS Log requires Javascript to be enabled in order to function.")
			    << "</noscript>";
		}
		return out.str();
	}

	string BossLog::PrintFooter() {
		stringstream out;
		string colourTooltip = translate("Colours must be specified using lowercase hex codes.");

		if (logFormat == HTML) {
			out << "<section id='browserBox'>"
			    << "<p>" << translate("Support for the BOSS Log's more advanced features varies. Here's what your browser supports:")
			    << "<h3>" << translate("Functionality") << "</h3>"
			    << "<table>"
			    << "	<tbody>"
			    << "		<tr><td id='pluginSubmitSupport'><td>" << translate("In-Log Plugin Submission") << "<td>" << translate("Allows unrecognised plugins to be anonymously submitted to the BOSS team directly from the BOSS Log.")
			    << "		<tr><td id='memorySupport'><td>" << translate("Settings Memory") << "<td>" << translate("Allows the BOSS Log to automatically restore the filter configuration last used whenever the BOSS Log is opened.")
			    // MCP Note: Look at replacing the four spaces with a tab. It looks like a mistake but will need to verify first.
			    << "	    <tr><td id='placeholderSupport'><td>" << translate("Input Placeholders")
			    << "	    <tr><td id='validationSupport'><td>" << translate("Form Validation")
			    << "</table>"
			    << "</section>"

			    << "<aside>"
			    << "<label><input type='checkbox' id='hideVersionNumbers' data-class='version'/>" << translate("Hide Version Numbers") << "</label>"
			    << "<label><input type='checkbox' id='hideActiveLabel' data-class='active'/>" << translate("Hide 'Active' Label") << "</label>"
			    << "<label><input type='checkbox' id='hideChecksums' data-class='crc'/>" << translate("Hide Checksums") << "</label>"
			    << "<label><input type='checkbox' id='hideNotes'/>" << translate("Hide Notes") << "</label>"
			    << "<label><input type='checkbox' id='hideBashTags'/>" << translate("Hide Bash Tag Suggestions") << "</label>"
			    << "<label><input type='checkbox' id='hideRequirements'/>" << translate("Hide Requirements") << "</label>"
			    << "<label><input type='checkbox' id='hideIncompatibilities'/>" << translate("Hide Incompatibilities") << "</label>"
			    << "<label><input type='checkbox' id='hideDoNotCleanMessages'/>" << translate("Hide 'Do Not Clean' Messages") << "</label>"
			    << "<label><input type='checkbox' id='hideAllPluginMessages'/>" << translate("Hide All Plugin Messages") << "</label>"
			    << "<label><input type='checkbox' id='hideInactivePlugins'/>" << translate("Hide Inactive Plugins") << "</label>"
			    << "<label><input type='checkbox' id='hideMessagelessPlugins'/>" << translate("Hide Messageless Plugins") << "</label>"
			    << "<footer>"
			    << "	" << (boost::format(translate("%1% of %2% recognised plugins hidden.")) % "<span id='hiddenPluginNo'>0</span>" % recognised).str()
			    << "	" << (boost::format(translate("%1% of %2% messages hidden.")) % "<span id='hiddenMessageNo'>0</span>" % messages).str()
			    << "</footer>"
			    << "</aside>"

			    << "<div id='overlay'>"
			    << "<div id='submitBox'>"
			    << "<h2>" << translate("Submit Plugin") << "</h2>"
			    << "<p><span id='pluginLabel'>" << translate("Plugin") << ":</span><span id='plugin'></span>"
			    << "<form>"
			    << "<label>" << translate("Download Location") << ":<br /><input type='url' placeholder='" << translate("Label for text box. Do not use a single quote in translation, use '&#x27;' instead", "A link to the plugin&#x27;s download location.") << "' id='link'></label>"
			    << "<label>" << translate("Additional Notes") << ":<br /><textarea id='notes' placeholder='" << translate("Any additional information, such as recommended Bash Tags, load order suggestions, ITM/UDR counts and dirty CRCs, can be supplied here. If no download link is available, this information is crucial.") << "'></textarea></label>"
			    << "<div id='output'></div>"
			    << "<p class='last'><button>" << translate("Submit") << "</button>"
			    << "<button type='reset'>" << translate("Close") << "</button>"
			    << "</form>"
			    << "</div>"
			    << "</div>"

			    // Need to define some variables in code.
			    << "<script>"
			    << "var gameName = '" << gameName << "';"
			    << "var txt1 = '" << translate("Checking for existing submission...") << "';"
			    << "var txt2 = '" << translate("Matching submission already exists.") << "';"
			    << "var txt3 = '" << translate("Plugin already submitted. Submission updated with new comment.") << "';"
			    << "var txt4 = '" << translate("Plugin submitted!") << "';"
			    << "var txt5 = '" << translate("Plugin submission failed! Authorisation failure. Please report this to the BOSS team.") << "';"
			    << "var txt6 = '" << translate("Plugin submission failed! GitHub API rate limit exceeded. Please try again after %1%.") << "';"
			    << "var txt7 = '" << translate("Plugin submission failed!") << "';"
			    << "var txt8 = '" << translate("Web storage quota for this document has been exceeded.Please empty your browser\\'s cache. Note that this will delete all locally stored data.") << "';"
			    << "var txt9 = '" << translate("Please supply at least a link or some notes.") << "';"
			    << "var txt10 = '" << translate("Do not clean.") << "';"
			    << "</script>"
			    << "<script src='../resources/promise-1.0.0.min.js'></script>"
			    << "<script src='../resources/octokit.js'></script>"
			    << "<script src='../resources/script.js'></script>";
		}
		return out.str();
	}

	string BossLog::PrintLog() {
		stringstream out;
		Outputter formattedOut(logFormat);

		// Print header
		//-------------------------

		out << PrintHeaderTop();

		if (logFormat == HTML) {
			formattedOut << DIV_SUMMARY_BUTTON_OPEN << translate("Summary") << DIV_CLOSE;

			if (!userRules.Empty())
				formattedOut << DIV_USERLIST_BUTTON_OPEN << translate("User Rules") << DIV_CLOSE;

			if (!sePlugins.Empty())
				formattedOut << DIV_SE_BUTTON_OPEN << (boost::format(translate("%1% Plugins")) % scriptExtender).str() << DIV_CLOSE;

			if (!recognisedPlugins.Empty()) {
				if (gl_revert < 1)
					formattedOut << DIV_RECOGNISED_BUTTON_OPEN << translate("Recognised Plugins") << DIV_CLOSE;
				else
					formattedOut << DIV_RECOGNISED_BUTTON_OPEN << translate("Restored Load Order") << DIV_CLOSE;
			}

			if (!unrecognisedPlugins.Empty())
				formattedOut << DIV_UNRECOGNISED_BUTTON_OPEN << translate("Unrecognised Plugins") << DIV_CLOSE;

			out << formattedOut.AsString() << PrintHeaderBottom();
			formattedOut.Clear();  // Clear formattedOut for re-use.
		}


		// Print Summary
		//-------------------------

		formattedOut.SetHTMLSpecialEscape(false);

		if (logFormat == HTML)
			formattedOut << SECTION_ID_SUMMARY_OPEN;
		else
			formattedOut << SECTION_ID_SUMMARY_OPEN << HEADING_OPEN << translate("Summary") << HEADING_CLOSE;

		if (recognised != 0 || unrecognised != 0 || messages != 0) {
			formattedOut << TABLE_OPEN << TABLE_HEAD << TABLE_ROW << TABLE_HEADING << translate("Plugin Type") << TABLE_HEADING << translate("Count")
			             << TABLE_BODY << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << translate("Recognised (or sorted by user rules)") << TABLE_DATA << recognised;
			if (unrecognised != 0)
				formattedOut << TABLE_ROW_CLASS_WARN << TABLE_DATA << translate("Unrecognised") << TABLE_DATA << unrecognised;
			else
				formattedOut << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << translate("Unrecognised") << TABLE_DATA << unrecognised;
			formattedOut << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << translate("Inactive") << TABLE_DATA << inactive
			             << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << translate("All") << TABLE_DATA << (recognised + unrecognised)
			             << TABLE_CLOSE

			             << TABLE_OPEN << TABLE_HEAD << TABLE_ROW << TABLE_HEADING << translate("Plugin Message Type") << TABLE_HEADING << translate("Count")
			             << TABLE_BODY;
			if (warnings != 0)
				formattedOut << TABLE_ROW_CLASS_WARN << TABLE_DATA << translate("Warning") << TABLE_DATA << warnings;
			else
				formattedOut << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << translate("Warning") << TABLE_DATA << warnings;
			if (errors != 0)
				formattedOut << TABLE_ROW_CLASS_ERROR << TABLE_DATA << translate("Error") << TABLE_DATA << errors;
			else
				formattedOut << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << translate("Error") << TABLE_DATA << errors;
			formattedOut << TABLE_ROW_CLASS_SUCCESS << TABLE_DATA << translate("All") << TABLE_DATA << messages
			             << TABLE_CLOSE;
		}

		formattedOut << LIST_OPEN;

		formattedOut << updaterOutput.AsString();  // This contains BOSS & masterlist update strings.

		if (recognisedHasChanged)
			formattedOut << LIST_ITEM_CLASS_SUCCESS << translate("No change in recognised plugin list since last run.");

		size_t size = parsingErrors.size();  // First print parser/syntax error messages.
		for (size_t i = 0; i < size; i++)
			formattedOut << parsingErrors[i];

		formattedOut << criticalError.AsString();  // Print any critical errors.

		formattedOut.SetHTMLSpecialEscape(true);
		size = globalMessages.size();
		for (size_t i = 0; i < size; i++)
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
				formattedOut << SECTION_ID_USERLIST_OPEN << HEADING_OPEN << translate("User Rules") << HEADING_CLOSE;
			formattedOut << TABLE_OPEN << TABLE_HEAD << TABLE_ROW << TABLE_HEADING << translate("Rule") << TABLE_HEADING << translate("Applied") << TABLE_HEADING << translate("Details (if applicable)")
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
				formattedOut << SECTION_ID_SE_OPEN << HEADING_OPEN << scriptExtender << translate(" Plugins") << HEADING_CLOSE;
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
					formattedOut << SECTION_ID_RECOGNISED_OPEN << HEADING_OPEN << translate("Recognised Plugins") << HEADING_CLOSE;
				else if (gl_revert == 1)
					formattedOut << SECTION_ID_RECOGNISED_OPEN << HEADING_OPEN << translate("Restored Load Order (Using modlist.txt)") << HEADING_CLOSE;
				else if (gl_revert == 2)
					formattedOut << SECTION_ID_RECOGNISED_OPEN << HEADING_OPEN << translate("Restored Load Order (Using modlist.old)") << HEADING_CLOSE;
			}
			formattedOut << PARAGRAPH
			             << translate("These plugins are recognised by BOSS and have been sorted according to its masterlist. Please read any attached messages and act on any that require action.")
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
				formattedOut << SECTION_ID_UNRECOGNISED_OPEN << HEADING_OPEN << translate("Unrecognised Plugins") << HEADING_CLOSE;

			formattedOut << PARAGRAPH << translate("The following plugins were not found in the masterlist, and must be positioned manually, using your favourite mod manager or by using BOSS's user rules functionality.")
			             << SPAN_ID_UNRECPLUGINSSUBMITNOTE_OPEN << translate(" You can submit unrecognised plugins for addition to the masterlist directly from this log by clicking on a plugin and supplying a link and/or description of its contents in the panel that is displayed.") << SPAN_CLOSE << LIST_OPEN
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

	void BossLog::SetFormat(const uint32_t format) {
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

		ofstream outFile;
		if (overwrite)
			// MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
			outFile.open(file.string());
		else
			// MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
			outFile.open(file.string(), ios_base::out|ios_base::app);
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
		size_t pos1, pos2;
		string result;
		fileToBuffer(file, result);
		if (logFormat == HTML) {
			pos1 = result.find("<section id='recPlugins'>");
			if (pos1 != string::npos)
				pos1 = result.find("<ul>", pos1);
			if (pos1 != string::npos)
				pos2 = result.find("</section>", pos1);
			if (pos2 != string::npos)
				result = result.substr(pos1 + 4, pos2 - pos1 - 9);
		} else {
			pos1 = result.find(translate("Please read any attached messages and act on any that require action."));
			if (pos1 != string::npos)
				pos2 = result.find("======================================", pos1);
			if (pos2 != string::npos)
				result = result.substr(pos1 + 69, pos2 - pos1 - 69 - 3);
		}
		return (result == recognisedPlugins.AsString());
	}
}