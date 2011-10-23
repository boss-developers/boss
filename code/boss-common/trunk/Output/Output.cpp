/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "Output/Output.h"
#include "Common/Globals.h"
#include "Common/Error.h"
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/karma.hpp>

namespace boss {
	using namespace std;
	namespace karma = boost::spirit::karma;
	using boost::algorithm::replace_all;
	using boost::algorithm::replace_first;

	//Default filter options.
	BOSS_COMMON_EXP bool UseDarkColourScheme    = false;
	BOSS_COMMON_EXP bool HideVersionNumbers     = false;
	BOSS_COMMON_EXP bool HideGhostedLabel       = false;
	BOSS_COMMON_EXP bool HideChecksums          = false;
	BOSS_COMMON_EXP bool HideMessagelessMods    = false;
	BOSS_COMMON_EXP bool HideGhostedMods        = false;
	BOSS_COMMON_EXP bool HideCleanMods			= false;
	BOSS_COMMON_EXP bool HideRuleWarnings       = false;
	BOSS_COMMON_EXP bool HideAllModMessages     = false;
	BOSS_COMMON_EXP bool HideNotes              = false;
	BOSS_COMMON_EXP bool HideBashTagSuggestions = false;
	BOSS_COMMON_EXP bool HideRequirements       = false;
	BOSS_COMMON_EXP bool HideIncompatibilities  = false;
	BOSS_COMMON_EXP bool HideDoNotCleanMessages	= false;

	//Default CSS.
	BOSS_COMMON_EXP string CSSBody				= "font-family:Calibri,Arial,sans-serifs;";
	BOSS_COMMON_EXP string CSSFilters			= "border:1px gray dashed;background:#F5F5F5;padding:.3em;display:table;";
	BOSS_COMMON_EXP string CSSFiltersList		= "display:inline-block;padding:.2em .5em;white-space:nowrap;margin:0;width:200px;";
	BOSS_COMMON_EXP string CSSTitle				= "font-size:2.4em;font-weight:700;text-align:center;margin-bottom:.2em;";
	BOSS_COMMON_EXP string CSSCopyright			= "text-align:center;";
	BOSS_COMMON_EXP string CSSSections			= "margin-bottom:3em;";
	BOSS_COMMON_EXP string CSSSectionTitle		= "cursor:pointer;";
	BOSS_COMMON_EXP string CSSSectionPlusMinus	= "display:inline-block;position:relative;top:.05em; font-size:1.3em;width:.6em;margin-right:.1em;";
	BOSS_COMMON_EXP string CSSLastSection		= "text-align:center;cursor:default;";
	BOSS_COMMON_EXP string CSSTable				= "padding:0 .5em;";
	BOSS_COMMON_EXP string CSSList				= "list-style:none;padding-left:0;";
	BOSS_COMMON_EXP string CSSListItem			= "margin-left:0;margin-bottom:1em;";
	BOSS_COMMON_EXP string CSSSubList			= "margin-top:.5em;padding-left:2.5em;margin-bottom:2em;";
	BOSS_COMMON_EXP string CSSCheckbox			= "position:relative;top:.15em;margin-right:.5em;";
	BOSS_COMMON_EXP string CSSBlockquote		= "font-style:italic;";
	BOSS_COMMON_EXP string CSSError				= "background:red;color:#FFF;display:table;padding:0 4px;";
	BOSS_COMMON_EXP string CSSWarning			= "background:orange;color:#FFF;display:table;padding:0 4px;";
	BOSS_COMMON_EXP string CSSSuccess			= "color:green;";
	BOSS_COMMON_EXP string CSSVersion			= "color:#6394F8;margin-right:1em;padding:0 4px;";
	BOSS_COMMON_EXP string CSSGhost				= "color:#888;margin-right:1em;padding:0 4px;";
	BOSS_COMMON_EXP string CSSCRC				= "color:#BC8923;margin-right:1em;padding:0 4px;";
	BOSS_COMMON_EXP string CSSTagPrefix			= "color:#CD5555;";
	BOSS_COMMON_EXP string CSSDirty				= "color:#960;";
	BOSS_COMMON_EXP string CSSQuotedMessage		= "color:gray;";
	BOSS_COMMON_EXP string CSSMod				= "margin-right:1em;";
	BOSS_COMMON_EXP string CSSTag				= "";
	BOSS_COMMON_EXP string CSSNote				= "";
	BOSS_COMMON_EXP string CSSRequirement		= "";
	BOSS_COMMON_EXP string CSSIncompatibility	= "";

