/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011  Random/Random007/jpearce, WrinklyNinja & the BOSS 
	development team. Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "Output/Output.h"
#include "Common/Error.h"
#include "Support/Helpers.h"
#include <boost/algorithm/string.hpp>

namespace boss {
	using namespace std;
	using boost::algorithm::replace_all;

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
	BOSS_COMMON_EXP string CSSDarkBody			= "color:white;background:black;";
	BOSS_COMMON_EXP string CSSDarkLink			= "color:#0AF;";
	BOSS_COMMON_EXP string CSSDarkLinkVisited	= "color:#E000E0;";
	BOSS_COMMON_EXP string CSSFilters			= "border:1px gray dashed;background:#F5F5F5;padding:.3em;display:table;";
	BOSS_COMMON_EXP string CSSFiltersList		= "display:inline-block;padding:.2em .5em;white-space:nowrap;margin:0;width:200px;";
	BOSS_COMMON_EXP string CSSDarkFilters		= "border:1px gray dashed;padding:.3em;display:table;background:#333333;";
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

	Outputter::Outputter() {
		outFormat = PLAINTEXT;
		escapeHTMLSpecialChars = false;
	}

	Outputter::Outputter(unsigned int format) {
		outFormat = format;
		if (outFormat == HTML)
			escapeHTMLSpecialChars = true;
		else
			escapeHTMLSpecialChars = false;
	}

