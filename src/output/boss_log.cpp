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

#include "output/boss_log.h"

#include <cstddef>
#include <cstdint>

#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "common/error.h"
#include "common/globals.h"
#include "output/output.h"
#include "support/helpers.h"

namespace boss {

namespace fs = boost::filesystem;
namespace bloc = boost::locale;

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

	//ofstream outFile;
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

std::string BossLog::PrintLog() {
	std::stringstream out;
	Outputter formattedOut(logFormat);

	// Print header
	//-------------------------

	out << PrintHeaderTop();

	if (logFormat == HTML) {
		formattedOut << "	" << DIV_SUMMARY_BUTTON_OPEN << bloc::translate("Summary") << DIV_CLOSE << "\n";

		if (!userRules.Empty())
			formattedOut << "	" << DIV_USERLIST_BUTTON_OPEN << bloc::translate("User Rules") << DIV_CLOSE << "\n";

		if (!sePlugins.Empty())
			formattedOut << "	" << DIV_SE_BUTTON_OPEN << (boost::format(bloc::translate("%1% Plugins")) % scriptExtender).str() << DIV_CLOSE << "\n";

		if (!recognisedPlugins.Empty()) {
			if (gl_revert < 1)
				formattedOut << "	" << DIV_RECOGNISED_BUTTON_OPEN << bloc::translate("Recognised Plugins") << DIV_CLOSE << "\n";
			else
				formattedOut << "	" << DIV_RECOGNISED_BUTTON_OPEN << bloc::translate("Restored Load Order") << DIV_CLOSE << "\n";
		}

		if (!unrecognisedPlugins.Empty())
			formattedOut << "	" << DIV_UNRECOGNISED_BUTTON_OPEN << bloc::translate("Unrecognised Plugins") << DIV_CLOSE << "\n";

		out << formattedOut.AsString() << "\n" << PrintHeaderBottom();
		formattedOut.Clear();  // Clear formattedOut for re-use.
	}


	// Print Summary
	//-------------------------

	formattedOut.SetHTMLSpecialEscape(false);

	if (logFormat == HTML)
		formattedOut << SECTION_ID_SUMMARY_OPEN << "\n";
	else
		formattedOut << SECTION_ID_SUMMARY_OPEN << HEADING_OPEN << bloc::translate("Summary") << HEADING_CLOSE;

	if (recognised != 0 || unrecognised != 0 || messages != 0) {
		/*
		 * TODO(MCP)
		 *
		 * Need to replace the added tabs with the HT enum and
		 * the \n with the NEWLINE enum to keep extra characters from being written to the text-based log.
		 * Not exactly elegant so investigate other solutions later.
		 *
		 * It's pretty ugly so a better solution is probably desired sooner rather than later.
		 *
		 * May need to split things up into two different functions: one for the HTML log, one for the text-based log
		 */
		formattedOut << HT << TABLE_OPEN << NEWLINE
		             << HT << HT << TABLE_HEAD << NEWLINE
		             << HT << HT << HT << TABLE_ROW << NEWLINE
		             << HT << HT << HT << HT << TABLE_HEADING << bloc::translate("Plugin Type") << NEWLINE
		             << HT << HT << HT << HT << TABLE_HEADING << bloc::translate("Count") << NEWLINE
		             << HT << HT << HT << HT << TABLE_BODY << NEWLINE
		             << HT << HT << HT << TABLE_ROW_CLASS_SUCCESS << NEWLINE
		             << HT << HT << HT << HT << TABLE_DATA << bloc::translate("Recognised (or sorted by user rules)") << NEWLINE
		             << HT << HT << HT << HT << TABLE_DATA << recognised << NEWLINE;
		if (unrecognised != 0) {
			formattedOut << HT << HT << HT << TABLE_ROW_CLASS_WARN << NEWLINE
			             << HT << HT << HT << HT << TABLE_DATA << bloc::translate("Unrecognised") << NEWLINE
			             << HT << HT << HT << HT << TABLE_DATA << unrecognised << NEWLINE;
		} else {
			formattedOut << HT << HT << HT << TABLE_ROW_CLASS_SUCCESS << NEWLINE
			             << HT << HT << HT << HT << TABLE_DATA << bloc::translate("Unrecognised") << NEWLINE
			             << HT << HT << HT << HT << TABLE_DATA << unrecognised << NEWLINE;
		}
		formattedOut << HT << HT << HT << TABLE_ROW_CLASS_SUCCESS << NEWLINE
		             << HT << HT << HT << HT << TABLE_DATA << bloc::translate("Inactive") << NEWLINE
		             << HT << HT << HT << HT << TABLE_DATA << inactive << NEWLINE
		             << HT << HT << HT << TABLE_ROW_CLASS_SUCCESS << NEWLINE
		             << HT << HT << HT << HT << TABLE_DATA << bloc::translate("All") << NEWLINE
		             << HT << HT << HT << HT << TABLE_DATA << (recognised + unrecognised) << NEWLINE
		             << HT << TABLE_CLOSE << NEWLINE

		             << HT << TABLE_OPEN << NEWLINE
		             << HT << HT << TABLE_HEAD << NEWLINE
		             << HT << HT << HT << TABLE_ROW << NEWLINE
		             << HT << HT << HT << HT << TABLE_HEADING << bloc::translate("Plugin Message Type") << NEWLINE
		             << HT << HT << HT << HT << TABLE_HEADING << bloc::translate("Count") << NEWLINE
		             << HT << HT << TABLE_BODY << NEWLINE;
		if (warnings != 0) {
			formattedOut << HT << HT << HT << TABLE_ROW_CLASS_WARN << NEWLINE
			             << HT << HT << HT << HT << TABLE_DATA << bloc::translate("Warning") << NEWLINE
			             << HT << HT << HT << HT << TABLE_DATA << warnings << NEWLINE;
		} else {
			formattedOut << HT << HT << HT << TABLE_ROW_CLASS_SUCCESS << NEWLINE
			             << HT << HT << HT << HT << TABLE_DATA << bloc::translate("Warning") << NEWLINE
			             << HT << HT << HT << HT << TABLE_DATA << warnings << NEWLINE;
		}
		if (errors != 0) {
			formattedOut << HT << HT << HT << TABLE_ROW_CLASS_ERROR << NEWLINE
			             << HT << HT << HT << HT << TABLE_DATA << bloc::translate("Error") << NEWLINE
			             << HT << HT << HT << HT << TABLE_DATA << errors << NEWLINE;
		} else {
			formattedOut << HT << HT << HT << TABLE_ROW_CLASS_SUCCESS << NEWLINE
			             << HT << HT << HT << HT << TABLE_DATA << bloc::translate("Error") << NEWLINE
			             << HT << HT << HT << HT << TABLE_DATA << errors << NEWLINE;
		}
		formattedOut << HT << HT << HT << TABLE_ROW_CLASS_SUCCESS << NEWLINE
		             << HT << HT << HT << HT << TABLE_DATA << bloc::translate("All") << NEWLINE
		             << HT << HT << HT << HT << TABLE_DATA << messages << NEWLINE
		             << HT << TABLE_CLOSE << NEWLINE;
	}

	formattedOut << HT << LIST_OPEN << NEWLINE;

	formattedOut << updaterOutput.AsString();  // This contains BOSS & masterlist update strings.

	if (recognisedHasChanged)
		formattedOut << HT << HT << LIST_ITEM_CLASS_SUCCESS << bloc::translate("No change in recognised plugin list since last run.") << NEWLINE;

	std::size_t size = parsingErrors.size();  // First print parser/syntax error messages.
	for (std::size_t i = 0; i < size; i++)
		formattedOut << parsingErrors[i];

	formattedOut << criticalError.AsString();  // Print any critical errors.

	formattedOut.SetHTMLSpecialEscape(true);
	size = globalMessages.size();
	for (std::size_t i = 0; i < size; i++)
		formattedOut << globalMessages[i];  // Print global messages.
	formattedOut.SetHTMLSpecialEscape(false);

	formattedOut << HT << LIST_CLOSE << NEWLINE
	             << SECTION_CLOSE << NEWLINE;
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
			formattedOut << SECTION_ID_USERLIST_OPEN << "\n";
		else
			formattedOut << SECTION_ID_USERLIST_OPEN << HEADING_OPEN << bloc::translate("User Rules") << HEADING_CLOSE;
		formattedOut << HT << TABLE_OPEN << NEWLINE
		             << HT << HT << TABLE_HEAD << NEWLINE
		             << HT << HT << HT << TABLE_ROW << NEWLINE
		             << HT << HT << HT << HT << TABLE_HEADING << bloc::translate("Rule") << NEWLINE
		             << HT << HT << HT << HT << TABLE_HEADING << bloc::translate("Applied") << NEWLINE
		             << HT << HT << HT << HT << TABLE_HEADING << bloc::translate("Details (if applicable)") << NEWLINE
		             << HT << HT << TABLE_BODY << NEWLINE
		             << userRules.AsString()
		             << HT << TABLE_CLOSE << NEWLINE
		             << SECTION_CLOSE << NEWLINE;
		out << formattedOut.AsString();
		formattedOut.Clear();
	}