	void ShowMessage(string& buffer, Message currentMessage) {
		string data = EscapeHTMLSpecial(currentMessage.data);
		//If bosslog format is HTML, wrap web addresses in HTML link format.
		if (log_format == "html") {
			size_t pos1,pos2;
			string link;
			pos1 = data.find("&quot;http");  //Start of a link, HTML escaped.
			while (pos1 != string::npos) {
				pos1 += 6;  //Now points to start of actual link.
				pos2 = data.find("&quot;",pos1);  //First character after the end of the link.
				link = data.substr(pos1,pos2-pos1);
				link = "<a href=\"" + link + "\">" + link + "</a>";
				data.replace(pos1-6,pos2-pos1+12,link);
				pos1 = data.find("&quot;http",pos1 + link.length());
			}
		}
		//Select message formatting.
		switch(currentMessage.key) {
		case TAG:
			buffer += "<li class='tag'><span class='tagPrefix'>Bash Tag suggestion(s):</span> " + data + "";
			break;
		case SAY:
			buffer += "<li class='note'>Note: " + data + "";
			break;
		case REQ:
			buffer += "<li class='req'>Requires: " + data + "";
			break;
		case INC:
			buffer += "<li class='inc'>Incompatible with: " + data + "";
			break;
		case WARN:
			buffer += "<li class='warn'>Warning: " + data + "";
			break;
		case ERR:
			buffer += "<li class='error'>ERROR: " + data + "";
			break;
		case DIRTY:
			buffer += "<li class='dirty'>Contains dirty edits: " + data + "";
			break;
		default:
			buffer += "<li class='note'>Note: " + data + "";
			break;
		}
	}

	//Prints header if format is HTML, else nothing.
	BOSS_COMMON_EXP void OutputHeader() {
		if (log_format == "html") {
			bosslog << "<!DOCTYPE html>"<<endl<<"<meta charset='utf-8'>"<<endl
				<< "<title>BOSS Log</title>"<<endl<<"<style>"
				<< "body{" << CSSBody << "}"
				<< "#filters{" << CSSFilters << "}"
				<< "#filters > li{" << CSSFiltersList << "}"
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
				<< ".tagPrefix{" << CSSTagPrefix << "}"
				<< ".dirty{" << CSSDirty << "}"
				<< ".message{" << CSSQuotedMessage << "}"
				<< ".mod{" << CSSMod << "}.tag{" << CSSTag << "}.note{" << CSSNote << "}.req{" << CSSRequirement << "}.inc{" << CSSIncompatibility << "}"
				<< "</style>"<<endl;
		}
		Output("<div>Better Oblivion Sorting Software Log</div>\n");
		Output("<div>&copy; Random007 &amp; the BOSS development team, 2011. Some rights reserved.<br />");
		Output("<a href=\"http://creativecommons.org/licenses/by-nc-nd/3.0/\">CC Attribution-Noncommercial-No Derivative Works 3.0</a><br />");
		Output("v"+IntToString(BOSS_VERSION_MAJOR)+"."+IntToString(BOSS_VERSION_MINOR)+"."+IntToString(BOSS_VERSION_PATCH)+" ("+boss_release_date+")</div>");
	}

