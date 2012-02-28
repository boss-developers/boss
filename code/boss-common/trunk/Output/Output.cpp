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

	//Default filter options.
	BOSS_COMMON bool UseDarkColourScheme    = false;
	BOSS_COMMON bool HideVersionNumbers     = false;
	BOSS_COMMON bool HideGhostedLabel       = false;
	BOSS_COMMON bool HideActiveLabel		= false;
	BOSS_COMMON bool HideChecksums          = false;
	BOSS_COMMON bool HideMessagelessMods    = false;
	BOSS_COMMON bool HideGhostedMods        = false;
	BOSS_COMMON bool HideCleanMods			= false;
	BOSS_COMMON bool HideRuleWarnings       = false;
	BOSS_COMMON bool HideAllModMessages     = false;
	BOSS_COMMON bool HideNotes              = false;
	BOSS_COMMON bool HideBashTagSuggestions = false;
	BOSS_COMMON bool HideRequirements       = false;
	BOSS_COMMON bool HideIncompatibilities  = false;
	BOSS_COMMON bool HideDoNotCleanMessages	= false;
	BOSS_COMMON bool HideInactivePlugins	= false;

	//Default CSS.
	BOSS_COMMON string CSSBody				= "font-family:Calibri,Arial,sans-serifs;";
	BOSS_COMMON string CSSDarkBody			= "color:white;background:black;";
	BOSS_COMMON string CSSDarkLink			= "color:#0AF;";
	BOSS_COMMON string CSSDarkLinkVisited	= "color:#E000E0;";
	BOSS_COMMON string CSSFilters			= "border:1px gray dashed;background:#F5F5F5;padding:.3em;display:table;";
	BOSS_COMMON string CSSFiltersList		= "display:inline-block;padding:.2em .5em;white-space:nowrap;margin:0;width:200px;";
	BOSS_COMMON string CSSDarkFilters		= "border:1px gray dashed;padding:.3em;display:table;background:#333;";
	BOSS_COMMON string CSSTitle				= "font-size:2.4em;font-weight:700;text-align:center;margin-bottom:.2em;";
	BOSS_COMMON string CSSCopyright			= "text-align:center;";
	BOSS_COMMON string CSSSections			= "margin-bottom:3em;";
	BOSS_COMMON string CSSSectionTitle		= "cursor:pointer;";
	BOSS_COMMON string CSSSectionPlusMinus	= "display:inline-block;position:relative;top:.05em; font-size:1.3em;width:.6em;margin-right:.1em;";
	BOSS_COMMON string CSSLastSection		= "text-align:center;cursor:default;";
	BOSS_COMMON string CSSTable				= "padding:0 .5em;";
	BOSS_COMMON string CSSList				= "list-style:none;padding-left:0;";
	BOSS_COMMON string CSSListItem			= "margin-left:0;margin-bottom:1em;";
	BOSS_COMMON string CSSSubList			= "margin-top:.5em;padding-left:2.5em;margin-bottom:2em;";
	BOSS_COMMON string CSSCheckbox			= "position:relative;top:.15em;margin-right:.5em;";
	BOSS_COMMON string CSSBlockquote		= "font-style:italic;";
	BOSS_COMMON string CSSError				= "background:red;color:#FFF;display:table;padding:0 4px;";
	BOSS_COMMON string CSSWarning			= "background:orange;color:#FFF;display:table;padding:0 4px;";
	BOSS_COMMON string CSSSuccess			= "color:green;";
	BOSS_COMMON string CSSVersion			= "color:#6394F8;margin-right:1em;padding:0 4px;";
	BOSS_COMMON string CSSGhost				= "color:#888;margin-right:1em;padding:0 4px;";
	BOSS_COMMON string CSSCRC				= "color:#BC8923;margin-right:1em;padding:0 4px;";
	BOSS_COMMON string CSSTagPrefix			= "color:#CD5555;";
	BOSS_COMMON string CSSDirty				= "color:#960;";
	BOSS_COMMON string CSSQuotedMessage		= "color:gray;";
	BOSS_COMMON string CSSMod				= "margin-right:1em;";
	BOSS_COMMON string CSSTag				= "";
	BOSS_COMMON string CSSNote				= "";
	BOSS_COMMON string CSSRequirement		= "";
	BOSS_COMMON string CSSIncompatibility	= "";
	BOSS_COMMON string CSSSubmit			= "margin-right:1em;";
	BOSS_COMMON string CSSPopupBox			= "padding:10px;width:410px;background:white;display:none;z-index:2;position:fixed;top:25%;left:35%;";
	BOSS_COMMON string CSSPopupBoxTitle		= "font-weight:bold;width:100%;display:table-cell;";
	BOSS_COMMON string CSSPopupBoxLink		= "width:400px;";
	BOSS_COMMON string CSSPopupBoxNotes		= "height:10em;width:400px;";
	BOSS_COMMON string CSSPopupBoxClose		= "cursor:pointer;display:table-cell;";
	BOSS_COMMON string CSSPopupBoxSubmit	= "display:block;margin:0 auto;margin-top:20px;";
	BOSS_COMMON string CSSMask				= "position:fixed;left:0px;top:0px;width:100%;height:100%;background:black;opacity:0.9;display:block;z-index:1;";
	BOSS_COMMON string CSSActive			= "color:#0A0;margin-right:1em;padding:0 4px;";

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
				<< "body{" << CSSBody << "}"
				<< "#darkBody{" << CSSDarkBody << "}"
				<< ".darkLink:link{" << CSSDarkLink << "}"
				<< ".darkLink:visited{" << CSSDarkLinkVisited << "}"
				<< "#filters{" << CSSFilters << "}"
				<< "#filters > li,#darkFilters > li{" << CSSFiltersList << "}"
				<< "#darkFilters{" << CSSDarkFilters << "}"
				<< "body > div:first-child{" << CSSTitle << "}"
				<< "body > div:first-child + div{" << CSSCopyright << "}"
				<< "h3 + *{" << CSSSections << "}"
				<< "h3{" << CSSSectionTitle << "}"
				<< "h3 > span{" << CSSSectionPlusMinus << "}"			
				<< "#end{" << CSSLastSection << "}"
				<< "td{" << CSSTable << "}"
				<< "ul{" << CSSList << "}"
				<< "ul li{" << CSSListItem << "}"
				<< "li ul{" << CSSSubList << "}"
				<< "input[type='checkbox']{" << CSSCheckbox << "}"
				<< "blockquote{" << CSSBlockquote << "}"
				<< ".error{" << CSSError << "}"
				<< ".warn{" << CSSWarning << "}"
				<< ".success{" << CSSSuccess << "}"
				<< ".version{" << CSSVersion << "}"
				<< ".ghosted{" << CSSGhost << "}"
				<< ".crc{" << CSSCRC << "}"
				<< ".active{" << CSSActive << "}"
				<< ".tagPrefix{" << CSSTagPrefix << "}"
				<< ".dirty{" << CSSDirty << "}"
				<< ".message{" << CSSQuotedMessage << "}"
				<< ".mod{" << CSSMod << "}"
				<< ".tag{" << CSSTag << "}"
				<< ".note{" << CSSNote << "}"
				<< ".req{" << CSSRequirement << "}"
				<< ".inc{" << CSSIncompatibility << "}"
				<< ".submit{" << CSSSubmit << "}"
				<< "#popupBox{" << CSSPopupBox << "}"
				<< "#mask{" << CSSMask << "}"
				<< "#popupBox > div:first-child{" << CSSPopupBoxTitle << "}"
				<< "#link{" << CSSPopupBoxLink << "}"
				<< "#notes{" << CSSPopupBoxNotes << "}"
				<< "#popupClose{" << CSSPopupBoxClose << "}"
				<< "#popup_yes{" << CSSPopupBoxSubmit << "}"
				<< "</style>"<<endl;
			outStream << "<div>BOSS Log</div>" << endl
				<< "<div>&copy; 2009-2012 BOSS Development Team<br />" << endl
				<< "<a href=\"http://www.gnu.org/licenses/gpl.html\">GNU General Public License v3.0</a><br />" << endl
				<< "v" << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH) << " (" << gl_boss_release_date << ")</div>";
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
				<< "<div id='popupBox'>" << endl
				<< "<div>Plugin Submission</div>" << endl
				<< "<div onclick='hidePopupBox()' id='popupClose'>&#x2715;</div>" << endl
				<< "<p>Plugin: <span id='plugin'></span>" << endl
				<< "<p>Download Location:<br /> <input type='text' id='link' placeholder='Please supply this to speed up the addition process.'/>" << endl
				<< "<p>Additional Notes:<br />" << endl
				<< "<textarea id='notes' placeholder='Any additional information, such as recommended Bash Tags, load order suggestions, ITM/UDR counts and dirty CRCs, can be supplied here.'></textarea>" << endl
				<< "<button id='popup_yes' onclick='submitPlugin()'>Submit Plugin</button>" << endl
				<< "</div>" << endl
				<< "<script>" << endl
				<< "function showPopupBox(plugin){" << endl
				<< "	if(document.getElementById('mask')==null){" << endl
				<< "		var mask=document.createElement('div');" << endl
				<< "		mask.id='mask';" << endl
				<< "		document.body.appendChild(mask);" << endl
				<< "	}" << endl
				<< "	document.getElementById('plugin').innerHTML=plugin;" << endl
				<< "	document.getElementById('popupBox').style.display='block';" << endl
				<< "}" << endl
				<< "function hidePopupBox(){" << endl
				<< "	var mask=document.getElementById('mask');" << endl
				<< "	document.getElementById('popupBox').style.display='none';" << endl
				<< "	document.getElementById('notes').value='';" << endl
				<< "	document.getElementById('link').value='';" << endl
				<< "	if(mask!=null){" << endl
				<< "		var parent=mask.parentNode;" << endl
				<< "		parent.removeChild(mask);" << endl
				<< "	}" << endl
				<< "}" << endl
				<< "function submitPlugin(){" << endl
				<< "}" << endl

				<< "var hm=0,hp=0,hpe=document.getElementById('hp'),hme=document.getElementById('hm');" << endl
				<< "function toggleSectionDisplay(h){if(h.nextSibling.style.display=='none'){h.nextSibling.style.display='block';h.firstChild.innerHTML='&#x2212;'}else{h.nextSibling.style.display='none';h.firstChild.innerHTML='+'}}" << endl
				<< "function swapColorScheme(b){var d=document.body,a=document.getElementsByTagName('a'),f=document.getElementById('filters');if(f==null){f=document.getElementById('darkFilters')}if(b.checked){d.id='darkBody';f.id='darkFilters';for(var i=0,z=a.length;i<z;i++){a[i].className='darkLink'}}else{d.id='';f.id='filters';for(var i=0,z=a.length;i<z;i++){a[i].className=''}}}" << endl
				<< "function toggleDisplayCSS(b,s){var r=new Array();if(document.styleSheets[0].cssRules){r=document.styleSheets[0].cssRules}else if(document.styleSheets[0].rules){r=document.styleSheets[0].rules}for(var i=0,z=r.length;i<z;i++){if(r[i].selectorText.toLowerCase()==s){if(b.checked){r[i].style.display='none'}else{r[i].style.display='inline'}return}}}" << endl
				<< "function toggleRuleListWarnings(b){var u=document.getElementById('userlistMessages');if(u!=null){u=u.childNodes;for(var i=0,z=u.length;i<z;i++){if(u[i].className=='warn'){if(b.checked){u[i].style.display='none';hm++}else if(u[i].style.display=='none'){u[i].style.display='table';hm--}}}}hme.innerHTML=hm}" << endl
				<< "function toggleMessages(){"<<endl
				<< "	var m=document.getElementById('recognised').childNodes,b9=document.getElementById('b9').checked,b10=document.getElementById('b10').checked,b11=document.getElementById('b11').checked,b12=document.getElementById('b12').checked,b13=document.getElementById('b13').checked,b14=document.getElementById('b14').checked;"<<endl
				<< "	for(var i=0,z=m.length;i<z;i++){"<<endl
				<< "		var ac=false,g=false,n=true,c=true,a=m[i].getElementsByTagName('span');"<<endl
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
				<< "	swapColorScheme(document.getElementById('b1'));"<<endl
				<< "	toggleRuleListWarnings(document.getElementById('b2'));"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b3'),'.version','inline');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b4'),'.ghosted','inline');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b5'),'.crc','inline');"<<endl
				<< "	toggleMessages();"<<endl
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
		case HEADING_ID_END_OPEN:
			if (outFormat == HTML)
				outStream << "<h3 id='end'>";
			else
				outStream << endl << endl << "======================================" << endl;
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
		case HEADING_OPEN:
			if (outFormat == HTML)
				outStream << "<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>";
			else
				outStream << endl << endl << "======================================" << endl;
			break;
		case HEADING_CLOSE:
			if (outFormat == HTML)
				outStream << "</h3>";
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
		case LIST_ITEM_SPAN_CLASS_MOD_OPEN:
			if (outFormat == HTML)
				outStream << "<li><span class='mod'>";
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
				outStream << "<button class='submit' onclick='showPopupBox(this.previousSibling.innerHTML)'>Submit Plugin</button>";
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
		commonMap.emplace('\x00', '\x0000');
		commonMap.emplace('\x01', '\x0001');
		commonMap.emplace('\x02', '\x0002');
		commonMap.emplace('\x03', '\x0003');
		commonMap.emplace('\x04', '\x0004');
		commonMap.emplace('\x05', '\x0005');
		commonMap.emplace('\x06', '\x0006');
		commonMap.emplace('\x07', '\x0007');
		commonMap.emplace('\x08', '\x0008');
		commonMap.emplace('\x09', '\x0009');
		commonMap.emplace('\x0A', '\x000A');
		commonMap.emplace('\x0B', '\x000B');
		commonMap.emplace('\x0C', '\x000C');
		commonMap.emplace('\x0D', '\x000D');
		commonMap.emplace('\x0E', '\x000E');
		commonMap.emplace('\x0F', '\x000F');
		commonMap.emplace('\x10', '\x0010');
		commonMap.emplace('\x11', '\x0011');
		commonMap.emplace('\x12', '\x0012');
		commonMap.emplace('\x13', '\x0013');
		commonMap.emplace('\x14', '\x0014');
		commonMap.emplace('\x15', '\x0015');
		commonMap.emplace('\x16', '\x0016');
		commonMap.emplace('\x17', '\x0017');
		commonMap.emplace('\x18', '\x0018');
		commonMap.emplace('\x19', '\x0019');
		commonMap.emplace('\x1A', '\x001A');
		commonMap.emplace('\x1B', '\x001B');
		commonMap.emplace('\x1C', '\x001C');
		commonMap.emplace('\x1D', '\x001D');
		commonMap.emplace('\x1E', '\x001E');
		commonMap.emplace('\x1F', '\x001F');
		commonMap.emplace('\x20', '\x0020');
		commonMap.emplace('\x21', '\x0021');
		commonMap.emplace('\x22', '\x0022');
		commonMap.emplace('\x23', '\x0023');
		commonMap.emplace('\x24', '\x0024');
		commonMap.emplace('\x25', '\x0025');
		commonMap.emplace('\x26', '\x0026');
		commonMap.emplace('\x27', '\x0027');
		commonMap.emplace('\x28', '\x0028');
		commonMap.emplace('\x29', '\x0029');
		commonMap.emplace('\x2A', '\x002A');
		commonMap.emplace('\x2B', '\x002B');
		commonMap.emplace('\x2C', '\x002C');
		commonMap.emplace('\x2D', '\x002D');
		commonMap.emplace('\x2E', '\x002E');
		commonMap.emplace('\x2F', '\x002F');
		commonMap.emplace('\x30', '\x0030');
		commonMap.emplace('\x31', '\x0031');
		commonMap.emplace('\x32', '\x0032');
		commonMap.emplace('\x33', '\x0033');
		commonMap.emplace('\x34', '\x0034');
		commonMap.emplace('\x35', '\x0035');
		commonMap.emplace('\x36', '\x0036');
		commonMap.emplace('\x37', '\x0037');
		commonMap.emplace('\x38', '\x0038');
		commonMap.emplace('\x39', '\x0039');
		commonMap.emplace('\x3A', '\x003A');
		commonMap.emplace('\x3B', '\x003B');
		commonMap.emplace('\x3C', '\x003C');
		commonMap.emplace('\x3D', '\x003D');
		commonMap.emplace('\x3E', '\x003E');
		commonMap.emplace('\x3F', '\x003F');
		commonMap.emplace('\x40', '\x0040');
		commonMap.emplace('\x41', '\x0041');
		commonMap.emplace('\x42', '\x0042');
		commonMap.emplace('\x43', '\x0043');
		commonMap.emplace('\x44', '\x0044');
		commonMap.emplace('\x45', '\x0045');
		commonMap.emplace('\x46', '\x0046');
		commonMap.emplace('\x47', '\x0047');
		commonMap.emplace('\x48', '\x0048');
		commonMap.emplace('\x49', '\x0049');
		commonMap.emplace('\x4A', '\x004A');
		commonMap.emplace('\x4B', '\x004B');
		commonMap.emplace('\x4C', '\x004C');
		commonMap.emplace('\x4D', '\x004D');
		commonMap.emplace('\x4E', '\x004E');
		commonMap.emplace('\x4F', '\x004F');
		commonMap.emplace('\x50', '\x0050');
		commonMap.emplace('\x51', '\x0051');
		commonMap.emplace('\x52', '\x0052');
		commonMap.emplace('\x53', '\x0053');
		commonMap.emplace('\x54', '\x0054');
		commonMap.emplace('\x55', '\x0055');
		commonMap.emplace('\x56', '\x0056');
		commonMap.emplace('\x57', '\x0057');
		commonMap.emplace('\x58', '\x0058');
		commonMap.emplace('\x59', '\x0059');
		commonMap.emplace('\x5A', '\x005A');
		commonMap.emplace('\x5B', '\x005B');
		commonMap.emplace('\x5C', '\x005C');
		commonMap.emplace('\x5D', '\x005D');
		commonMap.emplace('\x5E', '\x005E');
		commonMap.emplace('\x5F', '\x005F');
		commonMap.emplace('\x60', '\x0060');
		commonMap.emplace('\x61', '\x0061');
		commonMap.emplace('\x62', '\x0062');
		commonMap.emplace('\x63', '\x0063');
		commonMap.emplace('\x64', '\x0064');
		commonMap.emplace('\x65', '\x0065');
		commonMap.emplace('\x66', '\x0066');
		commonMap.emplace('\x67', '\x0067');
		commonMap.emplace('\x68', '\x0068');
		commonMap.emplace('\x69', '\x0069');
		commonMap.emplace('\x6A', '\x006A');
		commonMap.emplace('\x6B', '\x006B');
		commonMap.emplace('\x6C', '\x006C');
		commonMap.emplace('\x6D', '\x006D');
		commonMap.emplace('\x6E', '\x006E');
		commonMap.emplace('\x6F', '\x006F');
		commonMap.emplace('\x70', '\x0070');
		commonMap.emplace('\x71', '\x0071');
		commonMap.emplace('\x72', '\x0072');
		commonMap.emplace('\x73', '\x0073');
		commonMap.emplace('\x74', '\x0074');
		commonMap.emplace('\x75', '\x0075');
		commonMap.emplace('\x76', '\x0076');
		commonMap.emplace('\x77', '\x0077');
		commonMap.emplace('\x78', '\x0078');
		commonMap.emplace('\x79', '\x0079');
		commonMap.emplace('\x7A', '\x007A');
		commonMap.emplace('\x7B', '\x007B');
		commonMap.emplace('\x7C', '\x007C');
		commonMap.emplace('\x7D', '\x007D');
		commonMap.emplace('\x7E', '\x007E');
		commonMap.emplace('\x7F', '\x007F');
		commonMap.emplace('\x82', '\xE2\x82\xAC');
		commonMap.emplace('\x84', '\xE2\x80\x9E');
		commonMap.emplace('\x85', '\xE2\x80\xA6');
		commonMap.emplace('\x86', '\xE2\x80\xA0');
		commonMap.emplace('\x87', '\xE2\x80\xA1');
		commonMap.emplace('\x89', '\xE2\x80\xB0');
		commonMap.emplace('\x8B', '\xE2\x80\xB9');
		commonMap.emplace('\x91', '\xE2\x80\x98');
		commonMap.emplace('\x92', '\xE2\x80\x99');
		commonMap.emplace('\x93', '\xE2\x80\x9C');
		commonMap.emplace('\x94', '\xE2\x80\x9D');
		commonMap.emplace('\x95', '\xE2\x80\xA2');
		commonMap.emplace('\x96', '\xE2\x80\x93');
		commonMap.emplace('\x97', '\xE2\x80\x94');
		commonMap.emplace('\x99', '\xE2\x84\xA2');
		commonMap.emplace('\x9B', '\xE2\x80\xBA');
		commonMap.emplace('\xA0', '\xC2\xA0');
		commonMap.emplace('\xA4', '\xC2\xA4');
		commonMap.emplace('\xA6', '\xC2\xA6');
		commonMap.emplace('\xA7', '\xC2\xA7');
		commonMap.emplace('\xA9', '\xC2\xA9');
		commonMap.emplace('\xAB', '\xC2\xAB');
		commonMap.emplace('\xAC', '\xC2\xAC');
		commonMap.emplace('\xAD', '\xC2\xAD');
		commonMap.emplace('\xAE', '\xC2\xAE');
		commonMap.emplace('\xB0', '\xC2\xB0');
		commonMap.emplace('\xB1', '\xC2\xB1');
		commonMap.emplace('\xB5', '\xC2\xB5');
		commonMap.emplace('\xB6', '\xC2\xB6');
		commonMap.emplace('\xB7', '\xC2\xB7');
		commonMap.emplace('\xBB', '\xC2\xBB');
		/*
		//Now fill 1251 -> UTF-8 map.
		map1251toUtf8 = commonMap;  //Fill with common mapped characters.
		map1251toUtf8.emplace('\x80', '\x0402');
		map1251toUtf8.emplace('\x81', '\x0403');
		map1251toUtf8.emplace('\x83', '\x0453');
		map1251toUtf8.emplace('\x88', '\x20AC');
		map1251toUtf8.emplace('\x8A', '\x0409');
		map1251toUtf8.emplace('\x8C', '\x040A');
		map1251toUtf8.emplace('\x8D', '\x040C');
		map1251toUtf8.emplace('\x8E', '\x040B');
		map1251toUtf8.emplace('\x8F', '\x040F');
		map1251toUtf8.emplace('\x90', '\x0452');
		map1251toUtf8.emplace('\x98', '\x0020');
		map1251toUtf8.emplace('\x9A', '\x0459');
		map1251toUtf8.emplace('\x9C', '\x045A');
		map1251toUtf8.emplace('\x9D', '\x045C');
		map1251toUtf8.emplace('\x9E', '\x045B');
		map1251toUtf8.emplace('\x9F', '\x045F');
		map1251toUtf8.emplace('\xA1', '\x040E');
		map1251toUtf8.emplace('\xA2', '\x045E');
		map1251toUtf8.emplace('\xA3', '\x0408');
		map1251toUtf8.emplace('\xA5', '\x0490');
		map1251toUtf8.emplace('\xA8', '\x0401');
		map1251toUtf8.emplace('\xAA', '\x0404');
		map1251toUtf8.emplace('\xAF', '\x0407');
		map1251toUtf8.emplace('\xB2', '\x0406');
		map1251toUtf8.emplace('\xB3', '\x0456');
		map1251toUtf8.emplace('\xB4', '\x0491');
		map1251toUtf8.emplace('\xB8', '\x0451');
		map1251toUtf8.emplace('\xB9', '\x2116');
		map1251toUtf8.emplace('\xBA', '\x0454');
		map1251toUtf8.emplace('\xBC', '\x0458');
		map1251toUtf8.emplace('\xBD', '\x0405');
		map1251toUtf8.emplace('\xBE', '\x0455');
		map1251toUtf8.emplace('\xBF', '\x0457');
		map1251toUtf8.emplace('\xC0', '\x0410');
		map1251toUtf8.emplace('\xC1', '\x0411');
		map1251toUtf8.emplace('\xC2', '\x0412');
		map1251toUtf8.emplace('\xC3', '\x0413');
		map1251toUtf8.emplace('\xC4', '\x0414');
		map1251toUtf8.emplace('\xC5', '\x0415');
		map1251toUtf8.emplace('\xC6', '\x0416');
		map1251toUtf8.emplace('\xC7', '\x0417');
		map1251toUtf8.emplace('\xC8', '\x0418');
		map1251toUtf8.emplace('\xC9', '\x0419');
		map1251toUtf8.emplace('\xCA', '\x041A');
		map1251toUtf8.emplace('\xCB', '\x041B');
		map1251toUtf8.emplace('\xCC', '\x041C');
		map1251toUtf8.emplace('\xCD', '\x041D');
		map1251toUtf8.emplace('\xCE', '\x041E');
		map1251toUtf8.emplace('\xCF', '\x041F');
		map1251toUtf8.emplace('\xD0', '\x0420');
		map1251toUtf8.emplace('\xD1', '\x0421');
		map1251toUtf8.emplace('\xD2', '\x0422');
		map1251toUtf8.emplace('\xD3', '\x0423');
		map1251toUtf8.emplace('\xD4', '\x0424');
		map1251toUtf8.emplace('\xD5', '\x0425');
		map1251toUtf8.emplace('\xD6', '\x0426');
		map1251toUtf8.emplace('\xD7', '\x0427');
		map1251toUtf8.emplace('\xD8', '\x0428');
		map1251toUtf8.emplace('\xD9', '\x0429');
		map1251toUtf8.emplace('\xDA', '\x042A');
		map1251toUtf8.emplace('\xDB', '\x042B');
		map1251toUtf8.emplace('\xDC', '\x042C');
		map1251toUtf8.emplace('\xDD', '\x042D');
		map1251toUtf8.emplace('\xDE', '\x042E');
		map1251toUtf8.emplace('\xDF', '\x042F');
		map1251toUtf8.emplace('\xE0', '\x0430');
		map1251toUtf8.emplace('\xE1', '\x0431');
		map1251toUtf8.emplace('\xE2', '\x0432');
		map1251toUtf8.emplace('\xE3', '\x0433');
		map1251toUtf8.emplace('\xE4', '\x0434');
		map1251toUtf8.emplace('\xE5', '\x0435');
		map1251toUtf8.emplace('\xE6', '\x0436');
		map1251toUtf8.emplace('\xE7', '\x0437');
		map1251toUtf8.emplace('\xE8', '\x0438');
		map1251toUtf8.emplace('\xE9', '\x0439');
		map1251toUtf8.emplace('\xEA', '\x043A');
		map1251toUtf8.emplace('\xEB', '\x043B');
		map1251toUtf8.emplace('\xEC', '\x043C');
		map1251toUtf8.emplace('\xED', '\x043D');
		map1251toUtf8.emplace('\xEE', '\x043E');
		map1251toUtf8.emplace('\xEF', '\x043F');
		map1251toUtf8.emplace('\xF0', '\x0440');
		map1251toUtf8.emplace('\xF1', '\x0441');
		map1251toUtf8.emplace('\xF2', '\x0442');
		map1251toUtf8.emplace('\xF3', '\x0443');
		map1251toUtf8.emplace('\xF4', '\x0444');
		map1251toUtf8.emplace('\xF5', '\x0445');
		map1251toUtf8.emplace('\xF6', '\x0446');
		map1251toUtf8.emplace('\xF7', '\x0447');
		map1251toUtf8.emplace('\xF8', '\x0448');
		map1251toUtf8.emplace('\xF9', '\x0449');
		map1251toUtf8.emplace('\xFA', '\x044A');
		map1251toUtf8.emplace('\xFB', '\x044B');
		map1251toUtf8.emplace('\xFC', '\x044C');
		map1251toUtf8.emplace('\xFD', '\x044D');
		map1251toUtf8.emplace('\xFE', '\x044E');
		map1251toUtf8.emplace('\xFF', '\x044F');
		*/
		//Now fill 1252 -> UTF-8 map.
		map1252toUtf8 = commonMap;  //Fill with common mapped characters.
		map1252toUtf8.emplace('\x80', '\xE2\x82\xAC');
		map1252toUtf8.emplace('\x83', '\xC6\x92');
		map1252toUtf8.emplace('\x88', '\xCB\x86');
		map1252toUtf8.emplace('\x8A', '\xC5\xA0');
		map1252toUtf8.emplace('\x8C', '\xC5\x92');
		map1252toUtf8.emplace('\x8E', '\xC5\xBD');
		map1252toUtf8.emplace('\x98', '\xCB\x9C');
		map1252toUtf8.emplace('\x9A', '\xC5\xA1');
		map1252toUtf8.emplace('\x9C', '\xC5\x93');
		map1252toUtf8.emplace('\x9E', '\xC5\xBE');
		map1252toUtf8.emplace('\x9F', '\xC5\xB8');
		map1252toUtf8.emplace('\xA1', '\xC2\xA1');
		map1252toUtf8.emplace('\xA2', '\xC2\xA2');
		map1252toUtf8.emplace('\xA3', '\xC2\xA3');
		map1252toUtf8.emplace('\xA5', '\xC2\xA5');
		map1252toUtf8.emplace('\xA8', '\xC2\xA8');
		map1252toUtf8.emplace('\xAA', '\xC2\xAA');
		map1252toUtf8.emplace('\xAF', '\xC2\xAF');
		map1252toUtf8.emplace('\xB2', '\xC2\xB2');
		map1252toUtf8.emplace('\xB3', '\xC2\xB3');
		map1252toUtf8.emplace('\xB4', '\xC2\xB4');
		map1252toUtf8.emplace('\xB8', '\xC2\xB8');
		map1252toUtf8.emplace('\xB9', '\xC2\xB9');
		map1252toUtf8.emplace('\xBA', '\xC2\xBA');
		map1252toUtf8.emplace('\xBC', '\xC2\xBC');
		map1252toUtf8.emplace('\xBD', '\xC2\xBD');
		map1252toUtf8.emplace('\xBE', '\xC2\xBE');
		map1252toUtf8.emplace('\xBF', '\xC2\xBF');
		map1252toUtf8.emplace('\xC0', '\xC3\x80');
		map1252toUtf8.emplace('\xC1', '\xC3\x81');
		map1252toUtf8.emplace('\xC2', '\xC3\x82');
		map1252toUtf8.emplace('\xC3', '\xC3\x83');
		map1252toUtf8.emplace('\xC4', '\xC3\x84');
		map1252toUtf8.emplace('\xC5', '\xC3\x85');
		map1252toUtf8.emplace('\xC6', '\xC3\x86');
		map1252toUtf8.emplace('\xC7', '\xC3\x87');
		map1252toUtf8.emplace('\xC8', '\xC3\x88');
		map1252toUtf8.emplace('\xC9', '\xC3\x89');
		map1252toUtf8.emplace('\xCA', '\xC3\x8A');
		map1252toUtf8.emplace('\xCB', '\xC3\x8B');
		map1252toUtf8.emplace('\xCC', '\xC3\x8C');
		map1252toUtf8.emplace('\xCD', '\xC3\x8D');
		map1252toUtf8.emplace('\xCE', '\xC3\x8E');
		map1252toUtf8.emplace('\xCF', '\xC3\x8F');
		map1252toUtf8.emplace('\xD0', '\xC3\x90');
		map1252toUtf8.emplace('\xD1', '\xC3\x91');
		map1252toUtf8.emplace('\xD2', '\xC3\x92');
		map1252toUtf8.emplace('\xD3', '\xC3\x93');
		map1252toUtf8.emplace('\xD4', '\xC3\x94');
		map1252toUtf8.emplace('\xD5', '\xC3\x95');
		map1252toUtf8.emplace('\xD6', '\xC3\x96');
		map1252toUtf8.emplace('\xD7', '\xC3\x97');
		map1252toUtf8.emplace('\xD8', '\xC3\x98');
		map1252toUtf8.emplace('\xD9', '\xC3\x99');
		map1252toUtf8.emplace('\xDA', '\xC3\x9A');
		map1252toUtf8.emplace('\xDB', '\xC3\x9B');
		map1252toUtf8.emplace('\xDC', '\xC3\x9C');
		map1252toUtf8.emplace('\xDD', '\xC3\x9D');
		map1252toUtf8.emplace('\xDE', '\xC3\x9E');
		map1252toUtf8.emplace('\xDF', '\xC3\x9F');
		map1252toUtf8.emplace('\xE0', '\xC3\xA0');
		map1252toUtf8.emplace('\xE1', '\xC3\xA1');
		map1252toUtf8.emplace('\xE2', '\xC3\xA2');
		map1252toUtf8.emplace('\xE3', '\xC3\xA3');
		map1252toUtf8.emplace('\xE4', '\xC3\xA4');
		map1252toUtf8.emplace('\xE5', '\xC3\xA5');
		map1252toUtf8.emplace('\xE6', '\xC3\xA6');
		map1252toUtf8.emplace('\xE7', '\xC3\xA7');
		map1252toUtf8.emplace('\xE8', '\xC3\xA8');
		map1252toUtf8.emplace('\xE9', '\xC3\xA9');
		map1252toUtf8.emplace('\xEA', '\xC3\xAA');
		map1252toUtf8.emplace('\xEB', '\xC3\xAB');
		map1252toUtf8.emplace('\xEC', '\xC3\xAC');
		map1252toUtf8.emplace('\xED', '\xC3\xAD');
		map1252toUtf8.emplace('\xEE', '\xC3\xAE');
		map1252toUtf8.emplace('\xEF', '\xC3\xAF');
		map1252toUtf8.emplace('\xF0', '\xC3\xB0');
		map1252toUtf8.emplace('\xF1', '\xC3\xB1');
		map1252toUtf8.emplace('\xF2', '\xC3\xB2');
		map1252toUtf8.emplace('\xF3', '\xC3\xB3');
		map1252toUtf8.emplace('\xF4', '\xC3\xB4');
		map1252toUtf8.emplace('\xF5', '\xC3\xB5');
		map1252toUtf8.emplace('\xF6', '\xC3\xB6');
		map1252toUtf8.emplace('\xF7', '\xC3\xB7');
		map1252toUtf8.emplace('\xF8', '\xC3\xB8');
		map1252toUtf8.emplace('\xF9', '\xC3\xB9');
		map1252toUtf8.emplace('\xFA', '\xC3\xBA');
		map1252toUtf8.emplace('\xFB', '\xC3\xBB');
		map1252toUtf8.emplace('\xFC', '\xC3\xBC');
		map1252toUtf8.emplace('\xFD', '\xC3\xBD');
		map1252toUtf8.emplace('\xFE', '\xC3\xBE');
		map1252toUtf8.emplace('\xFF', '\xC3\xBF');

		currentEncoding = 0;
	}

	void Transcoder::SetEncoding(uint32_t inEncoding) {
		if (inEncoding != 1251 && inEncoding != 1252)
			return;
		currentEncoding = inEncoding;
		//Set the enc -> UTF-8 map.
		if (inEncoding == 1251)
			encToUtf8 = map1251toUtf8;
		else if (inEncoding == 1252)
			encToUtf8 = map1252toUtf8;

		//Now create the UTF-8 -> enc map.
		for (boost::unordered_map<char, char>::iterator iter = encToUtf8.begin(); iter != encToUtf8.end(); ++iter) {
			utf8toEnc.emplace(iter->second, iter->first);  //Swap mapping. There *should* be unique values for each character either way.
		}
	}

	uint32_t Transcoder::GetEncoding() {
		return currentEncoding;
	}

	uint32_t Transcoder::DetectEncoding(string str) {
		return 1252;
	}

	string Transcoder::Utf8ToEnc(string inString) {
		stringstream outString;
	/*	char * str = (char*)inString.c_str();
		char * str_i = str;
		char * end = str+strlen(str)+1;
		do {
			char cp = 
		//I need to use a UTF-8 string iterator. See UTF-CPP for usage.
		for (utf8::iterator<string::iterator> iter; iter.base() != inString.end(); ++iter) {*/
		for (string::iterator iter = inString.begin(); iter != inString.end(); ++iter) {
			boost::unordered_map<char, char>::iterator mapIter = utf8toEnc.find(*iter);
			if (mapIter != utf8toEnc.end())
				outString << mapIter->second;
			else
				throw boss_error(BOSS_ERROR_ENCODING_CONVERSION_FAIL, inString);
		}
		return outString.str();
	}

	string Transcoder::EncToUtf8(string inString) {
		stringstream outString;
		for (string::iterator iter = inString.begin(); iter != inString.end(); ++iter) {
			boost::unordered_map<char, char>::iterator mapIter = encToUtf8.find(*iter);
			if (mapIter != encToUtf8.end())
				outString << mapIter->second;
			else
				throw boss_error(BOSS_ERROR_ENCODING_CONVERSION_FAIL, inString);
		}
		return outString.str();
	}
}