	// Print Script Extender Plugins
	//--------------------------------------

	if (!sePlugins.Empty()) {
		if (logFormat == HTML)
			formattedOut << SECTION_ID_SE_OPEN << "\n";
		else
			formattedOut << SECTION_ID_SE_OPEN << HEADING_OPEN << scriptExtender << bloc::translate(" Plugins") << HEADING_CLOSE;
		formattedOut << HT << LIST_OPEN << NEWLINE
		             << sePlugins.AsString()
		             << HT << LIST_CLOSE << NEWLINE
		             << SECTION_CLOSE << NEWLINE;
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
				formattedOut << SECTION_ID_RECOGNISED_OPEN << HEADING_OPEN << bloc::translate("Recognised Plugins") << HEADING_CLOSE;
			else if (gl_revert == 1)
				formattedOut << SECTION_ID_RECOGNISED_OPEN << HEADING_OPEN << bloc::translate("Restored Load Order (Using modlist.txt)") << HEADING_CLOSE;
			else if (gl_revert == 2)
				formattedOut << SECTION_ID_RECOGNISED_OPEN << HEADING_OPEN << bloc::translate("Restored Load Order (Using modlist.old)") << HEADING_CLOSE;
		}
		formattedOut << PARAGRAPH
		             << bloc::translate("These plugins are recognised by BOSS and have been sorted according to its masterlist. Please read any attached messages and act on any that require action.")
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
			formattedOut << SECTION_ID_UNRECOGNISED_OPEN << HEADING_OPEN << bloc::translate("Unrecognised Plugins") << HEADING_CLOSE;