	void Outputter::SetFormat(unsigned int format) {
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
				<< ".tagPrefix{" << CSSTagPrefix << "}"
				<< ".dirty{" << CSSDirty << "}"
				<< ".message{" << CSSQuotedMessage << "}"
				<< ".mod{" << CSSMod << "}.tag{" << CSSTag << "}.note{" << CSSNote << "}.req{" << CSSRequirement << "}.inc{" << CSSIncompatibility << "}"
				<< "</style>"<<endl;
			outStream << "<div>Better Oblivion Sorting Software Log</div>" << endl
				<< "<div>&copy; Random007, WrinklyNinja &amp; the BOSS development team, 2011. Some rights reserved.<br />" << endl
				<< "<a href=\"http://creativecommons.org/licenses/by-nc-nd/3.0/\">CC Attribution-Noncommercial-No Derivative Works 3.0</a><br />" << endl
				<< "v" << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH) << " (" << boss_release_date << ")</div>";
		} else
			outStream << endl << "Better Oblivion Sorting Software Log" << endl
				<< "Copyright Random007, WrinklyNinja & the BOSS development team, 2011. Some rights reserved." << endl
				<< "License: CC Attribution-Noncommercial-No Derivative Works 3.0" << endl
				<< "(http://creativecommons.org/licenses/by-nc-nd/3.0/)" << endl
				<< "v" << IntToString(BOSS_VERSION_MAJOR) << "." << IntToString(BOSS_VERSION_MINOR) << "." << IntToString(BOSS_VERSION_PATCH) << " (" << boss_release_date << ")" << endl;
	}

	void Outputter::PrintFooter() {
		if (outFormat == HTML) {
			outStream << endl << "<script>" << endl
			
				<< "var hm=0,hp=0,hpe=document.getElementById('hp'),hme=document.getElementById('hm');" << endl
				<< "function toggleSectionDisplay(h){if(h.nextSibling.style.display=='none'){h.nextSibling.style.display='block';h.firstChild.innerHTML='&#x2212;'}else{h.nextSibling.style.display='none';h.firstChild.innerHTML='+'}}" << endl
				<< "function swapColorScheme(b){var d=document.body,a=document.getElementsByTagName('a'),f=document.getElementById('filters');if(f==null){f=document.getElementById('darkFilters')}if(b.checked){d.id='darkBody';f.id='darkFilters';for(var i=0,z=a.length;i<z;i++){a[i].className='darkLink'}}else{d.id='';f.id='filters';for(var i=0,z=a.length;i<z;i++){a[i].className=''}}}" << endl
				<< "function toggleDisplayCSS(b,s){var r=new Array();if(document.styleSheets[0].cssRules){r=document.styleSheets[0].cssRules}else if(document.styleSheets[0].rules){r=document.styleSheets[0].rules}for(var i=0,z=r.length;i<z;i++){if(r[i].selectorText.toLowerCase()==s){if(b.checked){r[i].style.display='none'}else{r[i].style.display='inline'}return}}}" << endl
				<< "function toggleRuleListWarnings(b){var u=document.getElementById('userlistMessages');if(u!=null){u=u.childNodes;for(var i=0,z=u.length;i<z;i++){if(u[i].className=='warn'){if(b.checked){u[i].style.display='none';hm++}else if(u[i].style.display=='none'){u[i].style.display='table';hm--}}}}hme.innerHTML=hm}" << endl
				<< "function toggleMessages(){var m=document.getElementById('recognised').childNodes,b9=document.getElementById('b9').checked,b10=document.getElementById('b10').checked,b11=document.getElementById('b11').checked,b12=document.getElementById('b12').checked,b13=document.getElementById('b13').checked,b14=document.getElementById('b14').checked;for(var i=0,z=m.length;i<z;i++){var a=m[i].getElementsByTagName('li');for(var j=0,y=a.length;j<y;j++){var b=null;if(!b9&&a[j].style.display=='none'){a[j].style.display='table';hm--}if(a[j].className=='note'){b=b10}else if(a[j].className=='tag'){b=b11}else if(a[j].className=='req'){b=b12}else if(a[j].className=='inc'){b=b13}else if(a[j].className=='dirty'&&a[j].innerHTML.substr(0,35)=='Contains dirty edits: Do not clean.'){b=b14}if(b!=null){if(b&&a[j].style.display!='none'){a[j].style.display='none';hm++}else if(!b&&a[j].style.display=='none'){a[j].style.display='table';hm--}}if(b9&&a[j].style.display!='none'){a[j].style.display='none';hm++}}}hme.innerHTML=hm}" << endl
				<< "function toggleMods(){var m=document.getElementById('recognised').childNodes;for(var i=0,z=m.length;i<z;i++){if(m[i].nodeType==1){var g=false,n=true,c=true,a=m[i].getElementsByTagName('span');for(var j=0,y=a.length;j<y;j++){if(a[j].className=='ghosted'){g=true;break}}a=m[i].getElementsByTagName('li');for(var j=0,y=a.length;j<y;j++){if(a[j].className=='dirty'){c=false}if(a[j].style.display!='none'){n=false}}if(!((document.getElementById('b6').checked&&n)||(document.getElementById('b7').checked&&g)||(document.getElementById('b8').checked&&c))&&m[i].style.display=='none'){m[i].style.display='block';hp--}else if(((document.getElementById('b6').checked&&n)||(document.getElementById('b7').checked&&g)||(document.getElementById('b8').checked&&c))&&m[i].style.display!='none'){m[i].style.display='none';hp++}}}hpe.innerHTML=hp}" << endl
				<< "function initialSetup() {"<<endl
				<< "	swapColorScheme(document.getElementById('b1'));"<<endl
				<< "	toggleRuleListWarnings(document.getElementById('b2'));"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b3'),'.version','inline');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b4'),'.ghosted','inline');"<<endl
				<< "	toggleDisplayCSS(document.getElementById('b5'),'.crc','inline');"<<endl
				<< "	toggleMessages();"<<endl
				<< "	toggleMods();"<<endl
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
	
	Outputter& Outputter::operator<< (const int i) {
		outStream << i;
		return *this;
	}
	
	Outputter& Outputter::operator<< (const unsigned int i) {
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
		string data = EscapeHTMLSpecial(m.data);
		//If bosslog format is HTML, wrap web addresses in HTML link format.
		if (log_format == HTML) {
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
	string ParsingError::FormatFor(const unsigned int format) {
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
}