	//Converts an integer to a string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
	BOSS_COMMON_EXP string IntToString(const unsigned int n) {
		string out;
		back_insert_iterator<string> sink(out);
		karma::generate(sink,karma::upper[karma::uint_],n);
		return out;
	}

	//Converts an integer to a hex string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
	string IntToHexString(const unsigned int n) {
		string out;
		back_insert_iterator<string> sink(out);
		karma::generate(sink,karma::upper[karma::hex],n);
		return out;
	}

	//Converts a boolean to a string representation (true/false)
	string BoolToString(bool b) {
		if (b)
			return "true";
		else
			return "false";
	}

	//Prints ouptut with formatting according to output format.
	BOSS_COMMON_EXP void Output(string text) {
		if (log_format == "text") {
			//Yes. This really is as horrific as it looks. It should be only temporary though.
			replace_first(text, "&copy;", "(c)");
			replace_first(text, "&amp;", "&");
			replace_first(text, "&#x2212;", "");
			replace_first(text, "<tr>","");
			replace_first(text, "<table><tbody>","");
			replace_first(text, "</table>","");
			replace_first(text, "<div>", "\n");

			replace_first(text, " onclick='toggleSectionDisplay(this)'>", ">");
			replace_first(text, " id='recognised'>", ">");
			replace_first(text, " id='userlistMessages'>", ">");
			replace_first(text, " id='end'>", ">");

			replace_all(text, "</ul>", "\n");
			replace_all(text, "<li><span class='mod'>","\n\n");
			replace_all(text, "<td>","\n");
			replace_all(text, "&nbsp;", " ");
			replace_all(text, "<br />", "\n");
			replace_all(text, "<blockquote>", "\n\n");
			replace_all(text, "</blockquote>", "\n\n");
			replace_all(text, "</div>", ""); 
			replace_all(text, "</span>", "");
			
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
			replace_all(text, " class='message'>", ">");

			replace_all(text, "<ul>", "");
			replace_all(text, "<li>", "\n*  ");
			replace_all(text, "<span>", "");
			replace_all(text, "<p>", "\n");
			
			replace_first(text, "<h3>", "\n\n======================================\n");
			replace_first(text, "</h3>", "\n======================================\n\n");

			//Convert from HTML hyperlinks into text string.
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
		if (log_format == "html")
			bosslog << endl;
	}
	
	//Escapes HTML special characters.
	BOSS_COMMON_EXP string EscapeHTMLSpecial(string text) {
		if (log_format == "html") {
			replace_all(text, "&", "&amp;");
			replace_all(text, "\"", "&quot;");
			replace_all(text, "'", "&#039;");
			replace_all(text, "<", "&lt;");
			replace_all(text, ">", "&gt;");
		}
		return text;
	}

	BOSS_COMMON_EXP void OutputFooter() {
		if (log_format == "html") {
			bosslog << "<script>"
				<< "function toggleSectionDisplay(h){if(h.nextSibling.style.display=='none')"
				<< "{h.nextSibling.style.display='block';h.firstChild.innerHTML='&#x2212;'}else{"
				<< "h.nextSibling.style.display='none';h.firstChild.innerHTML='+'}}function "
				<< "toggleDisplayCSS(b,s,d){var r=new Array();if(document.styleSheets[0].cssRules){"
				<< "r=document.styleSheets[0].cssRules}else if(document.styleSheets[0].rules){"
				<< "r=document.styleSheets[0].rules}for(var i=0,z=r.length;i<z;i++){"
				<< "if(r[i].selectorText.toLowerCase()==s){if(b.checked){r[i].style.display='none'"
				<< "}else{r[i].style.display=d}return}}}function swapColorScheme(b){var d=document.body.style;"
				<< "var f=document.getElementById('filters').style;if(b.checked){d.color='white';"
				<< "d.background='black';f.background='#333333'}else{d.color='black';d.background='white';"
				<< "f.background='#F5F5F5'}}function toggleRuleListWarnings(b){var "
				<< "u=document.getElementById('userlistMessages').childNodes;if(u){for(var i=0,z=u.length;"
				<< "i<z;i++){if(u[i].className=='warn'){if(b.checked){u[i].style.display='none'}else{"
				<< "u[i].style.display='table'}}}}}function toggleMods(){var "
				<< "m=document.getElementById('recognised').childNodes;for(var i=0,z=m.length;i<z;i++){"
				<< "if(m[i].nodeType==1){var g=false,n=true,c=true,a=m[i].getElementsByTagName('span');"
				<< "for(var j=0,y=a.length;j<y;j++){if(a[j].className=='ghosted'){g=true;break}}"
				<< "a=m[i].getElementsByTagName('li');if(a.length>0){var p;if(window.getComputedStyle){"
				<< "p=window.getComputedStyle(a[0].parentNode,null).getPropertyValue('display')}else if(a[0].currentStyle){"
				<< "p=a[0].parentNode.currentStyle.display}for(var j=0,y=a.length;j<y;j++){if(a[j].className=='dirty'){"
				<< "c=false}var b;if(window.getComputedStyle){b=window.getComputedStyle(a[j],null).getPropertyValue('display')"
				<< "}else if(a[j].currentStyle){b=a[j].currentStyle.display}if(p!='none'&&b!='none'){n=false}}}"
				<< "if((document.getElementById('noMessageModFilter').checked&&n)||"
				<< "(document.getElementById('ghostModFilter').checked&&g)||(document.getElementById('cleanModFilter').checked&&c))"
				<< "{m[i].style.display='none'}else{m[i].style.display='block'}}}}function toggleDoNotClean(b,s){"
				<< "var m=document.getElementById('recognised').childNodes;for(var i=0,z=m.length;i<z;i++){if(m[i].nodeType==1){"
				<< "var a=m[i].getElementsByTagName('li');for(var j=0,y=a.length;j<y;j++){if(a[j].className=='dirty'){"
				<< "if(a[j].firstChild.nodeType==3){if(a[j].firstChild.nodeValue.toString().substr(0,35)=='Contains dirty edits: "
				<< "Do not clean.'){if(b.checked){a[j].style.display='none'}else{a[j].style.display=s}}}}}}}}"<<endl
				<< "function initialSetup() {"<<endl
				<< "	swapColorScheme(document.getElementById('b1'));"<<endl
				<< "	toggleRuleListWarnings(document.getElementById('b12'));"<<endl
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
				<< "</script>"<<endl;
		}
	}
	/*
	void Output::Open(fs::path file, string format, bool shouldEscapeHTMLSpecial) {
		outFormat = format;
		escapeHTMLSpecialChars = shouldEscapeHTMLSpecial;
	
		outStream.open(file.c_str());
		if (outStream.fail())
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
	
		if (outFormat == "html") {
			outStream << "<!DOCTYPE html>"<<endl<<"<meta charset='utf-8'>"<<endl
				<< "<title>BOSS Log</title>"<<endl<<"<style>"
				<< "body{" << CSSBody << "}"
				<< "#filters{" << CSSFilters << "}"
				<< "#filters > li{" << CSSFiltersList << "}"
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
				<< ".tagPrefix{" << CSSTagPrefix << "}"
				<< ".dirty{" << CSSDirty << "}"
				<< ".message{" << CSSQuotedMessage << "}"
				<< ".mod{" << CSSMod << "}.tag{" << CSSTag << "}.note{" << CSSNote << "}.req{" << CSSRequirement << "}.inc{" << CSSIncompatibility << "}"
				<< "</style>"<<endl;
		}
		*this << DIV_OPEN << "Better Oblivion Sorting Software Log" << DIV_CLOSE
			<< DIV_OPEN << SYMBOL_COPYRIGHT << " Random007, WrinklyNinja " << SYMBOL_AMPERSAND << " the BOSS development team, 2011. Some rights reserved." << LINE_BREAK
			<< LINK_OPEN_ADDRESS << "http://creativecommons.org/licenses/by-nc-nd/3.0/" << LINK_CLOSE_ADDRESS << "CC Attribution-Noncommercial-No Derivative Works 3.0" << LINK_CLOSE << LINE_BREAK
			<< "v" << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH) << " (" << boss_release_date << ")" << DIV_CLOSE;
	}

	void Output::Close() {
		if (outFormat == "html") {
			outStream << "<script>"
				<< "function toggleSectionDisplay(h){if(h.nextSibling.style.display=='none')"
				<< "{h.nextSibling.style.display='block';h.firstChild.innerHTML='&#x2212;'}else{"
				<< "h.nextSibling.style.display='none';h.firstChild.innerHTML='+'}}function "
				<< "toggleDisplayCSS(b,s,d){var r=new Array();if(document.styleSheets[0].cssRules){"
				<< "r=document.styleSheets[0].cssRules}else if(document.styleSheets[0].rules){"
				<< "r=document.styleSheets[0].rules}for(var i=0,z=r.length;i<z;i++){"
				<< "if(r[i].selectorText.toLowerCase()==s){if(b.checked){r[i].style.display='none'"
				<< "}else{r[i].style.display=d}return}}}function swapColorScheme(b){var d=document.body.style;"
				<< "var f=document.getElementById('filters').style;if(b.checked){d.color='white';"
				<< "d.background='black';f.background='#333333'}else{d.color='black';d.background='white';"
				<< "f.background='#F5F5F5'}}function toggleRuleListWarnings(b){var "
				<< "u=document.getElementById('userlistMessages').childNodes;if(u){for(var i=0,z=u.length;"
				<< "i<z;i++){if(u[i].className=='warn'){if(b.checked){u[i].style.display='none'}else{"
				<< "u[i].style.display='table'}}}}}function toggleMods(){var "
				<< "m=document.getElementById('recognised').childNodes;for(var i=0,z=m.length;i<z;i++){"
				<< "if(m[i].nodeType==1){var g=false,n=true,c=true,a=m[i].getElementsByTagName('span');"
				<< "for(var j=0,y=a.length;j<y;j++){if(a[j].className=='ghosted'){g=true;break}}"
				<< "a=m[i].getElementsByTagName('li');if(a.length>0){var p;if(window.getComputedStyle){"
				<< "p=window.getComputedStyle(a[0].parentNode,null).getPropertyValue('display')}else if(a[0].currentStyle){"
				<< "p=a[0].parentNode.currentStyle.display}for(var j=0,y=a.length;j<y;j++){if(a[j].className=='dirty'){"
				<< "c=false}var b;if(window.getComputedStyle){b=window.getComputedStyle(a[j],null).getPropertyValue('display')"
				<< "}else if(a[j].currentStyle){b=a[j].currentStyle.display}if(p!='none'&&b!='none'){n=false}}}"
				<< "if((document.getElementById('noMessageModFilter').checked&&n)||"
				<< "(document.getElementById('ghostModFilter').checked&&g)||(document.getElementById('cleanModFilter').checked&&c))"
				<< "{m[i].style.display='none'}else{m[i].style.display='block'}}}}function toggleDoNotClean(b,s){"
				<< "var m=document.getElementById('recognised').childNodes;for(var i=0,z=m.length;i<z;i++){if(m[i].nodeType==1){"
				<< "var a=m[i].getElementsByTagName('li');for(var j=0,y=a.length;j<y;j++){if(a[j].className=='dirty'){"
				<< "if(a[j].firstChild.nodeType==3){if(a[j].firstChild.nodeValue.toString().substr(0,35)=='Contains dirty edits: "
				<< "Do not clean.'){if(b.checked){a[j].style.display='none'}else{a[j].style.display=s}}}}}}}}"<<endl
				<< "function initialSetup() {"<<endl
				<< "	swapColorScheme(document.getElementById('b1'));"<<endl
				<< "	toggleRuleListWarnings(document.getElementById('b12'));"<<endl
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
				<< "</script>"<<endl;
		}
		outStream.close();
	}

	void Output::SetFormat(string format) {
		outFormat = format;
	}

	void Output::SetHTMLSpecialEscape(bool shouldEscape) {
		escapeHTMLSpecialChars = shouldEscape;
	}

	//Escapes HTML special characters.
	string Output::EscapeHTMLSpecial(string text) {
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

	string Output::EscapeHTMLSpecial(char c) {
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

	Output& Output::operator << (Output& o, const string s) {
		outStream << EscapeHTMLSpecial(s);
		return *this;
	}

	Output& Output::operator << (Output& o, const char c) {
		outStream << EscapeHTMLSpecial(c);
		return o;
	}

	Output& Output::operator << (Output& o, const char * s) {
		outStream << EscapeHTMLSpecial(s);
		return o;
	}

	Output& Output::operator << (Output& o, logFormatting l) {
		if (outFormat == "html") {
			switch(l) {
			default:
				break;
			}
		} else {
			switch(l) {
			default:
				break;
			}
		}
		return o;
	}

	Output& Output::operator << (Output& o, unsigned int i) {
		outStream << i;
		return o;
	}

	Output& Output::operator << (Output& o, bool b) {
		if (b)
			outStream << "true";
		else
			outStream << "false";
		return o;
	}

	Output& Output::operator << (Output& o, fs::path p) {
		this << p.string();
		return o;
	}

	Output& Output::operator << (Output& o, Message m) {
		string data = EscapeHTMLSpecial(m.data);
		//If bosslog format is HTML, wrap web addresses in HTML link format.
		if (log_format == "html") {
			size_t pos1,pos2;
			string link;
			pos1 = data.find("&quot;http");  //Start of a link, HTML escaped.
			while (pos1 != string::npos) {
				pos1 += 6;  //Now points to start of actual link.
				pos2 = data.find("&quot;",pos1);  //First character after the end of the link.
				link = data.substr(pos1,pos2-pos1);
				link = "<a href=\"" + link + "\">" + link + "</a>";
				data.replace(pos1-6,pos2-pos1+12,link);
				pos1 = data.find("&quot;http",pos1 + link.length());
			}
		}
		//Select message formatting.
		switch(m.key) {
		case TAG:
			*this << "<li class='tag'><span class='tagPrefix'>" << "Bash Tag suggestion(s):" << "</span> " << data;
			break;
		case SAY:
			*this << "<li class='note'>" << "Note: " << data;
			break;
		case REQ:
			*this << "<li class='req'>" << "Requires: " << data;
			break;
		case INC:
			*this << "<li class='inc'>" << "Incompatible with: " << data;
			break;
		case WARN:
			*this << "<li class='warn'>" << "Warning: " << data;
			break;
		case ERR:
			*this << "<li class='error'>" << "ERROR: " << data;
			break;
		case DIRTY:
			*this << "<li class='dirty'>" << "Contains dirty edits: " << data;
			break;
		default:
			*this << "<li class='note'>" << "Note: " << data;
			break;
		}
		return o;
	}*/

		//Outputs correctly-formatted error message.
		string ParsingError::FormatFor(const string format) {
			if (!wholeMessage.empty()) {
				if (format == "html")
					return "<li class='error'>"+EscapeHTMLSpecial(wholeMessage);
				else
					return wholeMessage;
			} else {
				if (format == "html") {
					boost::replace_all(detail, "\n", "<br />");
					return "<li><span class='error'>"+EscapeHTMLSpecial(header)+"</span><blockquote>"+EscapeHTMLSpecial(detail)+"</blockquote><span class='error'>"+EscapeHTMLSpecial(footer)+"</span>";
				}else
					return "\n*  "+header+"\n\n"+detail+"\n\n"+footer;
			}
		}
}