		formattedOut << PARAGRAPH << bloc::translate("The following plugins were not found in the masterlist, and must be positioned manually, using your favourite mod manager or by using BOSS's user rules functionality.")
		             << SPAN_ID_UNRECPLUGINSSUBMITNOTE_OPEN << bloc::translate(" You can submit unrecognised plugins for addition to the masterlist directly from this log by clicking on a plugin and supplying a link and/or description of its contents in the panel that is displayed.") << SPAN_CLOSE << LIST_OPEN
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

std::string BossLog::PrintHeaderTop() {
	std::stringstream out;
	if (logFormat == HTML) {
		out << "<!DOCTYPE html>\n"
		    << "<meta charset='utf-8'>\n"
		    << "<title>BOSS Log</title>\n"
		    << "<link rel='stylesheet' href='../resources/style.css' />\n"
		    << "<nav>\n"
		    << "	<header>\n"
		    << "		<h1>BOSS</h1>\n"
		    << "		" << bloc::translate("Version ") << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH) << "\n"
		    << "	</header>\n";
	} else {
		out << "\nBOSS\n"
		    << bloc::translate("Version ") << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH) << std::endl;
	}
	return out.str();
}

std::string BossLog::PrintHeaderBottom() {
	std::stringstream out;
	if (logFormat == HTML) {
		out << "	<footer>\n"
		    << "		<div class='button' data-section='browserBox' id='supportButtonShow'>" << bloc::translate("Log Feature Support") << "</div>\n"
		    << "		<div class='button' id='filtersButtonToggle'>" << bloc::translate("Filters") << "<span id='arrow'></span></div>\n"
		    << "	</footer>\n"
		    << "</nav>\n"
		    << "<noscript>" << bloc::translate("The BOSS Log requires Javascript to be enabled in order to function.") << "</noscript>\n";
	}
	return out.str();
}

