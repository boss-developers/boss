/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "BOSSLog.h"
#include "Globals.h"
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/karma.hpp>

namespace boss {
	using namespace std;
	namespace karma = boost::spirit::karma;
	using boost::algorithm::replace_all;
	using boost::algorithm::replace_first;

	//Default filter options.
	bool UseDarkColourScheme    = 0;
	bool HideVersionNumbers     = 0;
	bool HideGhostedLabel       = 0;
	bool HideChecksums          = 0;
	bool HideMessagelessMods    = 0;
	bool HideGhostedMods        = 0;
	bool HideRuleWarnings       = 0;
	bool HideAllModMessages     = 0;
	bool HideNotes              = 0;
	bool HideBashTagSuggestions = 0;
	bool HideRequirements       = 0;
	bool HideIncompatibilities  = 0;

	//Default CSS.
	string CSSBody				= "font-family:Calibri,Arial,sans-serifs;";
	string CSSFilters			= "border:1px grey dashed; background:#F5F5F5; padding:0.3em; display:table;";
	string CSSFiltersList		= "display:inline-block; padding:0.2em 1em; white-space:nowrap; margin:0;";
	string CSSTitle				= "font-size:2.4em; font-weight:bold; text-align:center; margin-bottom:0.2em;";
	string CSSCopyright			= "text-align:center;";
	string CSSSections			= "margin-bottom:4em;";
	string CSSSectionTitle		= "font-weight:bold; font-size:1.3em; cursor:pointer;";
	string CSSSectionPlusMinus	= "display:inline-block; position:relative; top:0.05em; font-size:1.30em; width:0.6em; margin-right:0.1em;";
	string CSSTopLevelList		= "padding-left:0; margin-top:1em;";
	string CSSLastSection		= "margin:0;";
	string CSSLastSectionTitle	= "cursor:default;";
	string CSSTopLevelListItem	= "margin-left:0; margin-bottom:2em;";
	string CSSList				= "list-style:none;";
	string CSSListItem			= "margin-bottom:0.4em;";
	string CSSItemList			= "CSSListItem";
	string CSSCheckbox			= "position:relative; top:0.15em; margin-right:0.5em;";
	string CSSBlockquote		= "font-style:italic;";
	string CSSUnrecognisedList	= "margin-bottom:1em;";
	string CSSSummaryRow		= "display:table-row;";
	string CSSSummaryCell		= "display:table-cell; padding: 0 10px;";
	string CSSError				= "background:red; color:white; display:table; padding:0 4px;";
	string CSSWarning			= "background:orange; color:white; display:table; padding:0 4px;";
	string CSSSuccess			= "color:green;";
	string CSSVersion			= "color:#6394F8; margin-left:1.3em; padding:0 4px;";
	string CSSGhost				= "color:#888888; margin-left:1.3em; padding:0 4px;";
	string CSSCRC				= "color:#BC8923; margin-left:1.3em; padding:0 4px;";
	string CSSTagPrefix			= "color:#CD5555;";
	string CSSDirty				= "color:#996600;";
	string CSSQuotedMessage		= "color:grey;";
	string CSSMod				= "";
	string CSSTag				= "";
	string CSSNote				= "";
	string CSSRequirement		= "";
	string CSSIncompatibility	= "";
	

