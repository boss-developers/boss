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

	void Outputter::PrintHeader() {
		if (outFormat == HTML) {
			outStream << "<!DOCTYPE html>"<<endl<<"<meta charset='utf-8'>"<<endl
				<< "<title>BOSS Log</title>"<<endl<<"<style>"
				<< "body{font-family:Calibri,Arial,sans-serifs;}"
				<< "body.dark{color:white;background:black;}"
				<< "a.dark:link{color:#0AF;}"
				<< "a.dark:visited{color:#E000E0;}"
				<< "#filters{" << "background:#F5F5F5; width:252px; position:fixed; right:0; top:-32em; z-index:0; box-shadow: -1px 1px 1px 1px rgba(0,0,0,0.5);"
				<< "	transition: top 0.2s;"
				<< "	-moz-transition: top 0.2s;"
				<< "	-webkit-transition: top 0.2s;"
				<< "	-ms-transition: top 0.2s;"
				<< "	-o-transition: top 0.2s;" << "}"
				<< "#filters > li{margin:0;padding:0.2em 0.5em; white-space:nowrap;}"
				<< "h1{display:inline;font-size:2.4em;font-weight:700;margin-right:1em;}"
				<< "h2 + *{margin-bottom:3em;}"
				<< "h2{font-size:1.17em;display:table;cursor:pointer;}"
				<< "h2 > span{display:inline-block;position:relative;top:.05em; font-size:1.3em;width:.6em;margin-right:.1em;}"
				<< "ul{list-style:none;padding-left:0;}"
				<< "ul li{margin-left:0;margin-bottom:1em;}"
				<< "li ul{margin-top:.5em;padding-left:2.5em;margin-bottom:2em;}"
				<< "td{padding:0 .5em;vertical-align:top;}"
				<< "input[type='checkbox']{position:relative;top:.15em;margin-right:.5em;}"
				<< "blockquote{font-style:italic;}"
				<< ".error{background:red;color:#FFF;display:table;padding:0 4px;}"
				<< ".warn{background:orange;color:#FFF;display:table;padding:0 4px;}"
				<< ".success{color:green;}"
				<< ".version{color:#6394F8;margin-right:1em;padding:0 4px;}"
				<< ".ghosted{color:#888;margin-right:1em;padding:0 4px;}"
				<< ".crc{color:#BC8923;margin-right:1em;padding:0 4px;}"
				<< ".active{color:#0A0;margin-right:1em;padding:0 4px;}"
				<< ".tagPrefix{color:#CD5555;}"
				<< ".dirty{color:#960;}"
				<< ".message{color:gray;}"
				<< ".mod{margin-right:1em;}"
				<< ".tag{}"
				<< ".note{}"
				<< ".req{}"
				<< ".inc{}"
				<< ".submit{margin-right:1em;}"
				<< ".hidden {display:none;}"
				<< ".button {cursor:pointer;text-decoration:underline;}"
				<< "p.last {text-align:center;margin-bottom:0;}"
				<< ".t {color:green;margin-right:0.5em;}"
				<< ".c {color:red;margin-right:0.5em;}"
				<< ".t:after {content: '\\2713';}"
				<< ".c:after {content: '\\2717';}"
				<< "#mask{position:fixed;left:0px;top:0px;width:100%;height:100%;background:black;opacity:0.9;display:block;z-index:1;}"
				<< "#submitBox{padding:10px;background:white;display:none;z-index:2;position:fixed;top:0;width:430px;left:50%;margin-left:-220px;}"
				<< "#submitBox > h1{text-align:center;display:block;margin:0;}"
				<< "#submitBox p {margin-left:10px; margin-right:10px;}"
				<< "#link{width:400px;}"
				<< "#notes{height:10em;width:400px;}"
				<< "#plugin{display:table-cell;font-style:italic;}"
				<< "#pluginLabel{display:table-cell;padding-right:0.5em;}"
				<< "#output {display:none;width:400px;margin:2em auto;text-align:center;}"
				<< "#arrow {display:inline-block;font-size:1.5em;position:relative;top:3px;	transform: rotate(-90deg);   -moz-transform: rotate(-90deg);   -webkit-transform: rotate(-90deg);   -ms-transform: rotate(-90deg);   -o-transform: rotate(-90deg);}"
				<< "#arrow.rotated {position:relative;left:0.2em;	transform: rotate(90deg);	-moz-transform: rotate(90deg);	-webkit-transform: rotate(90deg);	-ms-transform: rotate(90deg);	-o-transform: rotate(90deg);}"
				<< "#menu > div { display:table-cell;padding:0.5em 1em; transition: background 0.2s;	-webkit-transition: background 0.2s;	-moz-transition: background 0.2s;	-ms-transition: background 0.2s;	-o-transition: background 0.2s;}"
				<< "#menu > div:hover {background:lightgrey;}"
				<< "#filtersButton {width:220px;}"
				<< "#filtersButton > span:first-child {display:inline-block;width:200px;}"
				<< "#menu {position:fixed;right:0;top:0;background:#F5F5F5;line-height:1em;cursor:pointer;z-index:1; box-shadow: -1px 1px 1px 1px rgba(0,0,0,0.5);}"
				<< "#filters.dark, #menu.dark, #submitBox.dark, #cssBox.dark, #browserBox.dark {background:#333;}"
				<< "#cssBox.visible, #submitBox.visible, #output.visible, #browserBox.visible {display:block;}"
				<< "#filters.visible {top:1.25em;}"
				<< "#cssBox, #browserBox{padding:20px;width:70%;background:white;display:none;z-index:2;position:fixed;top:-1em;left:14%;overflow:auto;}"
				<< "#cssBox > h1, #browserBox > h1 {display:block;text-align:center;margin:0;}"
				<< "input:invalid {background:#FF6347;}"
				<< "#cssBox p, #cssBox table, #browserBox p, #browserBox table, #browserBox h3 {margin-left:20px; margin-right:20px;}"
				<< "#cssBox thead {font-weight:bold;}"
				<< "#browserBox > p > span {display:inline-block;padding:5px 10px;}"
				<< "</style>"<<endl;
			outStream << "<h1>BOSS Log</h1>"
				<< "BOSS v" << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH) << " (" << gl_boss_release_date << ")."
				<< " &copy; 2009-2012 BOSS Development Team." << endl;
		} else
			outStream << endl << "BOSS Log" << endl
				<< "Copyright 2009-2012 BOSS Development Team" << endl
				<< "License: GNU General Public License v3.0" << endl
				<< "(http://www.gnu.org/licenses/gpl.html)" << endl
				<< "v" << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH) << " (" << gl_boss_release_date << ")" << endl;
	}

	void Outputter::PrintFooter() {
		if (outFormat == HTML) {
			outStream << endl 
				<< "<div id='submitBox'>" << endl
				<< "<h1>Submit Plugin</h1>" << endl
				<< "<hr>" << endl
				<< "<p><span id='pluginLabel'>Plugin:</span><span id='plugin'></span>" << endl
				<< "<form>" << endl
				<< "<p><label>Download Location:<br /><input type='url' required placeholder=\"A link to the plugin's download location.\" id='link'></label>" << endl
				<< "<p><label>Additional Notes:<br /><textarea id='notes' placeholder='Any additional information, such as recommended Bash Tags, load order suggestions, ITM/UDR counts and dirty CRCs, can be supplied here. If no download link is available, this information is crucial.'></textarea></label>" << endl
				<< "</form>" << endl
				<< "<div id='output'></div>" << endl
				<< "<hr>" << endl
				<< "<p class='last'><button type='button' onclick='submitPlugin()' autofocus>Submit</button>" << endl
				<< "<button type='button' onclick='hidePopupBox()'>Cancel</button>" << endl
				<< "</div>" << endl


				<< "<div id='cssBox'>" << endl
				<< "<h1>CSS Settings</h1>" << endl
				<< "<hr>" << endl
				<< "<p>Here you can customise the colours used by the dark colour scheme.<span id='colorPickerNote'> Colours must be specified using their lowercase hex codes.</span> Boxes left blank will use their default values, which are given by their placeholders." << endl
				<< "<table>" << endl
				<< "	<thead><tr><td>Element<td>Colour<td>Element<td>Colour" << endl
				<< "	<tbody>" << endl
				<< "		<tr><td>General Text<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='body.dark' data-property='color' placeholder='#ffffff'>" << endl
				<< "		<td>Errors<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.error.dark' data-property='background' placeholder='#ff0000'>" << endl
				<< "		<tr><td>Window Backgrounds<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='body.dark,#cssBox.dark,#submitBox.dark,#browserBox.dark' data-property='background' placeholder='#000000'>" << endl
				<< "		<td>Warnings<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.warn.dark' data-property='background' placeholder='#ffa500'>" << endl
				<< "		<tr><td>Links<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='a.dark:link' data-property='color' placeholder='#00aaff'>" << endl
				<< "		<td>Ghosted Labels<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.ghost.dark' data-property='color' placeholder='#888888'>" << endl
				<< "		<tr><td>Visited Links<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='a.dark:visited' data-property='color' placeholder='#e000e0'>" << endl
				<< "		<td>CRC Labels<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.crc.dark' data-property='color' placeholder='#bc8923'>" << endl
				<< "		<tr><td>Menu Background<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='#menu.dark,#filters.dark' data-property='background' placeholder='#333333'>" << endl
				<< "		<td>Active Labels<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.active.dark' data-property='color' placeholder='#00aa00'>" << endl
				<< "		<tr><td>Menu Button Hover<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='#menu.dark > div:hover' data-property='background' placeholder='#d3d3d3'>" << endl
				<< "		<td>Dirty Messages<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.dirty.dark' data-property='color' placeholder='#996600'>" << endl
				<< "		<tr><td>Bash Tag Suggestion Prefixes<td><input type=color pattern='#[a-f0-9]{6}' title='Colours must be specified using lowercase hex codes.' data-selector='.tagPrefix.dark' data-property='color' placeholder='#cd5555'><td><td>" << endl
				<< "</table>" << endl
				<< "<p id='backupCSSNote'>You can <span class='button' onclick='backupCSS(this)'>back up</span> your CSS settings to avoid losing them when you <span id='loseSettingsClose'>close the BOSS Log</span><span id='loseSettingsCacheClear'>clear your browser's cache</span>. Backup files are given nonsensical names by your browser, but you can rename them to whatever you want. Drag 'n' drop the backup file into this panel to restore your settings again." << endl
				<< "<hr>" << endl
				<< "<p class='last'><button type='button' onclick='applyCSS()' autofocus>Apply</button>" << endl
				<< "<button type='button' onclick='hideCSSBox()'>Cancel</button>" << endl
				<< "</div>" << endl

				<< "<div id='browserBox'>" << endl
				<< "<h1>Browser Compatability</h1>" << endl
				<< "<hr>" << endl
				<< "<p>The BOSS Log's more advanced features require some of the latest web technologies, for which browser support varies. Here's what your browser supports:" << endl
				<< "<h3>Functionality</h3>" << endl
				<< "<table>" << endl
				<< "	<tbody>" << endl
				<< "		<tr><td id='pluginSubmitSupport'><td>In-Log Plugin Submission<td>Allows unrecognised plugins to be submitted straight from the BOSS Log." << endl
				<< "		<tr><td id='cssBackupSupport'><td>Backup/Restore of CSS Settings<td>Allows backup and restore of your custom CSS settings." << endl
				<< "		<tr><td id='memorySupport'><td>Settings Memory<td>Allows the BOSS Log to restore the configuration of CSS settings, filters and collapsed sections last used, and to prevent this panel being displayed, whenever the BOSS Log is opened." << endl
				<< "</table>" << endl
				<< "<h3>Appearance</h3>" << endl
				<< "<p><span><span id='opacitySupport'></span>Opacity</span>" << endl
				<< "<span><span id='shadowsSupport'></span>Shadows</span>" << endl
				<< "<span><span id='transitionsSupport'></span>Transitions</span>" << endl
				<< "<span><span id='transformsSupport'></span>Transforms</span>" << endl
				<< "<span><span id='colorSupport'></span>Colour Picker Input</span>" << endl
				<< "<span><span id='placeholderSupport'></span>Input Placeholders</span>" << endl
				<< "<span><span id='validationSupport'></span>Form Validation</span>" << endl
				<< "<p><label><input type=checkbox>Do not display this panel again for this browser.</label>" << endl
				<< "<hr>" << endl
				<< "<p class='last'><button type='button' onclick='closeBrowserBox()' autofocus>OK</button>" << endl
				<< "</div>" << endl


				<< "<script>" << endl
				<< "'use strict';" << endl
				<< "var hm=0,hp=0,hpe=document.getElementById('hp'),hme=document.getElementById('hm'),url = 'http://www.darkcreations.org/bugzilla/jsonrpc.cgi',form = document.getElementsByTagName('form')[0],output = document.getElementById('output');" << endl

				<< "var hideBrowserBox = false, supportsPluginSubmit = false, suppportsStateMemory = false, supportsCSSBackupRestore = false;"<<endl
				<< "function isSupported (propName) {"<<endl
				<< "	return typeof document.body.style[propName] !== 'undefined';"<<endl
				<< "}"<<endl
				<< "function closeBrowserBox(){"<<endl
				<< "	var mask=document.getElementById('mask');"<<endl
				<< "	var box=document.getElementById('browserBox');"<<endl
				<< "	box.className = box.className.replace('visible',''); "<<endl
				<< "	if (document.getElementById('browserBox').querySelector('input[type=checkbox]').checked && suppportsStateMemory) {"<<endl
				<< "		localStorage.setItem('hideBrowserBox','true');"<<endl
				<< "	}"<<endl
				<< "	if(mask!=null){"<<endl
				<< "		var parent=mask.parentNode;"<<endl
				<< "		parent.removeChild(mask);"<<endl
				<< "	}"<<endl
				<< "}"<<endl

				<< "function backupCSS(a){"<<endl
				<< "	if (typeof(JSON) === 'object' && typeof(JSON.parse) === 'function') {"<<endl
				<< "		var a = document.getElementById('cssBox').getElementsByTagName('input');"<<endl
				<< "		var json = '{\"colors\":{'"<<endl
				<< "		for(var i=0,z=a.length;i<z;i++){"<<endl
				<< "			json += '\"' + a[i].getAttribute('data-selector') + '\":\"' + a[i].getAttribute('data-property') + ':' + a[i].value + '\",'"<<endl
				<< "		}"<<endl
				<< "		json = json.substr(0,json.length-1);"<<endl
				<< "		json += '}}';"<<endl
				<< "		var uriContent = 'data:json/settings-backup,' + encodeURIComponent(json);"<<endl
				<< "		location.href = uriContent;"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function handleFileSelect(evt) {"<<endl
				<< "	evt.stopPropagation();"<<endl
				<< "	evt.preventDefault();"<<endl
				<< "	var files = evt.dataTransfer.files; // FileList object"<<endl
				<< "	var a = document.getElementById('cssBox').getElementsByTagName('input');"<<endl
				<< "	for (var i = 0, f; f = files[i]; i++) {"<<endl
				<< "		var reader = new FileReader();"<<endl
				<< "		reader.onload = (function(theFile) {"<<endl
				<< "			return function(e) {"<<endl
				<< "				try {"<<endl
				<< "					var json = JSON.parse(e.target.result).colors;"<<endl
				<< "					for (var key in json) {"<<endl
				<< "						if (json.hasOwnProperty(key) && json[key].length != 0) {"<<endl
				<< "							for (var i=0, z=a.length; i < z; i++) {"<<endl
				<< "								if (a[i].getAttribute('data-selector') == key) {"<<endl
				<< "									a[i].value = json[key].split(':').pop();"<<endl
				<< "								}"<<endl
				<< "							}"<<endl
				<< "						}"<<endl
				<< "					}"<<endl
				<< "				} catch (e) {"<<endl
				<< "					alert(e);"<<endl
				<< "				}"<<endl
				<< "			};"<<endl
				<< "		})(f);"<<endl
				<< "		reader.readAsText(f);"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function handleDragOver(evt) {"<<endl
				<< "	evt.stopPropagation();"<<endl
				<< "	evt.preventDefault();"<<endl
				<< "	evt.dataTransfer.dropEffect = 'copy';"<<endl
				<< "}"<<endl


				<< "function showCSSBox(){"<<endl
				<< "	if ('localStorage' in window && window['localStorage'] !== null && window['localStorage'] !== undefined) {"<<endl
				<< "		var a = document.getElementById('cssBox').getElementsByTagName('input');"<<endl
				<< "		var len = localStorage.length;"<<endl
				<< "		for (var i=0, z=a.length; i < z; i++) {"<<endl
				<< "			var s = localStorage.getItem(a[i].getAttribute('data-selector'));"<<endl
				<< "			if (s != null) {"<<endl
				<< "				a[i].value = s.split(':').pop();"<<endl
				<< "			}"<<endl
				<< "		}"<<endl
				<< "	}"<<endl
				<< "	if(document.getElementById('mask')==null){"<<endl
				<< "		var mask=document.createElement('div');"<<endl
				<< "		mask.id='mask';"<<endl
				<< "		document.body.appendChild(mask);"<<endl
				<< "	}"<<endl
				<< "	document.getElementById('cssBox').className += ' visible';"<<endl
				<< "}"<<endl
				<< "function hideCSSBox(){"<<endl
				<< "	var mask=document.getElementById('mask');"<<endl
				<< "	var box=document.getElementById('cssBox');"<<endl
				<< "	box.className = box.className.replace('visible',''); "<<endl
				<< "	if(mask!=null){"<<endl
				<< "		var parent=mask.parentNode;"<<endl
				<< "		parent.removeChild(mask);"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function toggleFilters(){"<<endl
				<< "	var filters = document.getElementById('filters');"<<endl
				<< "	var arrow = document.getElementById('arrow');"<<endl
				<< "	if (filters.className.indexOf('visible') == -1) {"<<endl
				<< "		filters.className += ' visible';"<<endl
				<< "		arrow.className += ' rotated';"<<endl
				<< "	} else {"<<endl
				<< "		filters.className = filters.className.replace('visible',''); "<<endl
				<< "		arrow.className = arrow.className.replace('rotated','');"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function showPopupBox(plugin){"<<endl
				<< "	if(document.getElementById('mask')==null){"<<endl
				<< "		var mask=document.createElement('div');"<<endl
				<< "		mask.id='mask';"<<endl
				<< "		document.body.appendChild(mask);"<<endl
				<< "	}"<<endl
				<< "	document.getElementById('plugin').innerHTML=plugin;"<<endl
				<< "	document.getElementById('submitBox').className += ' visible';"<<endl
				<< "}"<<endl
				<< "function hidePopupBox(){"<<endl
				<< "	var mask=document.getElementById('mask');"<<endl
				<< "	var box=document.getElementById('submitBox');"<<endl
				<< "	box.className = box.className.replace('visible',''); "<<endl
				<< "	document.getElementById('notes').value='';"<<endl
				<< "	document.getElementById('link').value='';"<<endl
				<< "	output.innerHTML='';"<<endl
				<< "	output.className = output.className.replace('visible',''); "<<endl
				<< "	form.className = form.className.replace('hidden',''); "<<endl
				<< "	var button = document.querySelector('#submitBox > p.last > button:first-child');"<<endl
				<< "	button.className = button.className.replace('hidden','');"<<endl
				<< "	if(mask!=null){"<<endl
				<< "		var parent=mask.parentNode;"<<endl
				<< "		parent.removeChild(mask);"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function outputText(text, flag) {"<<endl
				<< "	if (form.className.indexOf('hidden') == -1) {"<<endl
				<< "		form.className += ' hidden';"<<endl
				<< "	}"<<endl
				<< "	if (output.className.indexOf('visible') == -1) {"<<endl
				<< "		output.className += ' visible';"<<endl
				<< "	}"<<endl
				<< "	var button = document.querySelector('#submitBox > p.last > button:first-child');"<<endl
				<< "	if (button.className.indexOf('hidden') == -1) {"<<endl
				<< "		button.className += ' hidden';"<<endl
				<< "	}"<<endl
				<< "	if (output.innerHTML.length != 0) {"<<endl
				<< "		output.innerHTML += '<br />';"<<endl
				<< "	}"<<endl
				<< "	if (flag == -1) {"<<endl
				<< "		text = \"<span style='color:red;'>\" + text + \"</span>\";"<<endl
				<< "	} else if (flag == 1) {"<<endl
				<< "		text = \"<span class='success'>\" + text + \"</span>\";"<<endl
				<< "	}"<<endl
				<< "	output.innerHTML += text;"<<endl
				<< "}"<<endl
				<< "function toggleSectionDisplay(h){"<<endl
				<< "	if(h.nextSibling.className.indexOf('hidden') != -1){"<<endl
				<< "		h.nextSibling.className = h.nextSibling.className.replace('hidden','');"<<endl
				<< "		saveSectionState(h, false);"<<endl
				<< "		h.firstChild.innerHTML='&#x2212;'"<<endl
				<< "	}else{"<<endl
				<< "		h.nextSibling.className += ' hidden';"<<endl
				<< "		saveSectionState(h, true);"<<endl
				<< "		h.firstChild.innerHTML='+'"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function swapColorScheme(b){"<<endl
				<< "	saveFilterState(b);"<<endl
				<< "	var a = document.getElementById('cssBox').getElementsByTagName('input');"<<endl
				<< "	if(b.checked){"<<endl
				<< "		for(var i=0,z=a.length;i<z;i++){"<<endl
				<< "			var s = a[i].getAttribute('data-selector').replace(/\\.dark/g,'');"<<endl
				<< "			var c = document.querySelectorAll(s);"<<endl
				<< "			for(var j=0,d=c.length;j<d;j++){"<<endl
				<< "				c[j].className += ' dark';"<<endl
				<< "			}"<<endl
				<< "		}"<<endl
				<< "	}else{"<<endl
				<< "		for(var i=0,z=a.length;i<z;i++){"<<endl
				<< "			var c = document.querySelectorAll(a[i].getAttribute('data-selector'));"<<endl
				<< "			for(var j=0,d=c.length;j<d;j++){"<<endl
				<< "				c[j].className = c[j].className.replace(' dark','');"<<endl
				<< "			}"<<endl
				<< "		}"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function toggleDisplayCSS(b,s){"<<endl
				<< "	saveFilterState(b);"<<endl
				<< "	var e = document.querySelectorAll(s);"<<endl
				<< "	if(b.checked){"<<endl
				<< "		for(var i=0,z=e.length;i<z;i++){"<<endl
				<< "			e[i].className += ' hidden';"<<endl
				<< "		}"<<endl
				<< "	}else{"<<endl
				<< "		for(var i=0,z=e.length;i<z;i++){"<<endl
				<< "			e[i].className = e[i].className.replace(' hidden','');"<<endl
				<< "		}"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function applyCSS(){"<<endl
				<< "	var a = document.getElementById('cssBox').getElementsByTagName('input');"<<endl
				<< "	if (document.styleSheets[0].cssRules) {"<<endl
				<< "		var d = document.styleSheets[0];"<<endl
				<< "		for(var i=0,z=a.length;i<z;i++){"<<endl
				<< "			//Set rule."<<endl
				<< "			var css = a[i].getAttribute('data-property') + ':' + a[i].value;"<<endl
				<< "			//Store rule."<<endl
				<< "			if ('localStorage' in window && window['localStorage'] !== null && window['localStorage'] !== undefined) {"<<endl
				<< "				try {"<<endl
				<< "					localStorage.setItem(a[i].getAttribute('data-selector'), css);"<<endl
				<< "				} catch (e) {"<<endl
				<< "					if (e == QUOTA_EXCEEDED_ERR) {"<<endl
				<< "						alert('Web storage quota for this document has been exceeded. Please empty your browser\\'s cache. Note that this will delete all locally stored data.');"<<endl
				<< "					}"<<endl
				<< "				}"<<endl
				<< "			}"<<endl
				<< "			if (a[i].value.length == 0) {"<<endl
				<< "				css += a[i].placeholder;"<<endl
				<< "			}"<<endl
				<< "			var rule = a[i].getAttribute('data-selector') + '{' + css + '}';"<<endl
				<< "			d.insertRule(rule, d.cssRules.length);"<<endl
				<< "		}"<<endl
				<< "	} else if (document.styleSheets[0].rules) {  //IE8"<<endl
				<< "		var d = document.styleSheets[0].rules;"<<endl
				<< "		for(var i=0,z=a.length;i<z;i++){"<<endl
				<< "			var css = a[i].getAttribute('data-property') + ':' + a[i].value;"<<endl
				<< "			d.addRule(a[i].getAttribute('data-selector'), css, -1);"<<endl
				<< "		}"<<endl
				<< "	}"<<endl
				<< "	hideCSSBox();"<<endl
				<< "}"<<endl

				<< "function saveFilterState(filter) {"<<endl
				<< "	if ('localStorage' in window && window['localStorage'] !== null && window['localStorage'] !== undefined) {"<<endl
				<< "		if (filter.checked) {  //Add to local storage."<<endl
				<< "			try {"<<endl
				<< "				localStorage.setItem(filter.id, true);"<<endl
				<< "			} catch (e) {"<<endl
				<< "				if (e == QUOTA_EXCEEDED_ERR) {"<<endl
				<< "					alert('Web storage quota for this document has been exceeded. Please empty your browser\\'s cache. Note that this will delete all locally stored data.');"<<endl
				<< "				}"<<endl
				<< "			}"<<endl
				<< "		} else {"<<endl
				<< "			localStorage.removeItem(filter.id);"<<endl
				<< "		}"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function saveSectionState(section, isCollapsed) {"<<endl
				<< "	if ('localStorage' in window && window['localStorage'] !== null && window['localStorage'] !== undefined) {"<<endl
				<< "		if (isCollapsed) {  //Add to local storage."<<endl
				<< "			try {"<<endl
				<< "			localStorage.setItem(section.id, true);"<<endl
				<< "			} catch (e) {"<<endl
				<< "				if (e == QUOTA_EXCEEDED_ERR) {"<<endl
				<< "					alert('Web storage quota for this document has been exceeded. Please empty your browser\\'s cache. Note that this will delete all locally stored data.');"<<endl
				<< "				}"<<endl
				<< "			}"<<endl
				<< "		} else {"<<endl
				<< "			localStorage.removeItem(section.id);"<<endl
				<< "		}"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function isResponseOK(text) {" << endl
				<< "	return (JSON.parse(text).error == null);" << endl
				<< "}" << endl
				<< "function submitPlugin() {" << endl
				<< "	var desc = document.getElementById('link').value;" << endl
				<< "	if (desc.length != 0) {" << endl
				<< "		desc += '\\n\\n';" << endl
				<< "	}" << endl
				<< "	desc += document.getElementById('notes').value;" << endl
				<< "	if (desc.length == 0) {" << endl
				<< "		outputText('Please supply at least a link or some notes.', -1);" << endl
				<< "		return;" << endl
				<< "	}" << endl
				<< "	try {" << endl
				<< "		var xhr = new XMLHttpRequest();" << endl
				<< "		getBugId(document.getElementById('plugin').innerHTML, desc, xhr);" << endl
				<< "	} catch(err) {" << endl
				<< "		outputText('Exception occurred: ' + err.message, -1);" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function getBugId(plugin, desc, xhr) {" << endl
				<< "    var request = {" << endl
				<< "        \"method\":\"Bug.search\"," << endl
				<< "        \"params\":[{" << endl
 				<< "           \"Bugzilla_login\":\"bossguest@darkcreations.org\"," << endl
				<< "            \"Bugzilla_password\":\"bosspassword\"," << endl
 				<< "           \"product\":\"BOSS\"," << endl
				<< "            \"component\":\"" << GetGameString(gl_current_game) << "\"," << endl
				<< "            \"summary\":plugin" << endl
 				<< "       }]," << endl
				<< "        \"id\":1" << endl
				<< "    }" << endl
				<< "	outputText('Checking for existing submission...', 0);" << endl
 				<< "	xhr.open('POST', url, true);" << endl
 				<< "	xhr.setRequestHeader('Content-Type', 'application/json');" << endl
 				<< "	xhr.onerror = error;" << endl
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
				<< "			outputText('Error: Existing submission check failed!', -1);" << endl
				<< "		}" << endl
				<< "	}" << endl
				<< "	xhr.send(JSON.stringify(request));" << endl
				<< "}" << endl
				<< "function addBugComment(id, comment, xhr) {" << endl
				<< "	var request = {" << endl
				<< "		\"method\":\"Bug.add_comment\"," << endl
				<< "        \"params\":[{" << endl
				<< "			\"Bugzilla_login\":\"bossguest@darkcreations.org\"," << endl
				<< "			\"Bugzilla_password\":\"bosspassword\"," << endl
				<< "			\"id\":id," << endl
				<< "			\"comment\":comment" << endl
				<< "		}]," << endl
				<< "		\"id\":2" << endl
 				<< "	}" << endl
				<< "	outputText('Previous submission found, updating with supplied details...', 0);" << endl
 				<< "	xhr.open('POST', url, true);" << endl
 				<< "	xhr.setRequestHeader('Content-type', 'application/json');" << endl
 				<< "	xhr.onerror = error;" << endl
 				<< "	xhr.onload = function() {" << endl
				<< "	   if (xhr.status == 200 && isResponseOK(xhr.responseText)) {" << endl
				<< "	       outputText('Plugin submission updated!', 1);" << endl
				<< "	   } else {" << endl
				<< "	       outputText('Error: Plugin submission could not be updated.', -1);" << endl
				<< "	   }" << endl
				<< "	}" << endl
				<< "	xhr.send(JSON.stringify(request));" << endl
				<< "}" << endl
				<< "function addBug(summary, description, xhr) {" << endl
				<< "    var request = {" << endl
				<< "		\"method\":\"Bug.create\"," << endl
 				<< "       \"params\":[{" << endl
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
				<< "    }" << endl
				<< "	outputText('No previous submission found, creating new submission...', 0);" << endl
				<< "    xhr.open('POST', url, true);" << endl
				<< "    xhr.setRequestHeader('Content-type', 'application/json');" << endl
				<< "    xhr.onerror = error;" << endl
				<< "    xhr.onload = function() {" << endl
				<< "	   if (xhr.status == 200 && isResponseOK(xhr.responseText)) {" << endl
				<< "	       outputText('Plugin submitted!', 1);" << endl
				<< "	   } else {" << endl
				<< "	       outputText('Error: Plugin could not be submitted.', -1);" << endl
				<< "	   }" << endl
				<< "	}" << endl
				<< "	xhr.send(JSON.stringify(request));" << endl
				<< "}" << endl
				<< "function error() {" << endl
				<< "	outputText('Error: Data transfer failed.', -1);" << endl
				<< "}" << endl
				<< "function toggleRuleListWarnings(b){saveFilterState(b);var u=document.getElementById('userlistMessages');if(u!=null){u=u.childNodes;for(var i=0,z=u.length;i<z;i++){if(u[i].className=='warn'){if(b.checked){u[i].style.display='none';hm++}else if(u[i].style.display=='none'){u[i].style.display='table';hm--}}}}hme.innerHTML=hm}" << endl
				<< "function toggleMessages(filter){"<<endl
				<< "	if(filter!=null){saveFilterState(filter);}"<<endl
				<< "	var m=document.getElementById('recognised').childNodes,b9=document.getElementById('b9').checked,b10=document.getElementById('b10').checked,b11=document.getElementById('b11').checked,b12=document.getElementById('b12').checked,b13=document.getElementById('b13').checked,b14=document.getElementById('b14').checked;"<<endl
				<< "	for(var i=0,z=m.length;i<z;i++){"<<endl
				<< "		var ac=false,g=false,n=true,c=true,a=m[i].getElementsByTagName('span'),b=null;"<<endl
				<< "		for(var j=0,y=a.length;j<y;j++){"<<endl
				<< "			if(a[j].className=='ghosted'){"<<endl
				<< "				g=true;"<<endl
				<< "			}else if(a[j].className=='active'){"<<endl
				<< "				ac=true;"<<endl
				<< "			}"<<endl
				<< "		}"<<endl
				<< "		a=m[i].getElementsByTagName('li');"<<endl
				<< "		for(var j=0,y=a.length;j<y;j++){"<<endl
				<< "			if(!b9&&a[j].style.display=='none'){"<<endl
				<< "				a[j].style.display='table';"<<endl
				<< "				hm--"<<endl
				<< "			}"<<endl
				<< "			if(a[j].className=='note'){"<<endl
				<< "				b=b10"<<endl
				<< "			}else if(a[j].className=='tag'){"<<endl
				<< "				b=b11"<<endl
				<< "			}else if(a[j].className=='req'){"<<endl
				<< "				b=b12"<<endl
				<< "			}else if(a[j].className=='inc'){"<<endl
				<< "				b=b13"<<endl
				<< "			}else if(a[j].className=='dirty'){"<<endl
				<< "				c=false;"<<endl
				<< "				if(a[j].innerHTML.substr(0,35)=='Contains dirty edits: Do not clean.'){"<<endl
				<< "					b=b14"<<endl
				<< "				}"<<endl
				<< "			}"<<endl
				<< "			if(b!=null){"<<endl
				<< "				if(b&&a[j].style.display!='none'){"<<endl
				<< "					a[j].style.display='none';"<<endl
				<< "					hm++"<<endl
				<< "				}else if(!b&&a[j].style.display=='none'){"<<endl
				<< "					a[j].style.display='table';"<<endl
				<< "					hm--"<<endl
				<< "				}"<<endl
				<< "			}"<<endl
				<< "			if(b9&&a[j].style.display!='none'){"<<endl
				<< "				a[j].style.display='none';"<<endl
				<< "				hm++"<<endl
				<< "			}"<<endl
				<< "			if(a[j].style.display!='none'){"<<endl
				<< "				n=false"<<endl
				<< "			}"<<endl
				<< "		}"<<endl
				<< "		if(!((document.getElementById('b6').checked&&n)||(document.getElementById('b7').checked&&g)||(document.getElementById('b8').checked&&c)||(document.getElementById('b15').checked&&!ac))&&m[i].style.display=='none'){"<<endl
				<< "			m[i].style.display='block';"<<endl
				<< "			hp--"<<endl
				<< "		}else if(((document.getElementById('b6').checked&&n)||(document.getElementById('b7').checked&&g)||(document.getElementById('b8').checked&&c)||(document.getElementById('b15').checked&&!ac))&&m[i].style.display!='none'){"<<endl
				<< "			m[i].style.display='none';"<<endl
				<< "			hp++"<<endl
				<< "		}"<<endl
				<< "	}"<<endl
				<< "	hme.innerHTML=hm;"<<endl
				<< "	hpe.innerHTML=hp"<<endl
				<< "}"<<endl
				<< "function initialSetup() {"<<endl
				<< "	var supportsXhr2CORS = ('withCredentials' in new XMLHttpRequest);"<<endl
				<< "	var supportsJSON = (typeof(JSON) === 'object' && typeof(JSON.parse) === 'function');"<<endl
				<< "	var supportsFileDnD = (window.File && window.FileReader && window.FileList && window.Blob && 'draggable' in document.createElement('span'));"<<endl
				<< "	supportsPluginSubmit = (supportsXhr2CORS && supportsJSON);"<<endl
				<< "	supportsCSSBackupRestore = (supportsFileDnD && supportsJSON);"<<endl
				<< "	try {"<<endl
				<< "		suppportsStateMemory = ('localStorage' in window && window['localStorage'] !== null && window['localStorage'] !== undefined);"<<endl
				<< "	} catch (e) {}"<<endl
				<< "	if (suppportsStateMemory) {  //Local storage read on page load."<<endl
				<< "		var len = localStorage.length;"<<endl
				<< "		for (var i=0; i < len; i++) {"<<endl
				<< "			if (localStorage.key(i) == 'hideBrowserBox') {"<<endl
				<< "				hideBrowserBox = true;"<<endl
				<< "			} else {"<<endl
				<< "				var elem = document.getElementById(localStorage.key(i));"<<endl
				<< "				if (elem != null) {"<<endl
				<< "					if ('defaultChecked' in elem) {  //It's a checkbox."<<endl
				<< "						elem.checked = 'true';"<<endl
				<< "					} else {  //It's a section."<<endl
				<< "						toggleSectionDisplay(elem);"<<endl
				<< "					}"<<endl
				<< "				} else {  //It's a CSS selector."<<endl
				<< "					if (document.styleSheets[0].cssRules) {"<<endl
				<< "						var k = localStorage.key(i);"<<endl
				<< "						var d = document.styleSheets[0];"<<endl
				<< "						d.insertRule(k + '{' + localStorage.getItem(k) + '}', d.cssRules.length);"<<endl
				<< "					}"<<endl
				<< "				}"<<endl
				<< "			}"<<endl
				<< "		}"<<endl
				<< "	}"<<endl
				<< "	swapColorScheme(document.getElementById('b1'));"<<endl
				<< "	toggleRuleListWarnings(document.getElementById('b2'));"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b3'),'.version','inline');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b4'),'.ghosted','inline');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b5'),'.crc','inline');"<<endl
				<< "	toggleMessages(null);"<<endl
				<< "	if (!supportsPluginSubmit) { //Disable submission buttons."<<endl
				<< "		var buttons = document.getElementById('unrecognised').nextSibling.getElementsByTagName('button');"<<endl
				<< "		for (var i=0, len=buttons.length; i < len; i++) {"<<endl
				<< "			buttons[i].style.display = 'none';"<<endl
				<< "		}"<<endl
				<< "	}"<<endl
				<< "	if (supportsCSSBackupRestore) {"<<endl
				<< "		var dropZone = document.getElementById('cssBox');"<<endl
				<< "		dropZone.addEventListener('dragover', handleDragOver, false);"<<endl
				<< "		dropZone.addEventListener('drop', handleFileSelect, false);"<<endl
				<< "		if (suppportsStateMemory) {"<<endl
				<< "			document.getElementById('loseSettingsClose').className += ' hidden';"<<endl
				<< "		} else {"<<endl
				<< "			document.getElementById('loseSettingsCacheClear').className += ' hidden';"<<endl
				<< "		}"<<endl
				<< "	} else {"<<endl
				<< "		document.getElementById('backupCSSNote').className += ' hidden';"<<endl
				<< "	}"<<endl
				<< "	//Display browser compatibility panel."<<endl
				<< "	if (!hideBrowserBox) {  //Set feedback."<<endl
				<< "		if (supportsPluginSubmit) {"<<endl
				<< "			document.getElementById('pluginSubmitSupport').className = 't';"<<endl
				<< "		} else {"<<endl
				<< "			document.getElementById('pluginSubmitSupport').className = 'c';"<<endl
				<< "		}"<<endl
				<< "		if (supportsCSSBackupRestore) {"<<endl
				<< "			document.getElementById('cssBackupSupport').className = 't';"<<endl
				<< "		} else {"<<endl
				<< "			document.getElementById('cssBackupSupport').className = 'c';"<<endl
				<< "		}"<<endl
				<< "		if (suppportsStateMemory) {"<<endl
				<< "			document.getElementById('memorySupport').className = 't';"<<endl
				<< "		} else {"<<endl
				<< "			document.getElementById('browserBox').querySelector('label').className += ' hidden';"<<endl
				<< "			document.getElementById('memorySupport').className = 'c';"<<endl
				<< "		}"<<endl
				<< "		if (isSupported('opacity')) {"<<endl
				<< "			document.getElementById('opacitySupport').className = 't';"<<endl
				<< "		} else {"<<endl
				<< "			document.getElementById('opacitySupport').className = 'c';"<<endl
				<< "		}"<<endl
				<< "		if (isSupported('boxShadow')) {"<<endl
				<< "			document.getElementById('shadowsSupport').className = 't';"<<endl
				<< "		} else {"<<endl
				<< "			document.getElementById('shadowsSupport').className = 'c';"<<endl
				<< "		}"<<endl
				<< "		if (isSupported('transition') || isSupported('MozTransition') || isSupported('webkitTransition') || isSupported('OTransition') || isSupported('msTransition')) {"<<endl
				<< "			document.getElementById('transitionsSupport').className = 't';"<<endl
				<< "		} else {"<<endl
				<< "			document.getElementById('transitionsSupport').className = 'c';"<<endl
				<< "		}"<<endl
				<< "		if (isSupported('transform') || isSupported('MozTransform') || isSupported('webkitTransform') || isSupported('OTransform') || isSupported('msTransform')) {"<<endl
				<< "			document.getElementById('transformsSupport').className = 't';"<<endl
				<< "		} else {"<<endl
				<< "			document.getElementById('transformsSupport').className = 'c';"<<endl
				<< "		}"<<endl
				<< "		var i = document.createElement('input');"<<endl
				<< "		i.setAttribute('type', 'color');"<<endl
				<< "		if (i.type !== 'text') {"<<endl
				<< "			document.getElementById('colorSupport').className = 't';"<<endl
				<< "			document.getElementById('colorPickerNote').className += ' hidden';"<<endl
				<< "		} else {"<<endl
				<< "			document.getElementById('colorSupport').className = 'c';"<<endl
				<< "		}"<<endl
				<< "		if ('placeholder' in document.createElement('input')) {"<<endl
				<< "			document.getElementById('placeholderSupport').className = 't';"<<endl
				<< "		} else {"<<endl
				<< "			document.getElementById('placeholderSupport').className = 'c';"<<endl
				<< "		}"<<endl
				<< "		var i = document.createElement('input');"<<endl
				<< "		i.setAttribute('type', 'url');"<<endl
				<< "		if (i.type !== 'text') {"<<endl
				<< "			document.getElementById('validationSupport').className = 't';"<<endl
				<< "		} else {"<<endl
				<< "			document.getElementById('validationSupport').className = 'c';"<<endl
				<< "		}"<<endl
				<< "		if(document.getElementById('mask')==null){"<<endl
				<< "			var mask=document.createElement('div');"<<endl
				<< "			mask.id='mask';"<<endl
				<< "			document.body.appendChild(mask);"<<endl
				<< "		}"<<endl
				<< "		document.getElementById('browserBox').className += ' visible';"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function DomReady(fn){"<<endl
				<< "	if(document.addEventListener){"<<endl
				<< "		document.addEventListener('DOMContentLoaded', fn, false)"<<endl
				<< "	}else{"<<endl
				<< "		document.onreadystatechange = function(){readyState(fn)}"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function readyState(fn){"<<endl
				<< "	if(document.readyState == 'interactive'){"<<endl
				<< "		fn()"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "window.onDomReady = DomReady;"<<endl
				<< "window.onDomReady(initialSetup);"<<endl
				<< "</script>"<<endl;
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
		case DIV_OPEN:
			if (outFormat == HTML)
				outStream << "<div>";
			else
				outStream << endl;
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
		case LIST_ID_RECOGNISED_OPEN:
			if (outFormat == HTML)
				outStream << "<ul id='recognised'>";
			break;
		case LIST_ID_USERLIST_MESSAGES_OPEN:
			if (outFormat == HTML)
				outStream << "<ul id='userlistMessages'>";
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
		case HEADING_ID_SUMMARY_OPEN:
			if (outFormat == HTML)
				outStream << "<h2 id='summary' onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>";
			else
				outStream << endl << endl << "======================================" << endl;
			break;
		case HEADING_ID_GENERAL_OPEN:
			if (outFormat == HTML)
				outStream << "<h2 id='general' onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>";
			else
				outStream << endl << endl << "======================================" << endl;
			break;
		case HEADING_ID_USERLIST_OPEN:
			if (outFormat == HTML)
				outStream << "<h2 id='userlist' onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>";
			else
				outStream << endl << endl << "======================================" << endl;
			break;
		case HEADING_ID_SE_OPEN:
			if (outFormat == HTML)
				outStream << "<h2 id='se' onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>";
			else
				outStream << endl << endl << "======================================" << endl;
			break;
		case HEADING_ID_RECOGNISEDSEC_OPEN:
			if (outFormat == HTML)
				outStream << "<h2 id='recognisedSec' onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>";
			else
				outStream << endl << endl << "======================================" << endl;
			break;
		case HEADING_ID_UNRECOGNISED_OPEN:
			if (outFormat == HTML)
				outStream << "<h2 id='unrecognised' onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>";
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
		case SPAN_CLASS_GHOSTED_OPEN:
			if (outFormat == HTML)
				outStream << "<span class='ghosted'>&nbsp;";
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
		case BUTTON_SUBMIT_PLUGIN:
			if (outFormat == HTML)
				outStream << "<button class='submit' onclick='showPopupBox(this.nextSibling.innerHTML)'>Submit Plugin</button>";
			break;
		default:
			break;
		}
		return *this;
	}

	Outputter& Outputter::operator<< (const langString l) {
		switch(l) {
		case LOG_UseDarkColourScheme:
			if (gl_language == ENGLISH)
				outStream << "";
			else if (gl_language == SPANISH)
				outStream << "";
			else if (gl_language == GERMAN)
				outStream << "";
			else if (gl_language == RUSSIAN)
				outStream << "";
			break;
		default:
			break;
		}
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