std::string BossLog::PrintFooter() {
	std::stringstream out;
	std::string colourTooltip = bloc::translate("Colours must be specified using lowercase hex codes.");

	if (logFormat == HTML) {
		out << "<section id='browserBox'>\n"
		    << "	<p>" << bloc::translate("Support for the BOSS Log's more advanced features varies. Here's what your browser supports:") << "\n"
		    << "	<h3>" << bloc::translate("Functionality") << "</h3>\n"
		    << "	<table>\n"
		    << "		<tbody>\n"
		    << "			<tr>\n"
		    << "				<td id='pluginSubmitSupport'>\n"
		    << "				<td>" << bloc::translate("In-Log Plugin Submission") << "\n"
		    << "				<td>" << bloc::translate("Allows unrecognised plugins to be anonymously submitted to the BOSS team directly from the BOSS Log.") << "\n"
		    << "			<tr>\n"
		    << "				<td id='memorySupport'>\n"
		    << "				<td>" << bloc::translate("Settings Memory") << "\n"
		    << "				<td>" << bloc::translate("Allows the BOSS Log to automatically restore the filter configuration last used whenever the BOSS Log is opened.") << "\n"
		    << "			<tr>\n"
		    << "				<td id='placeholderSupport'>\n"
		    << "				<td>" << bloc::translate("Input Placeholders") << "\n"
		    << "			<tr>\n"
		    << "				<td id='validationSupport'>\n"
		    << "				<td>" << bloc::translate("Form Validation") << "\n"
		    << "	</table>\n"
		    << "</section>\n"

		    << "<aside>\n"
		    << "	<label><input type='checkbox' id='hideVersionNumbers' data-class='version'/>" << bloc::translate("Hide Version Numbers") << "</label>\n"
		    << "	<label><input type='checkbox' id='hideActiveLabel' data-class='active'/>" << bloc::translate("Hide 'Active' Label") << "</label>\n"
		    << "	<label><input type='checkbox' id='hideChecksums' data-class='crc'/>" << bloc::translate("Hide Checksums") << "</label>\n"
		    << "	<label><input type='checkbox' id='hideNotes'/>" << bloc::translate("Hide Notes") << "</label>\n"
		    << "	<label><input type='checkbox' id='hideBashTags'/>" << bloc::translate("Hide Bash Tag Suggestions") << "</label>\n"
		    << "	<label><input type='checkbox' id='hideRequirements'/>" << bloc::translate("Hide Requirements") << "</label>\n"
		    << "	<label><input type='checkbox' id='hideIncompatibilities'/>" << bloc::translate("Hide Incompatibilities") << "</label>\n"
		    << "	<label><input type='checkbox' id='hideDoNotCleanMessages'/>" << bloc::translate("Hide 'Do Not Clean' Messages") << "</label>\n"
		    << "	<label><input type='checkbox' id='hideAllPluginMessages'/>" << bloc::translate("Hide All Plugin Messages") << "</label>\n"
		    << "	<label><input type='checkbox' id='hideInactivePlugins'/>" << bloc::translate("Hide Inactive Plugins") << "</label>\n"
		    << "	<label><input type='checkbox' id='hideMessagelessPlugins'/>" << bloc::translate("Hide Messageless Plugins") << "</label>\n"
		    << "	<footer>\n"
		    << "			" << (boost::format(bloc::translate("%1% of %2% recognised plugins hidden.")) % "<span id='hiddenPluginNo'>0</span>" % recognised).str() << "\n"
		    << "			" << (boost::format(bloc::translate("%1% of %2% messages hidden.")) % "<span id='hiddenMessageNo'>0</span>" % messages).str() << "\n"
		    << "	</footer>\n"
		    << "</aside>\n"

		    << "<div id='overlay'>\n"
		    << "	<div id='submitBox'>\n"
		    << "		<h2>" << bloc::translate("Submit Plugin") << "</h2>\n"
		    << "		<p><span id='pluginLabel'>" << bloc::translate("Plugin") << ":</span><span id='plugin'></span>\n"
		    << "		<form>\n"
		    << "			<label>" << bloc::translate("Download Location") << ":<br /><input type='url' placeholder='" << bloc::translate("Label for text box. Do not use a single quote in translation, use '&#x27;' instead", "A link to the plugin&#x27;s download location.") << "' id='link'></label>\n"
		    << "			<label>" << bloc::translate("Additional Notes") << ":<br /><textarea id='notes' placeholder='" << bloc::translate("Any additional information, such as recommended Bash Tags, load order suggestions, ITM/UDR counts and dirty CRCs, can be supplied here. If no download link is available, this information is crucial.") << "'></textarea></label>\n"
		    << "			<div id='output'></div>\n"
		    << "			<p class='last'><button>" << bloc::translate("Submit") << "</button>\n"
		    << "			<button type='reset'>" << bloc::translate("Close") << "</button>\n"
		    << "		</form>\n"
		    << "	</div>\n"
		    << "</div>\n"

		    // Need to define some variables in code.
		    << "<script>\n"
		    << "	var gameName = '" << gameName << "';\n"
		    << "	var txt1 = '" << bloc::translate("Checking for existing submission...") << "';\n"
		    << "	var txt2 = '" << bloc::translate("Matching submission already exists.") << "';\n"
		    << "	var txt3 = '" << bloc::translate("Plugin already submitted. Submission updated with new comment.") << "';\n"
		    << "	var txt4 = '" << bloc::translate("Plugin submitted!") << "';\n"
		    << "	var txt5 = '" << bloc::translate("Plugin submission failed! Authorisation failure. Please report this to the BOSS team.") << "';\n"
		    << "	var txt6 = '" << bloc::translate("Plugin submission failed! GitHub API rate limit exceeded. Please try again after %1%.") << "';\n"
		    << "	var txt7 = '" << bloc::translate("Plugin submission failed!") << "';\n"
		    << "	var txt8 = '" << bloc::translate("Web storage quota for this document has been exceeded.Please empty your browser\\'s cache. Note that this will delete all locally stored data.") << "';\n"
		    << "	var txt9 = '" << bloc::translate("Please supply at least a link or some notes.") << "';\n"
		    << "	var txt10 = '" << bloc::translate("Do not clean.") << "';\n"
		    << "</script>\n"
		    << "<script src='../resources/promise-1.0.0.min.js'></script>\n"
		    << "<script src='../resources/octokit.js'></script>\n"
		    << "<script src='../resources/script.js'></script>";
	}
	return out.str();
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
		pos1 = result.find(bloc::translate("Please read any attached messages and act on any that require action."));
		if (pos1 != std::string::npos)
			pos2 = result.find("======================================", pos1);
		if (pos2 != std::string::npos)
			result = result.substr(pos1 + 69, pos2 - pos1 - 69 - 3);
	}
	return (result == recognisedPlugins.AsString());
}

}  // namespace boss
