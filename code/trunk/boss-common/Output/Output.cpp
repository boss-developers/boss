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
#include "Support/Helpers.h"
#include <boost/algorithm/string.hpp>
#include "source/utf8.h"

namespace boss {
	using namespace std;
	using boost::algorithm::replace_all;

	Outputter::Outputter() {
		outFormat = PLAINTEXT;
		escapeHTMLSpecialChars = false;
	}

	Outputter::Outputter(uint32_t format) {
		outFormat = format;
		if (outFormat == HTML)
			escapeHTMLSpecialChars = true;
		else
			escapeHTMLSpecialChars = false;
	}

	void Outputter::SetFormat(uint32_t format) {
		outFormat = format;
		if (outFormat == HTML)
			escapeHTMLSpecialChars = true;
		else
			escapeHTMLSpecialChars = false;
	}

	void Outputter::SetHTMLSpecialEscape(bool shouldEscape) {
		escapeHTMLSpecialChars = shouldEscape;
	}

	void Outputter::Clear() {
		outStream.str(std::string());
	}

	void Outputter::PrintHeaderTop() {
		if (outFormat == HTML) {
			outStream << "<!DOCTYPE html>"<<endl<<"<meta charset='utf-8'>"<<endl
				<< "<title>BOSS Log</title>"<<endl<<"<style>"
				<< "body{font-family:Calibri,Arial,sans-serifs;margin:0;height:100%;}" << endl
				<< "body.dark, #submitBox.dark{color:#eee;background:#222;}" << endl
				<< "a.dark:link{color:#0AF;}" << endl
				<< "a.dark:visited{color:#E000E0;}" << endl
				<< "h1{font-size:3em;margin:0;}" << endl
				<< "h2{font-size:2em;padding-top:0.24em;margin-top:0;}" << endl
				<< "ul{list-style:none;padding-left:0;}" << endl
				<< "ul li{margin-left:0;margin-bottom:1em;}" << endl
				<< "li ul{margin-top:.5em;padding-left:2.5em;margin-bottom:2em;}" << endl
				<< "table {margin-left:2em;}" << endl
				<< "thead {font-weight:bold;}" << endl
				<< "td{padding:0 .5em;vertical-align:top;}" << endl
				<< "blockquote{font-style:italic;}" << endl
				<< "input[type='checkbox']{position:relative;top:.15em;margin-right:.5em;}" << endl
				<< "noscript p {position:absolute;top:0;left:154px;padding-right:0.5em;padding-left:1em;}" << endl
				<< "small {padding-top:1em;font-size:0.5em;}" << endl
				<< ".error{background:red;color:#FFF;display:table;padding:0 4px;}" << endl
				<< ".warn{background:orange;color:#FFF;display:table;padding:0 4px;}" << endl
				<< ".success{color:green;}" << endl
				<< ".version{color:#6394F8;margin-right:1em;}" << endl
				<< ".crc{color:#BC8923;margin-right:1em;}" << endl
				<< ".active{color:#0A0;margin-right:1em;}" << endl
				<< ".tagPrefix{color:#CD5555;}" << endl
				<< ".dirty{color:#960;}" << endl
				<< ".message{color:gray;}" << endl
				<< ".mod{margin-right:1em;}" << endl
				<< ".tag{}" << endl
				<< ".note{}" << endl
				<< ".req{}" << endl
				<< ".inc{}" << endl
				<< ".submit{margin-right:1em;}" << endl
				<< ".hidden {display:none;}" << endl
				<< ".button {cursor:pointer;text-decoration:underline;}" << endl
				<< "p.last {text-align:center;margin-bottom:0;}" << endl
				<< ".t {color:green;margin-right:0.5em;}" << endl
				<< ".c {color:red;margin-right:0.5em;}" << endl
				<< ".t:after {content: '\\2713';}" << endl
				<< ".c:after {content: '\\2717';}" << endl
				<< ".dragHover {background:#DDD;}" << endl
				<< "body.dark section.dragHover {background:#444;}" << endl
				<< "#overlay{position:fixed;left:0px;top:0px;width:100%;height:100%;background:rgba(0,0,0,0.9);z-index:3;visibility:hidden;opacity:0;" << endl
				<< "	transition: opacity 0.3s;" << endl
				<< "	-webkit-transition: opacity 0.3s;" << endl
				<< "	-moz-transition: opacity 0.3s;" << endl
				<< "	-ms-transition: opacity 0.3s;" << endl
				<< "	-o-transition: opacity 0.3s;" << endl
				<< "}" << endl
				<< "#overlay.visible {opacity:1;visibility:visible;}" << endl
				<< "#output.visible, section.visible {display:block;}" << endl
				<< "nav.dark {background:#333;}" << endl
				<< "aside.dark {background:#666;}" << endl
				<< "#submitBox{padding:10px;background:white;z-index:2;position:fixed;top:0;width:430px;left:50%;margin-left:-220px;}" << endl
				<< "#submitBox > h2{text-align:center;margin:0;}" << endl
				<< "#submitBox p {margin-left:10px; margin-right:10px;}" << endl
				<< "#submitBox label {margin:16px 10px;display:block;}" << endl
				<< "#link{width:400px;}" << endl
				<< "#notes{height:10em;width:400px;}" << endl
				<< "#plugin{display:table-cell;font-style:italic;}" << endl
				<< "#pluginLabel{display:table-cell;padding-right:0.5em;}" << endl
				<< "#output {display:none;width:400px;margin:2em auto;text-align:center;}" << endl
				<< "nav {display:block;background:#F5F5F5;width:154px;position:fixed;top:0;left:0;height:100%;z-index:2;border-right:1px solid #CCC;box-shadow: 0 0 3px 1px rgba(0,0,0,0.5);}" << endl
				<< "nav div.button {cursor:pointer;padding:0.5em;text-decoration:none;border-top:1px solid transparent;border-bottom:1px solid transparent;" << endl
				<< "	transition: background 0.2s;" << endl
				<< "	-webkit-transition: background 0.2s;" << endl
				<< "	-moz-transition: background 0.2s;" << endl
				<< "	-ms-transition: background 0.2s;" << endl
				<< "	-o-transition: background 0.2s;" << endl
				<< "}" << endl
				<< "nav div.button:hover {background:lightgrey;}" << endl
				<< "nav div.button.dark:hover {background:darkgrey;}" << endl
				<< "nav div.button.current {background:lightgrey;border-color: #AAA;}" << endl
				<< "nav div.button.current.dark {background:darkgrey;border-color: #AAA;}" << endl
				<< "nav > footer {display:block;position:fixed;bottom:0;width:154px;background:inherit;}" << endl
				<< "nav > header {display:block;padding:0.5em;padding-top:0;margin-bottom:1em;}" << endl
				<< "#arrow {margin-left:5em;" << endl
				<< "/*	transition: transform 0.2s;" << endl
				<< "	-moz-transition: -moz-transform 0.2s;	" << endl
				<< "	-webkit-transition: -webkit-transform 0.2s;	" << endl
				<< "	-ms-transition: -ms-transform 0.2s;	" << endl
				<< "	-o-transition: -o-transform 0.2s;*/" << endl
				<< "	color:#757575;" << endl
				<< "}/*" << endl
				<< "#arrow.rotated {" << endl
				<< "	transform: rotate(180deg);	" << endl
				<< "	-moz-transform: rotate(180deg);	" << endl
				<< "	-webkit-transform: rotate(90deg);	" << endl
				<< "	-ms-transform: rotate(90deg);	" << endl
				<< "	-o-transform: rotate(90deg);" << endl
				<< "}*/" << endl
				<< "#arrow:after {" << endl
				<< "	content: '\\25b2';" << endl
				<< "}" << endl
				<< "#arrow.rotated:after {" << endl
				<< "	content: '\\25bc';" << endl
				<< "}" << endl
				<< "/*#arrow:after {" << endl
				<< "	content: '\\2C5';" << endl
				<< "}" << endl
				<< "#arrow.rotated:after {" << endl
				<< "	content: '\\2C4';" << endl
				<< "}*/" << endl
				<< "aside {display:block;background:#DDD;z-index:1;overflow:hidden;border-top:1px solid #AAA;position:fixed;left:154px;bottom:-1px;height:0;" << endl
				<< "/*	transition: height 0.2s;" << endl
				<< "	-moz-transition: height 0.2s;" << endl
				<< "	-webkit-transition: height 0.2s;" << endl
				<< "	-ms-transition: height 0.2s;" << endl
				<< "	-o-transition: height 0.2s;*/" << endl
				<< "	box-shadow: 0 0 3px 1px rgba(0,0,0,0.5);" << endl
				<< "	" << endl
				<< "}" << endl
				<< "aside.visible {height:auto;}" << endl
				<< "aside > label {display:inline-block;padding:0.2em 0.5em; white-space:nowrap;width:12.5em;cursor:pointer;}" << endl
				<< "aside > footer {display:block;padding:0.5em;padding-left:0.8em;font-style:italic;}" << endl
				<< "section {position:absolute;top:0;left:154px;display:none;padding-right:0.5em;padding-left:1em;height:100%;}" << endl
				<< "#unrecPlugins span.mod {cursor:pointer;border-bottom:#888 dotted 1px;}" << endl
				<< "#unrecPlugins span.mod:hover {border-bottom:#888 solid 1px;}" << endl
				<< "#unrecPlugins span.mod.nosubmit {border-bottom:none;cursor:auto;}" << endl
				<< "</style>" << endl
				<< "<!--[if lt IE 9]>" << endl
				<< "<script>" << endl
				<< "document.createElement('nav');document.createElement('header');document.createElement('footer');document.createElement('section');document.createElement('aside');" << endl
				<< "</script>" << endl
				<< "<![endif]-->" << endl;
			outStream << "<nav>" << endl
				<< "<header>" << endl
				<< "	<h1>BOSS</h1>" << endl
				<< "	Version " << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH) << "<br />" << endl
				<< "	" << gl_boss_release_date << "<br />" << endl
				<< "<small>&copy; 2009-2012 BOSS Development Team</small>" << endl
				<< "</header>" << endl;
		} else
			outStream << endl << "BOSS Log" << endl
				<< "Copyright 2009-2012 BOSS Development Team" << endl
				<< "License: GNU General Public License v3.0" << endl
				<< "(http://www.gnu.org/licenses/gpl.html)" << endl
				<< "v" << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH) << " (" << gl_boss_release_date << ")" << endl;
	}

	void Outputter::PrintHeaderBottom() {
		if (outFormat == HTML) {
			outStream << "<footer>" << endl
				<< "	<div class='button' data-section='cssSettings' id='cssButtonShow'>CSS Settings</div>" << endl
				<< "	<div class='button' id='filtersButtonToggle'>Filters<span id='arrow'></span></div>" << endl
				<< "</footer>" << endl
				<< "</nav>" << endl
				<< "<noscript>" << endl
				<< "<p>The BOSS Log requires Javascript to be enabled in order to function.</p>" << endl
				<< "</noscript>" << endl;
		}
	}

	void Outputter::PrintFooter(const uint32_t pluginNo, const uint32_t messageNo) {
		if (outFormat == HTML) {
			outStream << "<section id='cssSettings'>" << endl
				<< "<h2>CSS Settings</h2>" << endl
				<< "<p>Here you can customise the colours used by the alternative colour scheme.<span id='colorPickerNote'> Colours must be specified using their lowercase hex codes.</span> Boxes left blank will use their default values, which are given by their placeholders." << endl
				<< "<form>" << endl
				<< "<table>" << endl
				<< "	<thead><tr><td>Element<td>Colour" << endl
				<< "	<tbody>" << endl
				<< "		<tr><td>General Text<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='body.dark' data-property='color' placeholder='#eeeeee'>" << endl
				<< "		<tr><td>Window Backgrounds<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='body.dark,#submitBox.dark' data-property='background' placeholder='#222222'>" << endl
				<< "		<tr><td>Menu Background<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='nav.dark' data-property='background' placeholder='#333333'>" << endl
				<< "		<tr><td>Menu Button Hover<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='nav div.button.dark:hover' data-property='background' placeholder='#a9a9a9'>" << endl
				<< "		<tr><td>Active Menu Button<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='nav div.button.current.dark' data-property='background' placeholder='#a9a9a9'>" << endl
				<< "		<tr><td>Links<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='a.dark:link' data-property='color' placeholder='#00aaff'>" << endl
				<< "		<tr><td>Visited Links<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='a.dark:visited' data-property='color' placeholder='#e000e0'>" << endl
				<< "		<tr><td>CRC Labels<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.crc.dark' data-property='color' placeholder='#bc8923'>" << endl
				<< "		<tr><td>Active Labels<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.active.dark' data-property='color' placeholder='#00aa00'>" << endl
				<< "		<tr><td>Version Labels<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.version.dark' data-property='color' placeholder='#6394F8'>" << endl
				<< "		<tr><td>Errors<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.error.dark' data-property='background' placeholder='#ff0000'>" << endl
				<< "		<tr><td>Warnings<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.warn.dark' data-property='background' placeholder='#ffa500'>" << endl
				<< "		<tr><td>Dirty Messages<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.dirty.dark' data-property='color' placeholder='#996600'>" << endl
				<< "		<tr><td>Bash Tag Suggestion Prefixes<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.tagPrefix.dark' data-property='color' placeholder='#cd5555'>" << endl
				<< "		<tr><td>CSS Settings Panel File Drag Highlight<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.dark.dragHover' data-property='background' placeholder='#444444'>" << endl
				<< "		<tr><td>Filters Background<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='aside.dark' data-property='background' placeholder='#666666'>" << endl
				<< "		<tr><td>Success Messages<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.success.dark' data-property='color' placeholder='#008000'>" << endl
				<< "		<tr><td>Quoted Userlist Messages<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.message.dark' data-property='color' placeholder='#808080'>" << endl
				<< "</table>" << endl
				<< "<p id='backupCSSNote'>You can <span class='button' id='cssButtonBackup'>back up</span> your CSS settings to avoid losing them when you <span id='loseSettingsClose'>close the BOSS Log</span><span id='loseSettingsCacheClear'>clear your browser's cache</span>. Backup files are given nonsensical names by your browser, but you can rename them to whatever you want. Drag and drop a backup file into this panel to restore the settings it contains." << endl
				<< "<p><label><input type='checkbox' id='useDarkColourScheme'/>Use Alternative Colour Scheme</label>" << endl
				<< "<p><button>Apply</button>" << endl
				<< "</form>" << endl
				<< "</section>" << endl
				<< "" << endl
				<< "<section id='browserBox'>" << endl
				<< "<h2>Browser Compatibility</h2>" << endl
				<< "<p>The BOSS Log's more advanced features require some of the latest web technologies, for which browser support varies. Here's what your browser supports:" << endl
				<< "<h3>Functionality</h3>" << endl
				<< "<table>" << endl
				<< "	<tbody>" << endl
				<< "		<tr><td id='pluginSubmitSupport'><td>In-Log Plugin Submission<td>Allows unrecognised plugins to be submitted straight from the BOSS Log." << endl
				<< "		<tr><td id='cssBackupSupport'><td>Backup/Restore of CSS Settings<td>Allows backup and restore of your custom CSS settings." << endl
				<< "		<tr><td id='memorySupport'><td>Settings Memory<td>Allows the BOSS Log to restore the configuration of CSS settings and filters last used, and to prevent this panel being displayed, whenever the BOSS Log is opened." << endl
				<< "</table>" << endl
				<< "<h3>Appearance</h3>" << endl
				<< "<table>" << endl
				<< "	<tbody>" << endl
				<< "	<tr><td id='opacitySupport'><td>Opacity" << endl
				<< "	<tr><td id='shadowsSupport'><td>Shadows" << endl
				<< "	<tr><td id='transitionsSupport'><td>Transitions" << endl
				<< "	<tr><td id='transformsSupport'><td>Transforms" << endl
				<< "	<tr><td id='colorSupport'><td>Colour Picker Input" << endl
				<< "	<tr><td id='placeholderSupport'><td>Input Placeholders" << endl
				<< "	<tr><td id='validationSupport'><td>Form Validation" << endl
				<< "</table>" << endl
				<< "<p><label><input type=checkbox id='suppressBrowserBox'>Do not display this panel again for this browser. If this checkbox is left unchecked, this panel will be displayed every time you open the BOSS Log.</label>" << endl
				<< "</section>" << endl
				
				
				<< "<aside>" << endl
				<< "<label><input type='checkbox' id='hideVersionNumbers' data-class='version'/>Hide Version Numbers</label>" << endl
				<< "<label><input type='checkbox' id='hideActiveLabel' data-class='active'/>Hide 'Active' Label</label>" << endl
				<< "<label><input type='checkbox' id='hideChecksums' data-class='crc'/>Hide Checksums</label>" << endl
				<< "<label><input type='checkbox' id='hideNotes'/>Hide Notes</label>" << endl
				<< "<label><input type='checkbox' id='hideBashTags'/>Hide Bash Tag Suggestions</label>" << endl
				<< "<label><input type='checkbox' id='hideRequirements'/>Hide Requirements</label>" << endl
				<< "<label><input type='checkbox' id='hideIncompatibilities'/>Hide Incompatibilities</label>" << endl
				<< "<label><input type='checkbox' id='hideDoNotCleanMessages'/>Hide 'Do Not Clean' Messages</label>" << endl
				<< "<label><input type='checkbox' id='hideAllPluginMessages'/>Hide All Plugin Messages</label>" << endl
				<< "<label><input type='checkbox' id='hideInactivePlugins'/>Hide Inactive Plugins</label>" << endl
				<< "<label><input type='checkbox' id='hideMessagelessPlugins'/>Hide Messageless Plugins</label>" << endl
				<< "<label><input type='checkbox' id='hideCleanPlugins'/>Hide Clean Plugins</label>" << endl
				<< "<footer>" << endl
				<< "	<span id='hiddenPluginNo'>0</span> of " << pluginNo << " recognised plugins hidden." << endl
				<< "	<span id='hiddenMessageNo'>0</span> of " << messageNo << " messages hidden." << endl
				<< "</footer>" << endl
				<< "</aside>" << endl

				<< "<div id='overlay'>" << endl
				<< "<div id='submitBox'>" << endl
				<< "<h2>Submit Plugin</h2>" << endl
				<< "<p><span id='pluginLabel'>Plugin:</span><span id='plugin'></span>" << endl
				<< "<form>" << endl
				<< "<label>Download Location:<br /><input type='url' placeholder='A link to the plugin&#x27;s download location.' id='link'></label>" << endl
				<< "<label>Additional Notes:<br /><textarea id='notes' placeholder='Any additional information, such as recommended Bash Tags, load order suggestions, ITM/UDR counts and dirty CRCs, can be supplied here. If no download link is available, this information is crucial.'></textarea></label>" << endl
				<< "<div id='output'></div>" << endl
				<< "<p class='last'><button>Submit</button>" << endl
				<< "<button type='reset'>Close</button>" << endl
				<< "</form>" << endl
				<< "</div>" << endl
				<< "</div>" << endl

				<< "<script>" << endl
				<< "'use strict';" << endl
				<< "var url = 'http://www.darkcreations.org/bugzilla/jsonrpc.cgi';" << endl
				<< "var suppressBrowserBox = false;" << endl
				<< "function getBugId(plugin, desc, xhr) {" << endl
				<< "	var request = {" << endl
				<< "		\"method\":\"Bug.search\"," << endl
				<< "		\"params\":[{" << endl
				<< "			\"Bugzilla_login\":\"bossguest@darkcreations.org\"," << endl
				<< "			\"Bugzilla_password\":\"bosspassword\"," << endl
				<< "			\"product\":\"BOSS\"," << endl
				<< "			\"component\":\"TES IV: Oblivion\"," << endl
				<< "			\"summary\":plugin" << endl
				<< "		}]," << endl
				<< "		\"id\":1" << endl
				<< "	};" << endl
				<< "	outputPluginSubmitText('Checking for existing submission...', 0);" << endl
				<< "	xhr.open('POST', url, true);" << endl
				<< "	xhr.setRequestHeader('Content-Type', 'application/json');" << endl
				<< "	xhr.onerror = pluginSubmitError;" << endl
				<< "	xhr.onload = function() {" << endl
				<< "		if (xhr.status == 200 && isResponseOK(xhr.responseText)) {" << endl
				<< "			var response = JSON.parse(xhr.responseText);" << endl
				<< "			if (response.result.bugs.length > 0) {" << endl
				<< "				var id = response.result.bugs[0].id;" << endl
				<< "				addBugComment(id, desc, xhr);" << endl
				<< "			} else {" << endl
				<< "				addBug(plugin, desc, xhr);" << endl
				<< "			}" << endl
				<< "		} else {" << endl
				<< "			outputPluginSubmitText('Error: Existing submission check failed!', -1);" << endl
				<< "		}" << endl
				<< "	};" << endl
				<< "	xhr.send(JSON.stringify(request));" << endl
				<< "}" << endl
				<< "function addBugComment(id, comment, xhr) {" << endl
				<< "	var request = {" << endl
				<< "		\"method\":\"Bug.add_comment\"," << endl
				<< "		\"params\":[{" << endl
				<< "			\"Bugzilla_login\":\"bossguest@darkcreations.org\"," << endl
				<< "			\"Bugzilla_password\":\"bosspassword\"," << endl
				<< "			\"id\":id," << endl
				<< "			\"comment\":comment" << endl
				<< "		}]," << endl
				<< "		\"id\":2" << endl
				<< "	};" << endl
				<< "	outputPluginSubmitText('Previous submission found, updating with supplied details...', 0);" << endl
				<< "	xhr.open('POST', url, true);" << endl
				<< "	xhr.setRequestHeader('Content-type', 'application/json');" << endl
				<< "	xhr.onerror = pluginSubmitError;" << endl
				<< "	xhr.onload = function() {" << endl
				<< "		if (xhr.status == 200 && isResponseOK(xhr.responseText)) {" << endl
				<< "			outputPluginSubmitText('Plugin submission updated!', 1);" << endl
				<< "		} else {" << endl
				<< "			outputPluginSubmitText('Error: Plugin submission could not be updated.', -1);" << endl
				<< "		}" << endl
				<< "	};" << endl
				<< "	xhr.send(JSON.stringify(request));" << endl
				<< "}" << endl
				<< "function addBug(summary, description, xhr) {" << endl
				<< "	var request = {" << endl
				<< "		\"method\":\"Bug.create\"," << endl
				<< "		\"params\":[{" << endl
				<< "			\"Bugzilla_login\":\"bossguest@darkcreations.org\"," << endl
				<< "			\"Bugzilla_password\":\"bosspassword\"," << endl
				<< "			\"product\":\"BOSS\"," << endl
				<< "			\"component\":\"" << GetGameString(gl_current_game) << "\"," << endl
				<< "			\"summary\":summary," << endl
				<< "			\"version\":\"2.1\"," << endl
				<< "			\"description\":description," << endl
				<< "			\"op_sys\":\"Windows\"," << endl
				<< "			\"platform\":\"PC\"," << endl
				<< "			\"priority\":\"---\"," << endl
				<< "			\"severity\":\"enhancement\"" << endl
				<< "		}]," << endl
				<< "		\"id\":3" << endl
				<< "	};" << endl
				<< "	outputPluginSubmitText('No previous submission found, creating new submission...', 0);" << endl
				<< "	xhr.open('POST', url, true);" << endl
				<< "	xhr.setRequestHeader('Content-type', 'application/json');" << endl
				<< "	xhr.onerror = pluginSubmitError;" << endl
				<< "	xhr.onload = function() {" << endl
				<< "		if (xhr.status == 200 && isResponseOK(xhr.responseText)) {" << endl
				<< "			outputPluginSubmitText('Plugin submitted!', 1);" << endl
				<< "		} else {" << endl
				<< "			outputPluginSubmitText('Error: Plugin could not be submitted.', -1);" << endl
				<< "		}" << endl
				<< "	};" << endl
				<< "	xhr.send(JSON.stringify(request));" << endl
				<< "}" << endl
				<< "function pluginSubmitError() {" << endl
				<< "	outputPluginSubmitText('Error: Data transfer failed.', -1);" << endl
				<< "}" << endl
				<< "function isResponseOK(text) {" << endl
				<< "	return (JSON.parse(text).error == null);" << endl
				<< "}" << endl
				<< "function isStorageSupported(){" << endl
				<< "	try {" << endl
				<< "		return ('localStorage' in window && window['localStorage'] !== null && window['localStorage'] !== undefined);" << endl
				<< "	} catch (e) {" << endl
				<< "		return false;" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function isCSSBackupRestoreSupported(){" << endl
				<< "	return (window.File && window.FileReader && window.FileList && window.Blob && 'draggable' in document.createElement('span') && typeof(JSON) === 'object' && typeof(JSON.parse) === 'function');" << endl
				<< "}" << endl
				<< "function isPluginSubmitSupported(){" << endl
				<< "	return ('withCredentials' in new XMLHttpRequest && typeof(JSON) === 'object' && typeof(JSON.parse) === 'function');" << endl
				<< "}" << endl
				<< "function isStyleSupported(propName) {" << endl
				<< "	return typeof document.body.style[propName] !== 'undefined';" << endl
				<< "}" << endl
				<< "function isValidationSupported(){" << endl
				<< "	return ('checkValidity' in document.createElement('form'));" << endl
				<< "}" << endl
				<< "function storeData(key, value){" << endl
				<< "	try {" << endl
				<< "		localStorage.setItem(key, value);" << endl
				<< "	} catch (e) {" << endl
				<< "		if (e == QUOTA_EXCEEDED_ERR) {" << endl
				<< "			alert('Web storage quota for this document has been exceeded. Please empty your browser\\'s cache. Note that this will delete all locally stored data.');" << endl
				<< "		}" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "/*Wrapper for native getElementsByClassName, or Dustin Diaz's getElementsByClass if no native function available.*/" << endl
				<< "function getElementsByClassName(parent, className, tagName){" << endl
				<< "	if (document.getElementsByClassName){" << endl
				<< "		return parent.getElementsByClassName(className);" << endl
				<< "	}else{" << endl
				<< "		var classElements = new Array();" << endl
				<< "		if ( parent == null )" << endl
				<< "		parent = document;" << endl
				<< "		if ( tagName == null )" << endl
				<< "		tagName = '*';" << endl
				<< "		var els = parent.getElementsByTagName(tagName);" << endl
				<< "		var elsLen = els.length;" << endl
				<< "		var pattern = new RegExp(\"(^|\\\\s)\"+className+\"(\\\\s|$)\");" << endl
				<< "		for (i = 0, j = 0; i < elsLen; i++) {" << endl
				<< "			if ( pattern.test(els[i].className) ) {" << endl
				<< "				classElements[j] = els[i];" << endl
				<< "				j++;" << endl
				<< "			}" << endl
				<< "		}" << endl
				<< "		return classElements;" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function insertRule(styleSheet, selector, css){" << endl
				<< "	if (styleSheet.cssRules) {" << endl
				<< "		styleSheet.insertRule(selector + '{' + css + '}', styleSheet.cssRules.length);" << endl
				<< "	} else if (styleSheet.rules) {" << endl
				<< "		styleSheet.addRule(selector, css, -1);" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function addEventListener(element, event, fn) {" << endl
				<< "	if(document.addEventListener){" << endl
				<< "		element.addEventListener(event, fn, false);" << endl
				<< "	} else {" << endl
				<< "		element.attachEvent('on' + event, fn);" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function dispatchEvent(target, type) {" << endl
				<< "	var event;" << endl
				<< "	if (document.createEvent) {" << endl
				<< "		event = document.createEvent('Event');" << endl
				<< "		event.initEvent(type, true, true);" << endl
				<< "	} else {" << endl
				<< "		event = document.createEventObject();" << endl
				<< "		event.eventType = 'on' + type;" << endl
				<< "	}" << endl
				<< "	if (document.dispatchEvent) {" << endl
				<< "		target.dispatchEvent(event);" << endl
				<< "	} else {" << endl
				<< "		target.fireEvent(event.eventType, event);" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function showElement(element){" << endl
				<< "	if (element != null){" << endl
				<< "		if (element.className.indexOf('hidden') != -1) {" << endl
				<< "			element.className = element.className.replace('hidden','');" << endl
				<< "		} else if (element.className.indexOf('visible') == -1) {" << endl
				<< "			element.className += ' visible';" << endl
				<< "		}" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function hideElement(element){" << endl
				<< "	if (element != null) {" << endl
				<< "		if (element.className.indexOf('visible') != -1) {" << endl
				<< "			element.className = element.className.replace('visible','');" << endl
				<< "		}else if (element.className.indexOf('hidden') == -1) {" << endl
				<< "			element.className += ' hidden';" << endl
				<< "		}" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function stepUnhideElement(element){" << endl
				<< "	if (element != null && element.className.indexOf('hidden') != -1){" << endl
				<< "		element.className = element.className.replace('hidden','');" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function stepHideElement(element){" << endl
				<< "	if (element != null) {" << endl
				<< "		element.className += ' hidden';" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function saveCheckboxState(evt) {" << endl
				<< "	if (evt.currentTarget.checked) {" << endl
				<< "		storeData(evt.currentTarget.id, true);" << endl
				<< "	} else {" << endl
				<< "		localStorage.removeItem(evt.currentTarget.id);" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function handleFileSelect(evt) {" << endl
				<< "	evt.stopPropagation();" << endl
				<< "	evt.preventDefault();" << endl
				<< "	evt.currentTarget.className = evt.currentTarget.className.replace('dragHover',''); " << endl
				<< "	var files = evt.dataTransfer.files;" << endl
				<< "	var a = document.getElementById('cssSettings').querySelectorAll('input[type=text], input[type=color]');" << endl
				<< "	for (var i = 0, f; f = files[i]; i++) {" << endl
				<< "		var reader = new FileReader();" << endl
				<< "		reader.onload = (function(theFile) {" << endl
				<< "			return function(e) {" << endl
				<< "				try {" << endl
				<< "					var json = JSON.parse(e.target.result).colors;" << endl
				<< "					for (var key in json) {" << endl
				<< "						if (json.hasOwnProperty(key) && json[key].length != 0) {" << endl
				<< "							for (var i=0, z=a.length; i < z; i++) {" << endl
				<< "								if (a[i].getAttribute('data-selector') == key) {" << endl
				<< "									a[i].value = json[key].split(':').pop();" << endl
				<< "								}" << endl
				<< "							}" << endl
				<< "						}" << endl
				<< "					}" << endl
				<< "				} catch (e) {" << endl
				<< "					alert(e);" << endl
				<< "				}" << endl
				<< "			};" << endl
				<< "		})(f);" << endl
				<< "		reader.readAsText(f);" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function handleDragOver(evt) {" << endl
				<< "	evt.stopPropagation();" << endl
				<< "	evt.preventDefault();" << endl
				<< "	evt.dataTransfer.dropEffect = 'copy';" << endl
				<< "	if (evt.currentTarget.className.indexOf('dragHover') == -1) {" << endl
				<< "		evt.currentTarget.className = 'dragHover ' + evt.currentTarget.className;" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function handleDragLeave(evt) {" << endl
				<< "	evt.stopPropagation();" << endl
				<< "	evt.preventDefault();" << endl
				<< "	evt.currentTarget.className = evt.currentTarget.className.replace('dragHover',''); " << endl
				<< "}" << endl
				<< "function swapColorScheme(evt){" << endl
				<< "	if (evt.currentTarget.id == 'useDarkColourScheme' && document.getElementById('cssButtonShow').className.indexOf('current') != -1){" << endl
				<< "		return;" << endl
				<< "	} else {" << endl
				<< "		evt.stopPropagation();" << endl
				<< "		evt.preventDefault();" << endl
				<< "		if (isValidationSupported() && !evt.currentTarget.checkValidity()){" << endl
				<< "			return;" << endl
				<< "		}" << endl
				<< "	}" << endl
				<< "	var checkbox, a;" << endl
				<< "	if (evt.currentTarget.id == 'useDarkColourScheme'){" << endl
				<< "		checkbox = evt.currentTarget;" << endl
				<< "		a = document.getElementById('cssSettings').querySelectorAll('input[type=text], input[type=color]');" << endl
				<< "	} else {" << endl
				<< "		checkbox = document.getElementById('useDarkColourScheme');" << endl
				<< "		a = evt.currentTarget;" << endl
				<< "	}" << endl
				<< "	if(checkbox.checked){" << endl
				<< "		for(var i=0,z=a.length;i<z;i++){" << endl
				<< "			if (a[i].type == 'text' || a[i].type == 'color'){" << endl
				<< "				var s = a[i].getAttribute('data-selector').replace(/\\.dark/g,'').replace('.current','').replace(':hover','');" << endl
				<< "				var c = document.querySelectorAll(s);" << endl
				<< "				for(var j=0,d=c.length;j<d;j++){" << endl
				<< "					if (c[j].className.indexOf('dark') == -1){" << endl
				<< "						c[j].className += ' dark';" << endl
				<< "					}" << endl
				<< "				}" << endl
				<< "			}" << endl
				<< "		}" << endl
				<< "	}else{" << endl
				<< "		for(var i=0,z=a.length;i<z;i++){" << endl
				<< "			if (a[i].type == 'text' || a[i].type == 'color'){" << endl
				<< "				var c = document.querySelectorAll(a[i].getAttribute('data-selector').replace('.current','').replace(':hover',''));" << endl
				<< "				for(var j=0,d=c.length;j<d;j++){" << endl
				<< "					c[j].className = c[j].className.replace(' dark','');" << endl
				<< "				}" << endl
				<< "			}" << endl
				<< "		}" << endl
				<< "	}" << endl
				<< "	return false;" << endl
				<< "}" << endl
				<< "function toggleDisplayCSS(evt){" << endl
				<< "	var e = getElementsByClassName(document, evt.currentTarget.getAttribute('data-class'), 'span');" << endl
				<< "	if(evt.currentTarget.checked){" << endl
				<< "		for(var i=0,z=e.length;i<z;i++){" << endl
				<< "			e[i].className += ' hidden';" << endl
				<< "		}" << endl
				<< "	}else{" << endl
				<< "		for(var i=0,z=e.length;i<z;i++){" << endl
				<< "			e[i].className = e[i].className.replace(' hidden','');" << endl
				<< "		}" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function backupCSS(evt){" << endl
				<< "	if (isCSSBackupRestoreSupported()) {" << endl
				<< "		var a = document.getElementById('cssSettings').querySelectorAll('input[type=text], input[type=color]');" << endl
				<< "		var json = '{\"colors\":{';" << endl
				<< "		for(var i=0,z=a.length;i<z;i++){" << endl
				<< "			json += '\"' + a[i].getAttribute('data-selector') + '\":\"' + a[i].getAttribute('data-property') + ':' + a[i].value + '\",'" << endl
				<< "		}" << endl
				<< "		json = json.substr(0,json.length-1);" << endl
				<< "		json += '}}';" << endl
				<< "		var uriContent = 'data:json/settings-backup,' + encodeURIComponent(json);" << endl
				<< "		location.href = uriContent;" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function toggleFilters(evt){" << endl
				<< "	var filters = document.getElementsByTagName('aside')[0];" << endl
				<< "	var arrow = document.getElementById('arrow');" << endl
				<< "	if (arrow.className.indexOf('rotated') == -1) {" << endl
				<< "		showElement(filters);" << endl
				<< "		arrow.className += ' rotated';" << endl
				<< "	} else {" << endl
				<< "		hideElement(filters);" << endl
				<< "		arrow.className = arrow.className.replace('rotated','');" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function showCSSBox(evt){" << endl
				<< "	if (isStorageSupported()) {" << endl
				<< "		var a = document.getElementById('cssSettings').querySelectorAll('input[type=text], input[type=color]');" << endl
				<< "		var len = localStorage.length;" << endl
				<< "		for (var i=0, z=a.length; i < z; i++) {" << endl
				<< "			var s = localStorage.getItem(a[i].getAttribute('data-selector'));" << endl
				<< "			if (s != null) {" << endl
				<< "				a[i].value = s.split(':').pop();" << endl
				<< "			}" << endl
				<< "		}" << endl
				<< "	}" << endl
				<< "	showElement(document.getElementById('cssSettings'));" << endl
				<< "}" << endl
				<< "function showSubmitBox(evt){" << endl
				<< "	document.getElementById('plugin').innerHTML=evt.currentTarget.innerHTML;" << endl
				<< "	showElement(document.getElementById('overlay'));" << endl
				<< "}" << endl
				<< "function hideSubmitBox(evt){" << endl
				<< "	var output = document.getElementById('output');" << endl
				<< "	hideElement(document.getElementById('overlay'));" << endl
				<< "	hideElement(output);" << endl
				<< "	showElement(document.getElementById('submitBox').getElementsByTagName('form')[0][2]);" << endl
				<< "	output.innerHTML='';" << endl
				<< "}" << endl
				<< "function submitPlugin(evt) {" << endl
				<< "	evt.stopPropagation();" << endl
				<< "	evt.preventDefault();" << endl
				<< "	if (evt.currentTarget[0].value.length == 0 && evt.currentTarget[1].value.length == 0){" << endl
				<< "		outputPluginSubmitText('Please supply at least a link or some notes.', -2);" << endl
				<< "		return;" << endl
				<< "	} else if (isValidationSupported() && !evt.currentTarget.checkValidity()){" << endl
				<< "		return;" << endl
				<< "	}" << endl
				<< "	var desc = evt.currentTarget[0].value;" << endl
				<< "	if (desc.length != 0) {" << endl
				<< "		desc += '\\n\\n';" << endl
				<< "	}" << endl
				<< "	desc += evt.currentTarget[1].value;" << endl
				<< "	try {" << endl
				<< "		var xhr = new XMLHttpRequest();" << endl
				<< "		getBugId(document.getElementById('plugin').innerHTML, desc, xhr);" << endl
				<< "	} catch(err) {" << endl
				<< "		outputPluginSubmitText('Exception occurred: ' + err.message, -1);" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function applyCSS(evt){" << endl
				<< "	evt.stopPropagation();" << endl
				<< "	evt.preventDefault();" << endl
				<< "	if (isValidationSupported() && !evt.currentTarget.checkValidity()){" << endl
				<< "		return;" << endl
				<< "	}" << endl
				<< "	for(var i=0,z=evt.currentTarget.length;i<z;i++){" << endl
				<< "		if (evt.currentTarget[i].type == 'text' || evt.currentTarget[i].type == 'color'){" << endl
				<< "			var css = evt.currentTarget[i].getAttribute('data-property') + ':' + evt.currentTarget[i].value;" << endl
				<< "			if (evt.currentTarget[i].value.length == 0) {" << endl
				<< "				css += evt.currentTarget[i].placeholder;" << endl
				<< "			}" << endl
				<< "			insertRule(document.styleSheets[0], evt.currentTarget[i].getAttribute('data-selector'), css);" << endl
				<< "		}" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function storeCSS(evt){" << endl
				<< "	evt.stopPropagation();" << endl
				<< "	evt.preventDefault();" << endl
				<< "	if (isValidationSupported() && !evt.currentTarget.checkValidity()){" << endl
				<< "		return;" << endl
				<< "	}" << endl
				<< "	for(var i=0,z=evt.currentTarget.length;i<z;i++){" << endl
				<< "		if (evt.currentTarget[i].type == 'text' || evt.currentTarget[i].type == 'color'){" << endl
				<< "			storeData(evt.currentTarget[i].getAttribute('data-selector'), evt.currentTarget[i].getAttribute('data-property') + ':' + evt.currentTarget[i].value);" << endl
				<< "		} else if (evt.currentTarget[i].type == 'checkbox'){" << endl
				<< "			if (evt.currentTarget[i].checked) {" << endl
				<< "				storeData(evt.currentTarget[i].id, true);" << endl
				<< "			} else {" << endl
				<< "				localStorage.removeItem(evt.currentTarget[i].id);" << endl
				<< "			}" << endl
				<< "		}" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function showSection(evt){" << endl
				<< "	hideElement(document.querySelector('section.visible'));" << endl
				<< "	showElement(document.getElementById(evt.currentTarget.getAttribute('data-section')));" << endl
				<< "	var elem = document.querySelector('nav div.current');" << endl
				<< "	if (elem != null){" << endl
				<< "		elem.className = elem.className.replace('current', '');" << endl
				<< "	}" << endl
				<< "	if (evt.currentTarget.className.indexOf('current') == -1) {" << endl
				<< "		evt.currentTarget.className += ' current';" << endl
				<< "	}" << endl
				<< "	//Also enable/disable filters based on current page." << endl
				<< "	var elemArr = document.getElementsByTagName('aside')[0].getElementsByTagName('input');" << endl
				<< "	for (var i=0, z=elemArr.length;i<z;i++){" << endl
				<< "		if (elemArr[i].id  == 'hideVersionNumbers' && (evt.currentTarget.getAttribute('data-section') == 'sePlugins' || evt.currentTarget.getAttribute('data-section') == 'recPlugins' || evt.currentTarget.getAttribute('data-section') == 'unrecPlugins')){" << endl
				<< "			elemArr[i].disabled = false;" << endl
				<< "		} else if (elemArr[i].id  == 'hideActiveLabel' && (evt.currentTarget.getAttribute('data-section') == 'sePlugins' || evt.currentTarget.getAttribute('data-section') == 'recPlugins' || evt.currentTarget.getAttribute('data-section') == 'unrecPlugins')){" << endl
				<< "			elemArr[i].disabled = false;" << endl
				<< "		} else if (elemArr[i].id  == 'hideChecksums' && (evt.currentTarget.getAttribute('data-section') == 'sePlugins' || evt.currentTarget.getAttribute('data-section') == 'recPlugins' || evt.currentTarget.getAttribute('data-section') == 'unrecPlugins')){" << endl
				<< "			elemArr[i].disabled = false;" << endl
				<< "		} else if (evt.currentTarget.getAttribute('data-section') == 'recPlugins'){" << endl
				<< "			elemArr[i].disabled = false;" << endl
				<< "		} else {" << endl
				<< "			elemArr[i].disabled = true;" << endl
				<< "		}" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function toggleMessages(evt){" << endl
				<< "	var listItems = document.getElementById('recPlugins').getElementsByTagName('li');" << endl
				<< "	var i = listItems.length - 1;" << endl
				<< "	var hiddenNo = parseInt(document.getElementById('hiddenMessageNo').innerHTML);" << endl
				<< "	while (i>-1){" << endl
				<< "		var spans = listItems[i].getElementsByTagName('span');" << endl
				<< "		if (spans.length == 0 || spans[0].className.indexOf('mod') == -1){" << endl
				<< "			var filterMatch = false;" << endl
				<< "			if (evt.currentTarget.id == 'hideAllPluginMessages'){" << endl
				<< "				filterMatch = true;" << endl
				<< "			} else if (evt.currentTarget.id == 'hideNotes' && listItems[i].className.indexOf('note') != -1){" << endl
				<< "				filterMatch = true;" << endl
				<< "			} else if (evt.currentTarget.id == 'hideBashTags' && listItems[i].className.indexOf('tag') != -1){" << endl
				<< "				filterMatch = true;" << endl
				<< "			} else if (evt.currentTarget.id == 'hideRequirements' && listItems[i].className.indexOf('req') != -1){" << endl
				<< "				filterMatch = true;" << endl
				<< "			} else if (evt.currentTarget.id == 'hideIncompatibilities' && listItems[i].className.indexOf('inc') != -1){" << endl
				<< "				filterMatch = true;" << endl
				<< "			} else if (evt.currentTarget.id == 'hideDoNotCleanMessages' && listItems[i].className.indexOf('dirty') != -1 && listItems[i].innerHTML.indexOf('Contains dirty edits: Do not clean.') != -1){" << endl
				<< "				filterMatch = true;" << endl
				<< "			}" << endl
				<< "			if (filterMatch){" << endl
				<< "				if (evt.currentTarget.checked){" << endl
				<< "					if (listItems[i].className.indexOf('hidden') == -1){" << endl
				<< "						hiddenNo++;" << endl
				<< "					}" << endl
				<< "					stepHideElement(listItems[i]);" << endl
				<< "				} else {" << endl
				<< "					stepUnhideElement(listItems[i]);" << endl
				<< "					if (listItems[i].className.indexOf('hidden') == -1){" << endl
				<< "						hiddenNo--;" << endl
				<< "					}" << endl
				<< "				}" << endl
				<< "			}" << endl
				<< "		}" << endl
				<< "		i--;" << endl
				<< "	}" << endl
				<< "	document.getElementById('hiddenMessageNo').innerHTML = hiddenNo;" << endl
				<< "	dispatchEvent(document.getElementById('hideMessagelessPlugins'),'click');" << endl
				<< "}" << endl
				<< "function togglePlugins(evt){" << endl
				<< "	var plugins = document.getElementById('recPlugins').getElementsByTagName('ul')[0].childNodes;" << endl
				<< "	var i = plugins.length - 1;" << endl
				<< "	var hiddenNo = parseInt(document.getElementById('hiddenPluginNo').innerHTML);" << endl
				<< "	while (i>-1){" << endl
				<< "		if (plugins[i].nodeType == Node.ELEMENT_NODE){" << endl
				<< "			var isMessageless = true," << endl
				<< "			isInactive = true," << endl
				<< "			isClean = true;" << endl
				<< "			var messages = plugins[i].getElementsByTagName('li');" << endl
				<< "			var j = messages.length - 1;" << endl
				<< "			while (j > -1){" << endl
				<< "				if (messages[j].className.indexOf('hidden') == -1){" << endl
				<< "					isMessageless = false;" << endl
				<< "					break;" << endl
				<< "				}" << endl
				<< "				j--;" << endl
				<< "			}" << endl
				<< "			if (getElementsByClassName(plugins[i], 'active', 'span').length != 0){" << endl
				<< "				isInactive = false;" << endl
				<< "			}" << endl
				<< "			if (getElementsByClassName(plugins[i], 'dirty', 'li').length != 0){" << endl
				<< "				isClean = false;" << endl
				<< "			}" << endl
				<< "			if ((document.getElementById('hideMessagelessPlugins').checked && isMessageless)" << endl
				<< "				|| (document.getElementById('hideInactivePlugins').checked && isInactive)" << endl
				<< "				|| (document.getElementById('hideCleanPlugins').checked && isClean)){" << endl
				<< "				if (plugins[i].className.indexOf('hidden') == -1){" << endl
				<< "					hiddenNo++;" << endl
				<< "					hideElement(plugins[i]);" << endl
				<< "				}" << endl
				<< "			} else if (plugins[i].className.indexOf('hidden') != -1){" << endl
				<< "					hiddenNo--;" << endl
				<< "					showElement(plugins[i]);" << endl
				<< "			}" << endl
				<< "		}" << endl
				<< "		i--;" << endl
				<< "	}" << endl
				<< "	document.getElementById('hiddenPluginNo').innerHTML = hiddenNo;" << endl
				<< "}" << endl
				<< "function outputPluginSubmitText(text, flag) {" << endl
				<< "	var output = document.getElementById('output');" << endl
				<< "	if (flag != -2)" << endl
				<< "	hideElement(document.getElementById('submitBox').getElementsByTagName('form')[0][2]);" << endl
				<< "	showElement(output);" << endl
				<< "	if (output.innerHTML.length != 0) {" << endl
				<< "		output.innerHTML += '<br />';" << endl
				<< "	}" << endl
				<< "	if (flag < 0) {" << endl
				<< "		text = \"<span style='color:red;'>\" + text + \"</span>\";" << endl
				<< "	} else if (flag == 1) {" << endl
				<< "		text = \"<span class='success'>\" + text + \"</span>\";" << endl
				<< "	}" << endl
				<< "	output.innerHTML += text;" << endl
				<< "}" << endl
				<< "function showBrowserBox(){" << endl
				<< "	if (suppressBrowserBox){" << endl
				<< "		var summaryButton = getElementsByClassName(document.getElementsByTagName('nav')[0], 'button', 'div')[0];" << endl
				<< "		if (summaryButton.className.indexOf('current') == -1){" << endl
				<< "			summaryButton.className += ' current';" << endl
				<< "		}" << endl
				<< "		showElement(document.getElementsByTagName('section')[0]);" << endl
				<< "		return;" << endl
				<< "	}" << endl
				<< "	if (isPluginSubmitSupported()) {" << endl
				<< "		document.getElementById('pluginSubmitSupport').className = 't';" << endl
				<< "	} else {" << endl
				<< "		document.getElementById('pluginSubmitSupport').className = 'c';" << endl
				<< "	}" << endl
				<< "	if (isCSSBackupRestoreSupported()) {" << endl
				<< "		document.getElementById('cssBackupSupport').className = 't';" << endl
				<< "	} else {" << endl
				<< "		document.getElementById('cssBackupSupport').className = 'c';" << endl
				<< "	}" << endl
				<< "	if (isStorageSupported()) {" << endl
				<< "		document.getElementById('memorySupport').className = 't';" << endl
				<< "	} else {" << endl
				<< "		hideElement(document.getElementById('browserBox').querySelector('label'));" << endl
				<< "		document.getElementById('memorySupport').className = 'c';" << endl
				<< "	}" << endl
				<< "	if (isStyleSupported('opacity')) {" << endl
				<< "		document.getElementById('opacitySupport').className = 't';" << endl
				<< "	} else {" << endl
				<< "		document.getElementById('opacitySupport').className = 'c';" << endl
				<< "	}" << endl
				<< "	if (isStyleSupported('boxShadow')) {" << endl
				<< "		document.getElementById('shadowsSupport').className = 't';" << endl
				<< "	} else {" << endl
				<< "		document.getElementById('shadowsSupport').className = 'c';" << endl
				<< "	}" << endl
				<< "	if (isStyleSupported('transition') || isStyleSupported('MozTransition') || isStyleSupported('webkitTransition') || isStyleSupported('OTransition') || isStyleSupported('msTransition')) {" << endl
				<< "		document.getElementById('transitionsSupport').className = 't';" << endl
				<< "	} else {" << endl
				<< "		document.getElementById('transitionsSupport').className = 'c';" << endl
				<< "	}" << endl
				<< "	if (isStyleSupported('transform') || isStyleSupported('MozTransform') || isStyleSupported('webkitTransform') || isStyleSupported('OTransform') || isStyleSupported('msTransform')) {" << endl
				<< "		document.getElementById('transformsSupport').className = 't';" << endl
				<< "	} else {" << endl
				<< "		document.getElementById('transformsSupport').className = 'c';" << endl
				<< "	}" << endl
				<< "	var i = document.createElement('input');" << endl
				<< "	i.setAttribute('type', 'color');" << endl
				<< "	if (i.type !== 'text') {" << endl
				<< "		document.getElementById('colorSupport').className = 't';" << endl
				<< "	} else {" << endl
				<< "		document.getElementById('colorSupport').className = 'c';" << endl
				<< "	}" << endl
				<< "	if ('placeholder' in document.createElement('input')) {" << endl
				<< "		document.getElementById('placeholderSupport').className = 't';" << endl
				<< "	} else {" << endl
				<< "		document.getElementById('placeholderSupport').className = 'c';" << endl
				<< "	}" << endl
				<< "	if (isValidationSupported()) {" << endl
				<< "		document.getElementById('validationSupport').className = 't';" << endl
				<< "	} else {" << endl
				<< "		document.getElementById('validationSupport').className = 'c';" << endl
				<< "	}" << endl
				<< "	showElement(document.getElementById('browserBox'));" << endl
				<< "}" << endl
				<< "function loadSettings(){" << endl
				<< "	var i = localStorage.length - 1;" << endl
				<< "	while (i > -1) {" << endl
				<< "		if (localStorage.key(i) == 'suppressBrowserBox') {" << endl
				<< "			suppressBrowserBox = true;" << endl
				<< "		} else {" << endl
				<< "			var elem = document.getElementById(localStorage.key(i));" << endl
				<< "			if (elem != null && 'defaultChecked' in elem) {" << endl
				<< "				elem.checked = true;" << endl
				<< "				dispatchEvent(elem,'click'); " << endl
				<< "			} else {" << endl
				<< "				if (document.styleSheets[0].cssRules) {" << endl
				<< "					var k = localStorage.key(i);" << endl
				<< "					var d = document.styleSheets[0];" << endl
				<< "					d.insertRule(k + '{' + localStorage.getItem(k) + '}', d.cssRules.length);" << endl
				<< "				}" << endl
				<< "			}" << endl
				<< "		}" << endl
				<< "		i--;" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function setupEventHandlers(){" << endl
				<< "	var i, elemArr;" << endl
				<< "	if (isStorageSupported()){  /*Set up filter value and CSS setting storage read/write handlers.*/" << endl
				<< "		elemArr = document.getElementsByTagName('aside')[0].getElementsByTagName('input');" << endl
				<< "		i = elemArr.length - 1;" << endl
				<< "		while(i > -1){" << endl
				<< "			addEventListener(elemArr[i], 'click', saveCheckboxState);" << endl
				<< "			i--;" << endl
				<< "		}" << endl
				<< "		addEventListener(document.getElementById('suppressBrowserBox'), 'click', saveCheckboxState);" << endl
				<< "		addEventListener(document.getElementById('cssSettings').getElementsByTagName('form')[0], 'submit', storeCSS);" << endl
				<< "	}" << endl
				<< "	if (isCSSBackupRestoreSupported()){  /*Set up handlers for CSS backup & restore.*/" << endl
				<< "		var dropZone = document.getElementById('cssSettings');" << endl
				<< "		addEventListener(dropZone, 'dragover', handleDragOver);" << endl
				<< "		addEventListener(dropZone, 'drop', handleFileSelect);" << endl
				<< "		addEventListener(dropZone, 'dragleave', handleDragLeave);" << endl
				<< "		addEventListener(document.getElementById('cssButtonBackup'), 'click', backupCSS);" << endl
				<< "	}" << endl
				<< "	if (isPluginSubmitSupported() && document.getElementById('unrecPlugins') != null){  /*Set up handlers for plugin submitter.*/" << endl
				<< "		elemArr = document.getElementById('unrecPlugins').querySelectorAll('span.mod');" << endl
				<< "		i = elemArr.length - 1;" << endl
				<< "		while(i > -1){" << endl
				<< "			addEventListener(elemArr[i], 'click', showSubmitBox);" << endl
				<< "			i--;" << endl
				<< "		}" << endl
				<< "		addEventListener(document.getElementById('submitBox').getElementsByTagName('form')[0], 'reset', hideSubmitBox);" << endl
				<< "		addEventListener(document.getElementById('submitBox').getElementsByTagName('form')[0], 'submit', submitPlugin);" << endl
				<< "	}" << endl
				<< "	addEventListener(document.getElementById('filtersButtonToggle'), 'click', toggleFilters);" << endl
				<< "	/*Set up handlers for section display.*/" << endl
				<< "	elemArr = document.getElementsByTagName('nav')[0].querySelectorAll('nav > div.button');" << endl
				<< "	var i = elemArr.length - 1;" << endl
				<< "	while(i > -1){" << endl
				<< "		addEventListener(elemArr[i], 'click', showSection);" << endl
				<< "		i--;" << endl
				<< "	}" << endl
				<< "	addEventListener(document.getElementById('cssButtonShow'), 'click', showSection);" << endl
				<< "	addEventListener(document.getElementById('cssButtonShow'), 'click', showCSSBox);" << endl
				<< "	addEventListener(document.getElementById('cssSettings').getElementsByTagName('form')[0], 'submit', applyCSS);" << endl
				<< "	addEventListener(document.getElementById('cssSettings').getElementsByTagName('form')[0], 'submit', swapColorScheme);" << endl
				<< "	addEventListener(document.getElementById('useDarkColourScheme'), 'click', swapColorScheme);" << endl
				<< "	/*Set up handlers for filters.*/" << endl
				<< "	addEventListener(document.getElementById('hideVersionNumbers'), 'click', toggleDisplayCSS);" << endl
				<< "	addEventListener(document.getElementById('hideActiveLabel'), 'click', toggleDisplayCSS);" << endl
				<< "	addEventListener(document.getElementById('hideChecksums'), 'click', toggleDisplayCSS);" << endl
				<< "	addEventListener(document.getElementById('hideNotes'), 'click', toggleMessages);" << endl
				<< "	addEventListener(document.getElementById('hideBashTags'), 'click', toggleMessages);" << endl
				<< "	addEventListener(document.getElementById('hideRequirements'), 'click', toggleMessages);" << endl
				<< "	addEventListener(document.getElementById('hideIncompatibilities'), 'click', toggleMessages);" << endl
				<< "	addEventListener(document.getElementById('hideDoNotCleanMessages'), 'click', toggleMessages);" << endl
				<< "	addEventListener(document.getElementById('hideAllPluginMessages'), 'click', toggleMessages);" << endl
				<< "	addEventListener(document.getElementById('hideInactivePlugins'), 'click', togglePlugins);" << endl
				<< "	addEventListener(document.getElementById('hideMessagelessPlugins'), 'click', togglePlugins);" << endl
				<< "	addEventListener(document.getElementById('hideCleanPlugins'), 'click', togglePlugins);" << endl
				<< "}" << endl
				<< "function applyFeatureSupportRestrictions(){" << endl
				<< "	if (!isPluginSubmitSupported()) { /*Disable unrecognised mod underline effect.*/" << endl
				<< "		var buttons = document.getElementById('unrecPlugins').querySelectorAll('span.mod');" << endl
				<< "		for (var i=0, len=buttons.length; i < len; i++) {" << endl
				<< "			buttons[i].className += 'nosubmit';" << endl
				<< "		}" << endl
				<< "		hideElement(document.getElementById('unrecPluginsSubmitNote'));" << endl
				<< "	}" << endl
				<< "	if (isStorageSupported()) {" << endl
				<< "		hideElement(document.getElementById('loseSettingsClose'));" << endl
				<< "	} else {" << endl
				<< "		hideElement(document.getElementById('loseSettingsCacheClear'));" << endl
				<< "	}" << endl
				<< "	if (!isCSSBackupRestoreSupported()) {" << endl
				<< "		hideElement(document.getElementById('backupCSSNote'));" << endl
				<< "	}" << endl
				<< "	var i = document.createElement('input');" << endl
				<< "	i.setAttribute('type', 'color');" << endl
				<< "	if (i.type === 'text') {" << endl
				<< "		hideElement(document.getElementById('colorPickerNote'));" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function init(){" << endl
				<< "	setupEventHandlers();" << endl
				<< "	if (isStorageSupported()){" << endl
				<< "		loadSettings();" << endl
				<< "	}" << endl
				<< "	applyFeatureSupportRestrictions();" << endl
				<< "	//Initially disable all filters." << endl
				<< "	var elemArr = document.getElementsByTagName('aside')[0].getElementsByTagName('input');" << endl
				<< "	for (var i=0, z=elemArr.length;i<z;i++){" << endl
				<< "		elemArr[i].disabled = true;" << endl
				<< "	}" << endl
				<< "	showBrowserBox();" << endl
				<< "}" << endl
				<< "init();" << endl
				<< "</script>" << endl;
		}
	}

	void Outputter::Save(fs::path file, bool overwrite) {
		ofstream outFile;
		if (overwrite)
			outFile.open(file.c_str());
		else
			outFile.open(file.c_str(), ios_base::out|ios_base::app);
		if (outFile.fail())
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());

		outFile << outStream.str();
		outFile.close();
	}
	
	string Outputter::AsString() {
		return outStream.str();
	}

	//Escapes HTML special characters.
	string Outputter::EscapeHTMLSpecial(string text) {
		if (escapeHTMLSpecialChars) {
			replace_all(text, "&", "&amp;");
			replace_all(text, "\"", "&quot;");
			replace_all(text, "'", "&#039;");
			replace_all(text, "<", "&lt;");
			replace_all(text, ">", "&gt;");
			replace_all(text, "©", "&copy;");
		}
		return text;
	}

	string Outputter::EscapeHTMLSpecial(char c) {
		if (escapeHTMLSpecialChars) {
			switch(c) {
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
		}
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
		switch(l) {
		case SECTION_ID_SUMMARY_OPEN:
			if (outFormat == HTML)
				outStream << "<section id='summary'>";
			break;
		case SECTION_ID_GENERAL_OPEN:
			if (outFormat == HTML)
				outStream << "<section id='generalMessages'>";
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
		case DIV_GENERAL_BUTTON_OPEN:
			if (outFormat == HTML)
				outStream << "<div class='button' data-section='generalMessages'>";
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
		case TABLE_ROW:
			if (outFormat == HTML)
				outStream << "<tr>";
			break;
		case TABLE_DATA:
			if (outFormat == HTML)
				outStream << "<td>";
			else
				outStream << endl;
			break;
		case TABLE_OPEN:
			if (outFormat == HTML)
				outStream << "<table><tbody>";
			break;
		case TABLE_CLOSE:
			if (outFormat == HTML)
				outStream << "</table>";
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
				outStream << endl << "*  ";
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
			else
				outStream << endl << endl;
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
		case SPAN_CLASS_ERROR_OPEN:
			if (outFormat == HTML)
				outStream << "<span class='error'>";
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
		//If bosslog format is HTML, wrap web addresses in HTML link format.
		if (outFormat == HTML) {
			size_t pos1,pos2;
			string link;
			pos1 = data.find("&quot;http");  //Start of a link, HTML escaped.
			while (pos1 != string::npos) {
				pos1 += 6;  //Now points to start of actual link.
				pos2 = data.find("&quot;",pos1);  //First character after the end of the link.
				link = data.substr(pos1,pos2-pos1);
				link = "<a href=\"" + link + "\">" + link + "</a>";
				data.replace(pos1-6,pos2-pos1+12,link);
				pos1 = data.find("&quot;http",pos1);
			}
		}
		//Select message formatting.
		switch(m.Key()) {
		case TAG:
			if (outFormat == HTML)
				outStream << "<li class='tag'><span class='tagPrefix'>Bash Tag suggestion(s):</span> " << data;
			else
				outStream << endl << "*  Bash Tag suggestion(s):" << data;
			break;
		case SAY:
			if (outFormat == HTML)
				outStream << "<li class='note'>Note: " << data;
			else
				outStream << endl << "*  Note: " << data;
			break;
		case REQ:
			if (outFormat == HTML)
				outStream << "<li class='req'>Requires: " << data;
			else
				outStream << endl << "*  Requires: " << data;
			break;
		case INC:
			if (outFormat == HTML)
				outStream << "<li class='inc'>Incompatible with: " << data;
			else
				outStream << endl << "*  Incompatible with: " << data;
			break;
		case WARN:
			if (outFormat == HTML)
				outStream << "<li class='warn'>Warning: " << data;
			else
				outStream << endl << "*  Warning: " << data;
			break;
		case ERR:
			if (outFormat == HTML)
				outStream << "<li class='error'>ERROR: " << data;
			else
				outStream << endl << "*  ERROR: " << data;
			break;
		case DIRTY:
			if (outFormat == HTML)
				outStream << "<li class='dirty'>Contains dirty edits: " << data;
			else
				outStream << endl << "*  Contains dirty edits: " << data;
			break;
		default:
			if (outFormat == HTML)
				outStream << "<li class='note'>Note: " << data;
			else
				outStream << endl << "*  Note: " << data;
			break;
		}
		return *this;
	}

	//Outputs correctly-formatted error message.
	string ParsingError::FormatFor(const uint32_t format) {
		Outputter output(format);
		if (Empty())
			output << "";
		else if (!wholeMessage.empty())
			output << LIST_ITEM_CLASS_ERROR << wholeMessage;
		else {
			if (format == HTML)
				boost::replace_all(detail, "\n", "<br />");
			output << LIST_ITEM << SPAN_CLASS_ERROR_OPEN << header << SPAN_CLOSE 
				<< BLOCKQUOTE_OPEN << detail << BLOCKQUOTE_CLOSE
				<< SPAN_CLASS_ERROR_OPEN << footer << SPAN_CLOSE;
		}
		return output.AsString();
	}

	Transcoder::Transcoder() {
		//Fill common character maps (1251/1252 -> UTF-8).
		commonMap.emplace('\x00', 0x0000);
		commonMap.emplace('\x01', 0x0001);
		commonMap.emplace('\x02', 0x0002);
		commonMap.emplace('\x03', 0x0003);
		commonMap.emplace('\x04', 0x0004);
		commonMap.emplace('\x05', 0x0005);
		commonMap.emplace('\x06', 0x0006);
		commonMap.emplace('\x07', 0x0007);
		commonMap.emplace('\x08', 0x0008);
		commonMap.emplace('\x09', 0x0009);
		commonMap.emplace('\x0A', 0x000A);
		commonMap.emplace('\x0B', 0x000B);
		commonMap.emplace('\x0C', 0x000C);
		commonMap.emplace('\x0D', 0x000D);
		commonMap.emplace('\x0E', 0x000E);
		commonMap.emplace('\x0F', 0x000F);
		commonMap.emplace('\x10', 0x0010);
		commonMap.emplace('\x11', 0x0011);
		commonMap.emplace('\x12', 0x0012);
		commonMap.emplace('\x13', 0x0013);
		commonMap.emplace('\x14', 0x0014);
		commonMap.emplace('\x15', 0x0015);
		commonMap.emplace('\x16', 0x0016);
		commonMap.emplace('\x17', 0x0017);
		commonMap.emplace('\x18', 0x0018);
		commonMap.emplace('\x19', 0x0019);
		commonMap.emplace('\x1A', 0x001A);
		commonMap.emplace('\x1B', 0x001B);
		commonMap.emplace('\x1C', 0x001C);
		commonMap.emplace('\x1D', 0x001D);
		commonMap.emplace('\x1E', 0x001E);
		commonMap.emplace('\x1F', 0x001F);
		commonMap.emplace('\x20', 0x0020);
		commonMap.emplace('\x21', 0x0021);
		commonMap.emplace('\x22', 0x0022);
		commonMap.emplace('\x23', 0x0023);
		commonMap.emplace('\x24', 0x0024);
		commonMap.emplace('\x25', 0x0025);
		commonMap.emplace('\x26', 0x0026);
		commonMap.emplace('\x27', 0x0027);
		commonMap.emplace('\x28', 0x0028);
		commonMap.emplace('\x29', 0x0029);
		commonMap.emplace('\x2A', 0x002A);
		commonMap.emplace('\x2B', 0x002B);
		commonMap.emplace('\x2C', 0x002C);
		commonMap.emplace('\x2D', 0x002D);
		commonMap.emplace('\x2E', 0x002E);
		commonMap.emplace('\x2F', 0x002F);
		commonMap.emplace('\x30', 0x0030);
		commonMap.emplace('\x31', 0x0031);
		commonMap.emplace('\x32', 0x0032);
		commonMap.emplace('\x33', 0x0033);
		commonMap.emplace('\x34', 0x0034);
		commonMap.emplace('\x35', 0x0035);
		commonMap.emplace('\x36', 0x0036);
		commonMap.emplace('\x37', 0x0037);
		commonMap.emplace('\x38', 0x0038);
		commonMap.emplace('\x39', 0x0039);
		commonMap.emplace('\x3A', 0x003A);
		commonMap.emplace('\x3B', 0x003B);
		commonMap.emplace('\x3C', 0x003C);
		commonMap.emplace('\x3D', 0x003D);
		commonMap.emplace('\x3E', 0x003E);
		commonMap.emplace('\x3F', 0x003F);
		commonMap.emplace('\x40', 0x0040);
		commonMap.emplace('\x41', 0x0041);
		commonMap.emplace('\x42', 0x0042);
		commonMap.emplace('\x43', 0x0043);
		commonMap.emplace('\x44', 0x0044);
		commonMap.emplace('\x45', 0x0045);
		commonMap.emplace('\x46', 0x0046);
		commonMap.emplace('\x47', 0x0047);
		commonMap.emplace('\x48', 0x0048);
		commonMap.emplace('\x49', 0x0049);
		commonMap.emplace('\x4A', 0x004A);
		commonMap.emplace('\x4B', 0x004B);
		commonMap.emplace('\x4C', 0x004C);
		commonMap.emplace('\x4D', 0x004D);
		commonMap.emplace('\x4E', 0x004E);
		commonMap.emplace('\x4F', 0x004F);
		commonMap.emplace('\x50', 0x0050);
		commonMap.emplace('\x51', 0x0051);
		commonMap.emplace('\x52', 0x0052);
		commonMap.emplace('\x53', 0x0053);
		commonMap.emplace('\x54', 0x0054);
		commonMap.emplace('\x55', 0x0055);
		commonMap.emplace('\x56', 0x0056);
		commonMap.emplace('\x57', 0x0057);
		commonMap.emplace('\x58', 0x0058);
		commonMap.emplace('\x59', 0x0059);
		commonMap.emplace('\x5A', 0x005A);
		commonMap.emplace('\x5B', 0x005B);
		commonMap.emplace('\x5C', 0x005C);
		commonMap.emplace('\x5D', 0x005D);
		commonMap.emplace('\x5E', 0x005E);
		commonMap.emplace('\x5F', 0x005F);
		commonMap.emplace('\x60', 0x0060);
		commonMap.emplace('\x61', 0x0061);
		commonMap.emplace('\x62', 0x0062);
		commonMap.emplace('\x63', 0x0063);
		commonMap.emplace('\x64', 0x0064);
		commonMap.emplace('\x65', 0x0065);
		commonMap.emplace('\x66', 0x0066);
		commonMap.emplace('\x67', 0x0067);
		commonMap.emplace('\x68', 0x0068);
		commonMap.emplace('\x69', 0x0069);
		commonMap.emplace('\x6A', 0x006A);
		commonMap.emplace('\x6B', 0x006B);
		commonMap.emplace('\x6C', 0x006C);
		commonMap.emplace('\x6D', 0x006D);
		commonMap.emplace('\x6E', 0x006E);
		commonMap.emplace('\x6F', 0x006F);
		commonMap.emplace('\x70', 0x0070);
		commonMap.emplace('\x71', 0x0071);
		commonMap.emplace('\x72', 0x0072);
		commonMap.emplace('\x73', 0x0073);
		commonMap.emplace('\x74', 0x0074);
		commonMap.emplace('\x75', 0x0075);
		commonMap.emplace('\x76', 0x0076);
		commonMap.emplace('\x77', 0x0077);
		commonMap.emplace('\x78', 0x0078);
		commonMap.emplace('\x79', 0x0079);
		commonMap.emplace('\x7A', 0x007A);
		commonMap.emplace('\x7B', 0x007B);
		commonMap.emplace('\x7C', 0x007C);
		commonMap.emplace('\x7D', 0x007D);
		commonMap.emplace('\x7E', 0x007E);
		commonMap.emplace('\x7F', 0x007F);
		commonMap.emplace('\x82', 0x201A);
		commonMap.emplace('\x84', 0x201E);
		commonMap.emplace('\x85', 0x2026);
		commonMap.emplace('\x86', 0x2026);
		commonMap.emplace('\x87', 0x2021);
		commonMap.emplace('\x89', 0x2030);
		commonMap.emplace('\x8B', 0x2039);
		commonMap.emplace('\x91', 0x2018);
		commonMap.emplace('\x92', 0x2019);
		commonMap.emplace('\x93', 0x201C);
		commonMap.emplace('\x94', 0x201D);
		commonMap.emplace('\x95', 0x2022);
		commonMap.emplace('\x96', 0x2013);
		commonMap.emplace('\x97', 0x2014);
		commonMap.emplace('\x99', 0x2122);
		commonMap.emplace('\x9B', 0x203A);
		commonMap.emplace('\xA0', 0x00A0);
		commonMap.emplace('\xA4', 0x00A4);
		commonMap.emplace('\xA6', 0x00A6);
		commonMap.emplace('\xA7', 0x00A7);
		commonMap.emplace('\xA9', 0x00A9);
		commonMap.emplace('\xAB', 0x00AB);
		commonMap.emplace('\xAC', 0x00AC);
		commonMap.emplace('\xAD', 0x00AD);
		commonMap.emplace('\xAE', 0x00AE);
		commonMap.emplace('\xB0', 0x00B0);
		commonMap.emplace('\xB1', 0x00B1);
		commonMap.emplace('\xB5', 0x00B5);
		commonMap.emplace('\xB6', 0x00B6);
		commonMap.emplace('\xB7', 0x00B7);
		commonMap.emplace('\xBB', 0x00BB);
		/*
		//Now fill 1251 -> UTF-8 map.
		map1251toUtf8 = commonMap;  //Fill with common mapped characters.
		map1251toUtf8.emplace('\x80', 0x0402);
		map1251toUtf8.emplace('\x81', 0x0403);
		map1251toUtf8.emplace('\x83', 0x0453);
		map1251toUtf8.emplace('\x88', 0x20AC);
		map1251toUtf8.emplace('\x8A', 0x0409);
		map1251toUtf8.emplace('\x8C', 0x040A);
		map1251toUtf8.emplace('\x8D', 0x040C);
		map1251toUtf8.emplace('\x8E', 0x040B);
		map1251toUtf8.emplace('\x8F', 0x040F);
		map1251toUtf8.emplace('\x90', 0x0452);
		map1251toUtf8.emplace('\x98', 0x0020);
		map1251toUtf8.emplace('\x9A', 0x0459);
		map1251toUtf8.emplace('\x9C', 0x045A);
		map1251toUtf8.emplace('\x9D', 0x045C);
		map1251toUtf8.emplace('\x9E', 0x045B);
		map1251toUtf8.emplace('\x9F', 0x045F);
		map1251toUtf8.emplace('\xA1', 0x040E);
		map1251toUtf8.emplace('\xA2', 0x045E);
		map1251toUtf8.emplace('\xA3', 0x0408);
		map1251toUtf8.emplace('\xA5', 0x0490);
		map1251toUtf8.emplace('\xA8', 0x0401);
		map1251toUtf8.emplace('\xAA', 0x0404);
		map1251toUtf8.emplace('\xAF', 0x0407);
		map1251toUtf8.emplace('\xB2', 0x0406);
		map1251toUtf8.emplace('\xB3', 0x0456);
		map1251toUtf8.emplace('\xB4', 0x0491);
		map1251toUtf8.emplace('\xB8', 0x0451);
		map1251toUtf8.emplace('\xB9', 0x2116);
		map1251toUtf8.emplace('\xBA', 0x0454);
		map1251toUtf8.emplace('\xBC', 0x0458);
		map1251toUtf8.emplace('\xBD', 0x0405);
		map1251toUtf8.emplace('\xBE', 0x0455);
		map1251toUtf8.emplace('\xBF', 0x0457);
		map1251toUtf8.emplace('\xC0', 0x0410);
		map1251toUtf8.emplace('\xC1', 0x0411);
		map1251toUtf8.emplace('\xC2', 0x0412);
		map1251toUtf8.emplace('\xC3', 0x0413);
		map1251toUtf8.emplace('\xC4', 0x0414);
		map1251toUtf8.emplace('\xC5', 0x0415);
		map1251toUtf8.emplace('\xC6', 0x0416);
		map1251toUtf8.emplace('\xC7', 0x0417);
		map1251toUtf8.emplace('\xC8', 0x0418);
		map1251toUtf8.emplace('\xC9', 0x0419);
		map1251toUtf8.emplace('\xCA', 0x041A);
		map1251toUtf8.emplace('\xCB', 0x041B);
		map1251toUtf8.emplace('\xCC', 0x041C);
		map1251toUtf8.emplace('\xCD', 0x041D);
		map1251toUtf8.emplace('\xCE', 0x041E);
		map1251toUtf8.emplace('\xCF', 0x041F);
		map1251toUtf8.emplace('\xD0', 0x0420);
		map1251toUtf8.emplace('\xD1', 0x0421);
		map1251toUtf8.emplace('\xD2', 0x0422);
		map1251toUtf8.emplace('\xD3', 0x0423);
		map1251toUtf8.emplace('\xD4', 0x0424);
		map1251toUtf8.emplace('\xD5', 0x0425);
		map1251toUtf8.emplace('\xD6', 0x0426);
		map1251toUtf8.emplace('\xD7', 0x0427);
		map1251toUtf8.emplace('\xD8', 0x0428);
		map1251toUtf8.emplace('\xD9', 0x0429);
		map1251toUtf8.emplace('\xDA', 0x042A);
		map1251toUtf8.emplace('\xDB', 0x042B);
		map1251toUtf8.emplace('\xDC', 0x042C);
		map1251toUtf8.emplace('\xDD', 0x042D);
		map1251toUtf8.emplace('\xDE', 0x042E);
		map1251toUtf8.emplace('\xDF', 0x042F);
		map1251toUtf8.emplace('\xE0', 0x0430);
		map1251toUtf8.emplace('\xE1', 0x0431);
		map1251toUtf8.emplace('\xE2', 0x0432);
		map1251toUtf8.emplace('\xE3', 0x0433);
		map1251toUtf8.emplace('\xE4', 0x0434);
		map1251toUtf8.emplace('\xE5', 0x0435);
		map1251toUtf8.emplace('\xE6', 0x0436);
		map1251toUtf8.emplace('\xE7', 0x0437);
		map1251toUtf8.emplace('\xE8', 0x0438);
		map1251toUtf8.emplace('\xE9', 0x0439);
		map1251toUtf8.emplace('\xEA', 0x043A);
		map1251toUtf8.emplace('\xEB', 0x043B);
		map1251toUtf8.emplace('\xEC', 0x043C);
		map1251toUtf8.emplace('\xED', 0x043D);
		map1251toUtf8.emplace('\xEE', 0x043E);
		map1251toUtf8.emplace('\xEF', 0x043F);
		map1251toUtf8.emplace('\xF0', 0x0440);
		map1251toUtf8.emplace('\xF1', 0x0441);
		map1251toUtf8.emplace('\xF2', 0x0442);
		map1251toUtf8.emplace('\xF3', 0x0443);
		map1251toUtf8.emplace('\xF4', 0x0444);
		map1251toUtf8.emplace('\xF5', 0x0445);
		map1251toUtf8.emplace('\xF6', 0x0446);
		map1251toUtf8.emplace('\xF7', 0x0447);
		map1251toUtf8.emplace('\xF8', 0x0448);
		map1251toUtf8.emplace('\xF9', 0x0449);
		map1251toUtf8.emplace('\xFA', 0x044A);
		map1251toUtf8.emplace('\xFB', 0x044B);
		map1251toUtf8.emplace('\xFC', 0x044C);
		map1251toUtf8.emplace('\xFD', 0x044D);
		map1251toUtf8.emplace('\xFE', 0x044E);
		map1251toUtf8.emplace('\xFF', 0x044F);
		*/
		//Now fill 1252 -> UTF-8 map.
		map1252toUtf8 = commonMap;  //Fill with common mapped characters.
		map1252toUtf8.emplace('\x80', 0x20AC);
		map1252toUtf8.emplace('\x83', 0x0192);
		map1252toUtf8.emplace('\x88', 0x02C6);
		map1252toUtf8.emplace('\x8A', 0x0160);
		map1252toUtf8.emplace('\x8C', 0x0152);
		map1252toUtf8.emplace('\x8E', 0x017D);
		map1252toUtf8.emplace('\x98', 0x02DC);
		map1252toUtf8.emplace('\x9A', 0x0161);
		map1252toUtf8.emplace('\x9C', 0x0153);
		map1252toUtf8.emplace('\x9E', 0x017E);
		map1252toUtf8.emplace('\x9F', 0x0178);
		map1252toUtf8.emplace('\xA1', 0x00A1);
		map1252toUtf8.emplace('\xA2', 0x00A2);
		map1252toUtf8.emplace('\xA3', 0x00A3);
		map1252toUtf8.emplace('\xA5', 0x00A5);
		map1252toUtf8.emplace('\xA8', 0x00A8);
		map1252toUtf8.emplace('\xAA', 0x00AA);
		map1252toUtf8.emplace('\xAF', 0x00AF);
		map1252toUtf8.emplace('\xB2', 0x00B2);
		map1252toUtf8.emplace('\xB3', 0x00B3);
		map1252toUtf8.emplace('\xB4', 0x00B4);
		map1252toUtf8.emplace('\xB8', 0x00B8);
		map1252toUtf8.emplace('\xB9', 0x00B9);
		map1252toUtf8.emplace('\xBA', 0x00BA);
		map1252toUtf8.emplace('\xBC', 0x00BC);
		map1252toUtf8.emplace('\xBD', 0x00BD);
		map1252toUtf8.emplace('\xBE', 0x00BE);
		map1252toUtf8.emplace('\xBF', 0x00BF);
		map1252toUtf8.emplace('\xC0', 0x00C0);
		map1252toUtf8.emplace('\xC1', 0x00C1);
		map1252toUtf8.emplace('\xC2', 0x00C2);
		map1252toUtf8.emplace('\xC3', 0x00C3);
		map1252toUtf8.emplace('\xC4', 0x00C4);
		map1252toUtf8.emplace('\xC5', 0x00C5);
		map1252toUtf8.emplace('\xC6', 0x00C6);
		map1252toUtf8.emplace('\xC7', 0x00C7);
		map1252toUtf8.emplace('\xC8', 0x00C8);
		map1252toUtf8.emplace('\xC9', 0x00C9);
		map1252toUtf8.emplace('\xCA', 0x00CA);
		map1252toUtf8.emplace('\xCB', 0x00CB);
		map1252toUtf8.emplace('\xCC', 0x00CC);
		map1252toUtf8.emplace('\xCD', 0x00CD);
		map1252toUtf8.emplace('\xCE', 0x00CE);
		map1252toUtf8.emplace('\xCF', 0x00CF);
		map1252toUtf8.emplace('\xD0', 0x00D0);
		map1252toUtf8.emplace('\xD1', 0x00D1);
		map1252toUtf8.emplace('\xD2', 0x00D2);
		map1252toUtf8.emplace('\xD3', 0x00D3);
		map1252toUtf8.emplace('\xD4', 0x00D4);
		map1252toUtf8.emplace('\xD5', 0x00D5);
		map1252toUtf8.emplace('\xD6', 0x00D6);
		map1252toUtf8.emplace('\xD7', 0x00D7);
		map1252toUtf8.emplace('\xD8', 0x00D8);
		map1252toUtf8.emplace('\xD9', 0x00D9);
		map1252toUtf8.emplace('\xDA', 0x00DA);
		map1252toUtf8.emplace('\xDB', 0x00DB);
		map1252toUtf8.emplace('\xDC', 0x00DC);
		map1252toUtf8.emplace('\xDD', 0x00DD);
		map1252toUtf8.emplace('\xDE', 0x00DE);
		map1252toUtf8.emplace('\xDF', 0x00DF);
		map1252toUtf8.emplace('\xE0', 0x00E0);
		map1252toUtf8.emplace('\xE1', 0x00E1);
		map1252toUtf8.emplace('\xE2', 0x00E2);
		map1252toUtf8.emplace('\xE3', 0x00E3);
		map1252toUtf8.emplace('\xE4', 0x00E4);
		map1252toUtf8.emplace('\xE5', 0x00E5);
		map1252toUtf8.emplace('\xE6', 0x00E6);
		map1252toUtf8.emplace('\xE7', 0x00E7);
		map1252toUtf8.emplace('\xE8', 0x00E8);
		map1252toUtf8.emplace('\xE9', 0x00E9);
		map1252toUtf8.emplace('\xEA', 0x00EA);
		map1252toUtf8.emplace('\xEB', 0x00EB);
		map1252toUtf8.emplace('\xEC', 0x00EC);
		map1252toUtf8.emplace('\xED', 0x00ED);
		map1252toUtf8.emplace('\xEE', 0x00EE);
		map1252toUtf8.emplace('\xEF', 0x00EF);
		map1252toUtf8.emplace('\xF0', 0x00F0);
		map1252toUtf8.emplace('\xF1', 0x00F1);
		map1252toUtf8.emplace('\xF2', 0x00F2);
		map1252toUtf8.emplace('\xF3', 0x00F3);
		map1252toUtf8.emplace('\xF4', 0x00F4);
		map1252toUtf8.emplace('\xF5', 0x00F5);
		map1252toUtf8.emplace('\xF6', 0x00F6);
		map1252toUtf8.emplace('\xF7', 0x00F7);
		map1252toUtf8.emplace('\xF8', 0x00F8);
		map1252toUtf8.emplace('\xF9', 0x00F9);
		map1252toUtf8.emplace('\xFA', 0x00FA);
		map1252toUtf8.emplace('\xFB', 0x00FB);
		map1252toUtf8.emplace('\xFC', 0x00FC);
		map1252toUtf8.emplace('\xFD', 0x00FD);
		map1252toUtf8.emplace('\xFE', 0x00FE);
		map1252toUtf8.emplace('\xFF', 0x00FF);
		
		currentEncoding = 0;
	}

	void Transcoder::SetEncoding(uint32_t inEncoding) {
		if (inEncoding != 1252)
			return;

		//Set the enc -> UTF-8 map.
		encToUtf8 = map1252toUtf8;
		currentEncoding = inEncoding;
		
		//Now create the UTF-8 -> enc map.
		for (boost::unordered_map<char, uint32_t>::iterator iter = encToUtf8.begin(); iter != encToUtf8.end(); ++iter) {
			utf8toEnc.emplace(iter->second, iter->first);  //Swap mapping. There *should* be unique values for each character either way.
		}
	}

	uint32_t Transcoder::GetEncoding() {
		return currentEncoding;
	}

	string Transcoder::Utf8ToEnc(string inString) {
		stringstream outString;
		string::iterator strIter = inString.begin();
		//I need to use a UTF-8 string iterator. See UTF-CPP for usage.
		try {
			utf8::iterator<string::iterator> iter(strIter, strIter, inString.end());
			for (iter; iter.base() != inString.end(); ++iter) {
				boost::unordered_map<uint32_t, char>::iterator mapIter = utf8toEnc.find(*iter);
				if (mapIter != utf8toEnc.end())
					outString << mapIter->second;
				else
					throw boss_error(BOSS_ERROR_ENCODING_CONVERSION_FAIL, inString);
			}
		} catch (...) {
			throw boss_error(BOSS_ERROR_ENCODING_CONVERSION_FAIL, inString);
		}
		return outString.str();
	}

	string Transcoder::EncToUtf8(string inString) {
		string outString;
		for (string::iterator iter = inString.begin(); iter != inString.end(); ++iter) {
			boost::unordered_map<char, uint32_t>::iterator mapIter = encToUtf8.find(*iter);
			if (mapIter != encToUtf8.end()) {
				try {
					utf8::append(mapIter->second, back_inserter(outString));
				} catch (...) {
					throw boss_error(BOSS_ERROR_ENCODING_CONVERSION_FAIL, inString);
				}
			} else
				throw boss_error(BOSS_ERROR_ENCODING_CONVERSION_FAIL, inString);
		}
		//Let's check that it's valid UTF-8.
		if (!utf8::is_valid(outString.begin(), outString.end()))
			throw boss_error(BOSS_ERROR_ENCODING_CONVERSION_FAIL, outString);
		return outString;
	}
}