	void ShowMessage(string& buffer, message currentMessage) {
		size_t pos1,pos2;
		string link;
		//Wrap web addresses in HTML link format. Skip those already in HTML format.
		pos1 = currentMessage.data.find("\"http");  //Start of a link.
		while (pos1 != string::npos) {
			if (currentMessage.data[pos1-1] != '=') {  //Link is (probably) not in HTML format.
				pos2 = currentMessage.data.find("\"",pos1+1)+1;  //First character after the end of the link.
				link = currentMessage.data.substr(pos1,pos2-pos1);
				link = "<a href=" + link + ">" + link.substr(1,link.length()-2) + "</a>";
				currentMessage.data.replace(pos1,pos2-pos1,link);
			}
			pos1 = currentMessage.data.find("http",pos1 + link.length());
		}
		//Select message formatting.
		switch(currentMessage.key) {
		case TAG:
			buffer += "<li class='tag'><span class='tagPrefix'>Bash Tag suggestion(s):</span> " + currentMessage.data + "</li>\n";
			break;
		case SAY:
			buffer += "<li class='note'>Note: " + currentMessage.data + "</li>\n";
			break;
		case REQ:
			buffer += "<li class='req'>Requires: " + currentMessage.data + "</li>\n";
			break;
		case SPECIFIC_REQ:
			buffer += "<li class='req'>Requires: " + currentMessage.data + "</li>\n";
			break;
		case INC:
			buffer += "<li class='inc'>Incompatible with: " + currentMessage.data + "</li>\n";
			break;
		case SPECIFIC_INC:
			buffer += "<li class='inc'>Incompatible with: " + currentMessage.data + "</li>\n";
			break;
		case WARN:
			buffer += "<li class='warn'>Warning: " + currentMessage.data + "</li>\n";
			break;
		case ERR:
			buffer += "<li class='error'>ERROR: " + currentMessage.data + "</li>\n";
			break;
		case DIRTY:
			buffer += "<li class='dirty'>Contains dirty edits: " + currentMessage.data + "</li>\n";
			break;
		default:
			buffer += "<li class='note'>Note: " + currentMessage.data + "</li>\n";
			break;
		}
	}

