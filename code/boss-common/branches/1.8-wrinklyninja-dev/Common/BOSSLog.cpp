/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "BOSSLog.h"
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/karma.hpp>

namespace boss {
	using namespace std;
	namespace karma = boost::spirit::karma;
	using boost::algorithm::replace_all;
	using boost::algorithm::replace_first;


	void ShowMessage(ofstream &log, string format, message currentMessage) {
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
			Output(log, format, "<li class='tag'><span class='tagPrefix'>Bash Tag suggestion(s):</span> " + currentMessage.data + "</li>\n");
			break;
		case SAY:
			Output(log, format, "<li class='note'>Note: " + currentMessage.data + "</li>\n");
			break;
		case REQ:
			Output(log, format, "<li class='req'>Requires: " + currentMessage.data + "</li>\n");
			break;
		case INC:
			Output(log, format, "<li class='inc'>Incompatible with: " + currentMessage.data + "</li>\n");
			break;
		case WARN:
			Output(log, format, "<li class='warn'>Warning: " + currentMessage.data + "</li>\n");
			break;
		case ERR:
			Output(log, format, "<li class='error'>ERROR: " + currentMessage.data + "</li>\n");
			break;
		case DIRTY:
			Output(log, format, "<li class='dirty'>Contains dirty edits: " + currentMessage.data + "</li>\n");
			break;
		default:
			Output(log, format, "<li class='note'>Note: " + currentMessage.data + "</li>\n");
			break;
		}
	}

	//Prints header if format is HTML, else nothing.
	void OutputHeader(ofstream &log) {
		log << "<!DOCTYPE html>"<<endl<<"<html>"<<endl<<"<head>"<<endl<<"<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>"<<endl
			<< "<title>BOSS Log</title>"<<endl<<"<style type='text/css'>"<<endl
			<< "body {font-family:Calibri,Arial,sans-serifs;}"<<endl
			<< ".filters {border:1px grey dashed; background:#F5F5F5; padding:0.3em; display:table;}"<<endl
			<< ".filters > li {display:inline-block; padding:0.2em 1em; white-space:nowrap; margin:0;}"<<endl
			<< "body > div:first-child {font-size:2.4em; font-weight:bold; text-align:center; margin-bottom:0.2em;}"<<endl
			<< "body > div:first-child + div {text-align:center;}"<<endl
			<< "body > div {margin-bottom:4em;}"<<endl
			<< "body > div > span:first-child {font-weight:bold; font-size:1.3em; cursor:pointer;}"<<endl
			<< "body > div > span:first-child > span {display:inline-block; position:relative; top:0.05em; font-size:1.30em; width:0.6em; margin-right:0.1em;}"<<endl			
			<< "div > ul {padding-left:0; margin-top:1em;}"<<endl
			<< "body > div:last-child {margin:0;}"<<endl
			<< "body > div:last-child > span:first-child {cursor:default;}"<<endl
			<< "div > ul > li {margin-left:0; margin-bottom:2em;}"<<endl
			<< "ul {list-style:none;}"<<endl
			<< "ul li {margin-bottom:0.4em;}"<<endl
			<< "li ul {margin-top:0.4em;}"<<endl
			<< "input[type='checkbox'] {position:relative; top:0.15em; margin-right:0.5em;}"<<endl
			<< "blockquote {font-style:italic;}"<<endl
			<< "#unrecognised > li {margin-bottom:1em;}"<<endl
			<< ".error {background:red; color:white; display:table; padding:0 4px;}"<<endl
			<< ".warn {background:orange; color:white; display:table; padding:0 4px;}"<<endl
			<< ".success {color:green;}"<<endl
			<< ".version {color:#6394F8; margin-left:1.3em; padding:0 4px;}"<<endl
			<< ".ghosted {color:#888888; margin-left:1.3em; padding:0 4px;}"<<endl
			<< ".crc {color:#BC8923; margin-left:1.3em; padding:0 4px;}"<<endl
			<< ".tagPrefix {color:#CD5555;}"<<endl
			<< ".dirty {color:#996600;}"<<endl
			<< ".message {color:grey;}"<<endl
			<< ".mod{} .tag{} .note{} .req{} .inc{}"<<endl
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
			<< "</script>"<<endl
			<< "</head><body>"<<endl;
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
	void Output(ofstream &log, string format, string text) {
		if (format == "text") {
			//Yes. This really is as horrific as it looks. It should be only temporary though.
			replace_first(text, "</body>\n</html>", "");
			replace_first(text, "&copy;", "c");
			replace_first(text, "&amp;", "&");
			replace_first(text, "&#x2212;", "");
			
			
			replace_first(text, "</ul>", "");
			replace_first(text, "<b>", "");
			replace_first(text, "</b>", "");
			replace_first(text, "<i>", "");
			replace_first(text, "</i>", "");

			replace_first(text, "<span>", "======================================\n");

			replace_first(text, " class='warn'>", ">");
			replace_all(text, " class='error'>", ">");
			replace_first(text, " class='success'>", ">");
			replace_first(text, " class='tag'>", ">");
			replace_first(text, " class='ghosted'>", ">  ");
			replace_first(text, " class='version'>", ">  ");
			replace_first(text, " class='dirty'>", ">");
			replace_first(text, " class='crc'>", ">  ");
			replace_first(text, " class='note'>", ">");
			replace_first(text, " class='req'>", ">");
			replace_first(text, " class='inc'>", ">");
			replace_first(text, " class='tagPrefix'>", ">");
			replace_first(text, "<li><span class='mod'>","");

			replace_first(text, " onclick='toggleSectionDisplay(this)'>", ">");
			replace_first(text, " id='seplugins'>", ">");
			replace_first(text, " id='recognised'>", ">");
			replace_first(text, " id='unrecognised'>", ">");
			replace_first(text, " id='userlistMessages'>", ">");

			replace_first(text, "<blockquote>", "\n\n");
			replace_first(text, "</blockquote>", "\n\n");
			replace_first(text, "<ul>", "");
			
			replace_first(text, "<li>", "*  ");
			replace_first(text, "</li>", "");
			
			replace_all(text, "</div>", "");
			replace_first(text, "</p>", "");	

			replace_all(text, "<br />", "");
			replace_all(text, "<span>", "");
			replace_all(text, "</span>", "");
			replace_all(text, "<div>", "\n");

			replace_all(text, "<p>", "\n");

			replace_first(text, "&gt;", ">");
			replace_first(text, "&lt;", "<");

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
		log << text;
	}
}