	//Prints header if format is HTML, else nothing.
	void OutputHeader() {
		if (log_format == "html") {
			bosslog << "<!DOCTYPE html>"<<endl<<"<html>"<<endl<<"<head>"<<endl<<"<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>"<<endl
				<< "<title>BOSS Log</title>"<<endl<<"<style type='text/css'>"<<endl
				<< "body {" << CSSBody << "}"<<endl
				<< ".filters {" << CSSFilters << "}"<<endl
				<< ".filters > li {" << CSSFiltersList << "}"<<endl
				<< "body > div:first-child {" << CSSTitle << "}"<<endl
				<< "body > div:first-child + div {" << CSSCopyright << "}"<<endl
				<< "body > div {" << CSSSections << "}"<<endl
				<< "body > div > span:first-child {" << CSSSectionTitle << "}"<<endl
				<< "body > div > span:first-child > span {" << CSSSectionPlusMinus << "}"<<endl			
				<< "div > ul {" << CSSTopLevelList << "}"<<endl
				<< "body > div:last-child {" << CSSLastSection << "}"<<endl
				<< "body > div:last-child > span:first-child {" << CSSLastSectionTitle << "}"<<endl
				<< "div > ul > li {" << CSSTopLevelListItem << "}"<<endl
				<< "ul {" << CSSList << "}"<<endl
				<< "ul li {" << CSSListItem << "}"<<endl
				<< "li ul {" << CSSItemList << "}"<<endl
				<< "input[type='checkbox'] {" << CSSCheckbox << "}"<<endl
				<< "blockquote {" << CSSBlockquote << "}"<<endl
				<< "#unrecognised > li {" << CSSUnrecognisedList << "}"<<endl
				<< "#summary > div {" << CSSSummaryRow << "}"<<endl
				<< "#summary > div > div {" << CSSSummaryCell << "}"<<endl
				<< ".error {" << CSSError << "}"<<endl
				<< ".warn {" << CSSWarning << "}"<<endl
				<< ".success {" << CSSSuccess << "}"<<endl
				<< ".version {" << CSSVersion << "}"<<endl
				<< ".ghosted {" << CSSGhost << "}"<<endl
				<< ".crc {" << CSSCRC << "}"<<endl
				<< ".tagPrefix {" << CSSTagPrefix << "}"<<endl
				<< ".dirty {" << CSSDirty << "}"<<endl
				<< ".message {" << CSSQuotedMessage << "}"<<endl
				<< ".mod{" << CSSMod << "} .tag{" << CSSTag << "} .note{" << CSSNote << "} .req{" << CSSRequirement << "} .inc{" << CSSIncompatibility << "}"<<endl
				<< "</style>"<<endl
				<< "<script type='text/javascript'>"<<endl
				<< "function toggleSectionDisplay(heading){"<<endl
				<< "	if(heading.nextSibling.style.display=='block'||heading.nextSibling.style.display==''){"<<endl
				<< "		heading.nextSibling.style.display='none';"<<endl
				<< "		heading.firstChild.innerHTML='+';"<<endl
				<< "		heading.parentNode.style.marginBottom = '1em';"<<endl
				<< "	}else{"<<endl
				<< "		heading.nextSibling.style.display='block';"<<endl
				<< "		heading.firstChild.innerHTML='&#x2212;';"<<endl
				<< "		heading.parentNode.style.marginBottom = '4em';"<<endl
				<< "	}"<<endl
				<< "	return;"<<endl
				<< "}"<<endl
				<< "function toggleMods(){"<<endl
				<< "	var hideNoMessageMods=document.getElementById('noMessageModFilter').checked;"<<endl
				<< "	var hideGhostMods=document.getElementById('ghostModFilter').checked;"<<endl
				<< "	var mods=document.getElementById('recognised').childNodes;"<<endl
				<< "	for(i=0;i<mods.length;i++){"<<endl
				<< "		if(mods[i].nodeType==1){"<<endl
				<< "			var ghosted=false;"<<endl
				<< "			var noMods=false;"<<endl
				<< "			var childs=mods[i].getElementsByTagName('span');"<<endl
				<< "			for(j=0;j<childs.length;j++){"<<endl
				<< "				if(childs[j].className=='ghosted'){"<<endl
				<< "					ghosted=true;"<<endl
				<< "					break;"<<endl
				<< "				}"<<endl
				<< "			}"<<endl
				<< "			if (!mods[i].getElementsByTagName('li')[0]){"<<endl
				<< "				noMods=true;"<<endl
				<< "			}else{"<<endl
				<< "				noMods=true;"<<endl
				<< "				childs=mods[i].getElementsByTagName('li');"<<endl
				<< "				for(j=0;j<childs.length;j++){"<<endl
				<< "					var childDisplay, parentDisplay;"<<endl
				<< "					if(window.getComputedStyle){"<<endl
				<< "						parentDisplay=window.getComputedStyle(childs[j].parentNode, null).getPropertyValue('display');"<<endl
				<< "						childDisplay=window.getComputedStyle(childs[j], null).getPropertyValue('display');"<<endl
				<< "					}else if(childs[j].currentStyle){"<<endl
				<< "						parentDisplay=childs[j].parentNode.currentStyle.display;"<<endl
				<< "						childDisplay=childs[j].currentStyle.display;"<<endl
				<< "					}"<<endl
				<< "					if(parentDisplay=='none') {"<<endl
				<< "							break;"<<endl
				<< "					}else{"<<endl
				<< "						if(childDisplay!='none') {"<<endl
				<< "							noMods=false;"<<endl
				<< "							break;"<<endl
				<< "						}"<<endl
				<< "					}"<<endl
				<< "				}"<<endl
				<< "			}"<<endl
				<< "			if(hideNoMessageMods&&noMods){"<<endl
				<< "				mods[i].style.display='none';"<<endl
				<< "			}else if(hideGhostMods&&ghosted){"<<endl
				<< "				mods[i].style.display='none';"<<endl
				<< "			}else{"<<endl
				<< "				mods[i].style.display='block';"<<endl
				<< "			}"<<endl
				<< "		}"<<endl
				<< "	}"<<endl
				<< "	return;"<<endl
				<< "}"<<endl
				<< "function toggleDisplayCSS(box, selector, defaultDisplay){"<<endl
				<< "	var theRules=new Array();"<<endl
				<< "	if(document.styleSheets[0].cssRules){"<<endl
				<< "		theRules=document.styleSheets[0].cssRules;"<<endl
				<< "	}else if(document.styleSheets[0].rules){"<<endl
				<< "		theRules=document.styleSheets[0].rules;"<<endl
				<< "	}"<<endl
				<< "	for(i=0;i<theRules.length;i++){"<<endl
				<< "		if(theRules[i].selectorText.toLowerCase() == selector){"<<endl
				<< "			if(box.checked){"<<endl
				<< "				theRules[i].style.display='none';"<<endl
				<< "			}else{"<<endl
				<< "				theRules[i].style.display=defaultDisplay;"<<endl
				<< "			}return;"<<endl
				<< "		}"<<endl
				<< "	}return;"<<endl
				<< "}"<<endl
				<< "function swapColorScheme(box) {"<<endl
				<< "	var theRules=new Array();"<<endl
				<< "	if(document.styleSheets[0].cssRules){"<<endl
				<< "		theRules=document.styleSheets[0].cssRules;"<<endl
				<< "	}else if(document.styleSheets[0].rules){"<<endl
				<< "		theRules=document.styleSheets[0].rules;"<<endl
				<< "	}"<<endl
				<< "	for(i=0;i<theRules.length;i++){"<<endl
				<< "		if(theRules[i].selectorText.toLowerCase() == 'body'){"<<endl
				<< "			if(box.checked){"<<endl
				<< "				theRules[i].style.color='white';"<<endl
				<< "				theRules[i].style.background='black';"<<endl
				<< "			}else{"<<endl
				<< "				theRules[i].style.color='black';"<<endl
				<< "				theRules[i].style.background='white';"<<endl
				<< "			}"<<endl
				<< "		}"<<endl
				<< "		if(theRules[i].selectorText.toLowerCase() == '.filters'){"<<endl
				<< "			if(box.checked){"<<endl
				<< "				theRules[i].style.background='#333333';"<<endl
				<< "			}else{"<<endl
				<< "				theRules[i].style.background='#F5F5F5';"<<endl
				<< "			}return;"<<endl
				<< "		}"<<endl
				<< "	}"<<endl
				<< "	return;"<<endl
				<< "}"<<endl
				<< "function toggleUserlistWarnings(box) {"<<endl
				<< "	var userlistMessages=document.getElementById('userlistMessages').childNodes;"<<endl
				<< "	if (userlistMessages){"<<endl
				<< "		for(i=0;i<userlistMessages.length;i++){"<<endl
				<< "			if(userlistMessages[i].nodeType==1){"<<endl
				<< "				var backgroundColor;"<<endl
				<< "				if(window.getComputedStyle){"<<endl
				<< "					backgroundColor=window.getComputedStyle(userlistMessages[i], null).getPropertyValue('background-color');"<<endl
				<< "				}else if(userlistMessages[i].currentStyle){"<<endl
				<< "					backgroundColor=userlistMessages[i].currentStyle.backgroundColor;"<<endl
				<< "				}"<<endl
				<< "				if(backgroundColor=='rgb(255, 165, 0)'||backgroundColor=='#ffa500'){"<<endl
				<< "					if(box.checked){"<<endl
				<< "						userlistMessages[i].style.display = 'none';"<<endl
				<< "					}else{"<<endl
				<< "						userlistMessages[i].style.display = 'table';"<<endl
				<< "					}"<<endl
				<< "				}"<<endl
				<< "			}"<<endl
				<< "		}"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function initialSetup() {"<<endl
				<< "	swapColorScheme(document.getElementById('b1'));"<<endl
				<< "	toggleUserlistWarnings(document.getElementById('b12'));"<<endl
				<< "	toggleMods();"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b2'),'.version','inline');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b3'),'.ghosted','inline');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b4'),'.crc','inline');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b7'),'li ul','block');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b8'),'.note','table');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b9'),'.tag','table');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b10'),'.req','table');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b11'),'.inc','table');"<<endl
				<< "}"<<endl
				<< "function DomReady(fn){"<<endl
				<< "	if(document.addEventListener){"<<endl
				<< "		document.addEventListener('DOMContentLoaded', fn, false);"<<endl
				<< "	}else{"<<endl
				<< "		document.onreadystatechange = function(){readyState(fn)}"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "function readyState(fn){"<<endl
				<< "	if(document.readyState == 'interactive'){"<<endl
				<< "		fn();"<<endl
				<< "	}"<<endl
				<< "}"<<endl
				<< "window.onDomReady = DomReady;"<<endl
				<< "window.onDomReady(initialSetup);"<<endl
				<< "</script>"<<endl
				<< "</head><body>"<<endl;
		}
		Output("<div>Better Oblivion Sorting Software Log</div>\n");
		Output("<div>&copy; Random007 &amp; the BOSS development team, 2009-2011. Some rights reserved.<br />\n");
		Output("<a href=\"http://creativecommons.org/licenses/by-nc-nd/3.0/\">CC Attribution-Noncommercial-No Derivative Works 3.0</a><br />\n");
		Output("v"+g_version+" ("+g_releaseDate+")</div>\n");
	}

	//Converts an integer to a string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
	string IntToString(unsigned int n) {
		string out;
		back_insert_iterator<string> sink(out);
		karma::generate(sink,karma::upper[karma::uint_],n);
		return out;
	}

	//Converts an integer to a hex string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
	string IntToHexString(unsigned int n) {
		string out;
		back_insert_iterator<string> sink(out);
		karma::generate(sink,karma::upper[karma::hex],n);
		return out;
	}

	//Prints ouptut with formatting according to output format.
	void Output(string text) {
		if (log_format == "text") {
			//Yes. This really is as horrific as it looks. It should be only temporary though.
			replace_first(text, "</body>\n</html>", "");
			replace_first(text, "&copy;", "c");
			replace_first(text, "&amp;", "&");
			replace_first(text, "&#x2212;", "");
			
			
			replace_all(text, "</ul>", "");
			replace_all(text, "<b>", "");
			replace_all(text, "</b>", "");
			replace_all(text, "<i>", "");
			replace_all(text, "</i>", "");

			replace_first(text, "<span>", "======================================\n");

			replace_all(text, " class='warn'>", ">");
			replace_all(text, " class='error'>", ">");
			replace_all(text, " class='success'>", ">");
			replace_all(text, " class='tag'>", ">");
			replace_all(text, " class='ghosted'>", ">  ");
			replace_all(text, " class='version'>", ">  ");
			replace_all(text, " class='dirty'>", ">");
			replace_all(text, " class='crc'>", ">  ");
			replace_all(text, " class='note'>", ">");
			replace_all(text, " class='req'>", ">");
			replace_all(text, " class='inc'>", ">");
			replace_all(text, " class='tagPrefix'>", ">");
			replace_all(text, "<li><span class='mod'>","");

			replace_first(text, " onclick='toggleSectionDisplay(this)'>", ">");
			replace_first(text, " id='seplugins'>", ">");
			replace_first(text, " id='recognised'>", ">");
			replace_first(text, " id='unrecognised'>", ">");
			replace_first(text, " id='userlistMessages'>", ">");
			replace_first(text, " id='summary'>", ">");

			replace_all(text, "<blockquote>", "\n\n");
			replace_all(text, "</blockquote>", "\n\n");
			replace_all(text, "<ul>", "");
			
			replace_all(text, "<li>", "*  ");
			replace_all(text, "</li>", "");
			
			replace_all(text, "</div>", "");
			replace_all(text, "</p>", "");	

			replace_all(text, "<br />", "");
			replace_all(text, "<span>", "");
			replace_all(text, "</span>", "");
			replace_all(text, "<div>", "\n");

			replace_all(text, "<p>", "\n");

			replace_all(text, "&gt;", ">");
			replace_all(text, "&lt;", "<");

			//Convert from generic format into HTML hyperlinks.
			size_t pos1,pos2;
			string link;
			pos1 = text.find("<a href="); //Start of a link
			while (pos1 != string::npos) {
				text.replace(pos1,8,"");
				pos1 = text.find(">",pos1);
				pos2 = text.find("</a>", pos1);
				text.replace(pos1, pos2-pos1+4,"");
				pos1 = text.find("<a href=",pos1);
			}
		}
		bosslog << text;
	}

	void OutputFooter() {
		if (log_format == "html") {
			bosslog << "</body>\n</html>";
		}
	}
}