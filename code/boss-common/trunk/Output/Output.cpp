/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "Output/Output.h"
#include "Common/Globals.h"
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
	bool HideCleanMods			= 0;
	bool HideRuleWarnings       = 0;
	bool HideAllModMessages     = 0;
	bool HideNotes              = 0;
	bool HideBashTagSuggestions = 0;
	bool HideRequirements       = 0;
	bool HideIncompatibilities  = 0;

	//Default CSS.
	string CSSBody				= "font-family:Calibri,Arial,sans-serifs;";
	string CSSFilters			= "border:1px gray dashed;background:#F5F5F5;padding:.3em;display:table;";
	string CSSFiltersList		= "display:inline-block;padding:.2em .5em;white-space:nowrap;margin:0;width:200px;";
	string CSSTitle				= "font-size:2.4em;font-weight:700;text-align:center;margin-bottom:.2em;";
	string CSSCopyright			= "text-align:center;";
	string CSSSections			= "margin-bottom:3em;";
	string CSSSectionTitle		= "cursor:pointer;";
	string CSSSectionPlusMinus	= "display:inline-block;position:relative;top:.05em; font-size:1.3em;width:.6em;margin-right:.1em;";
	string CSSLastSection		= "text-align:center;cursor:default;";
	string CSSTable				= "padding:0 .5em;";
	string CSSList				= "list-style:none;padding-left:0;";
	string CSSListItem			= "margin-left:0;margin-bottom:1em;";
	string CSSSubList			= "margin-top:.5em;padding-left:2.5em;margin-bottom:2em;";
	string CSSCheckbox			= "position:relative;top:.15em;margin-right:.5em;";
	string CSSBlockquote		= "font-style:italic;";
	string CSSError				= "background:red;color:#FFF;display:table;padding:0 4px;";
	string CSSWarning			= "background:orange;color:#FFF;display:table;padding:0 4px;";
	string CSSSuccess			= "color:green;";
	string CSSVersion			= "color:#6394F8;margin-right:1em;padding:0 4px;";
	string CSSGhost				= "color:#888;margin-right:1em;padding:0 4px;";
	string CSSCRC				= "color:#BC8923;margin-right:1em;padding:0 4px;";
	string CSSTagPrefix			= "color:#CD5555;";
	string CSSDirty				= "color:#960;";
	string CSSQuotedMessage		= "color:gray;";
	string CSSMod				= "margin-right:1em;";
	string CSSTag				= "";
	string CSSNote				= "";
	string CSSRequirement		= "";
	string CSSIncompatibility	= "";

	void ShowMessage(string& buffer, message currentMessage) {
		currentMessage.data = EscapeHTMLSpecial(currentMessage.data);
		//If bosslog format is HTML, wrap web addresses in HTML link format.
		if (log_format == "html") {
			size_t pos1,pos2;
			string link;
			pos1 = currentMessage.data.find("&quot;http");  //Start of a link, HTML escaped.
			while (pos1 != string::npos) {
				pos1 += 6;  //Now points to start of actual link.
				pos2 = currentMessage.data.find("&quot;",pos1);  //First character after the end of the link.
				link = currentMessage.data.substr(pos1,pos2-pos1);
				link = "<a href=\"" + link + "\">" + link + "</a>";
				currentMessage.data.replace(pos1-6,pos2-pos1+12,link);
				pos1 = currentMessage.data.find("&quot;http",pos1 + link.length());
			}
		}
		//Select message formatting.
		switch(currentMessage.key) {
		case TAG:
			buffer += "<li class='tag'><span class='tagPrefix'>Bash Tag suggestion(s):</span> " + currentMessage.data + "";
			break;
		case SAY:
			buffer += "<li class='note'>Note: " + currentMessage.data + "";
			break;
		case REQ:
			buffer += "<li class='req'>Requires: " + currentMessage.data + "";
			break;
		case INC:
			buffer += "<li class='inc'>Incompatible with: " + currentMessage.data + "";
			break;
		case WARN:
			buffer += "<li class='warn'>Warning: " + currentMessage.data + "";
			break;
		case ERR:
			buffer += "<li class='error'>ERROR: " + currentMessage.data + "";
			break;
		case DIRTY:
			buffer += "<li class='dirty'>Contains dirty edits: " + currentMessage.data + "";
			break;
		default:
			buffer += "<li class='note'>Note: " + currentMessage.data + "";
			break;
		}
	}

	//Prints header if format is HTML, else nothing.
	void OutputHeader() {
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
		Output("v"+g_version+" ("+g_releaseDate+")</div>");
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

	//Converts a boolean to a string representation (true/false)
	string BoolToString(bool b) {
		if (b)
			return "true";
		else
			return "false";
	}

	//Prints ouptut with formatting according to output format.
	void Output(string text) {
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
	}
	
	//Escapes HTML special characters.
	string EscapeHTMLSpecial(string text) {
		if (log_format == "html") {
			replace_all(text, "&", "&amp;");
			replace_all(text, "\"", "&quot;");
			replace_all(text, "'", "&#039;");
			replace_all(text, "<", "&lt;");
			replace_all(text, ">", "&gt;");
		}
		return text;
	}

	void OutputFooter() {
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
				<< "f.background='#F5F5F5'}}function toggleUserlistWarnings(b){var "
				<< "u=document.getElementById('userlistMessages').childNodes;if(u){for(var i=0,z=u.length;"
				<< "i<z;i++){if(u[i].className=='warn'){if(b.checked){u[i].style.display='none'}else{"
				<< "u[i].style.display='table'}}}}}function toggleMods(){var "
				<< "m=document.getElementById('recognised').childNodes;for(var i=0,z=m.length;i<l;i++){"
				<< "if(m[i].nodeType==1){var g=false,n=true,c=true,a=m[i].getElementsByTagName('span');"
				<< "for(var j=0,y=a.length;j<y;j++){if(a[j].className=='ghosted'){g=true;break}}"
				<< "a=m[i].getElementsByTagName('li');if(a){var p;if(window.getComputedStyle){"
				<< "p=window.getComputedStyle(a[0].parentNode,null).getPropertyValue('display')}else if(a[0].currentStyle){"
				<< "p=a[0].parentNode.currentStyle.display}for(var j=0,y=a.length;j<y;j++){if(a[j].className=='dirty'){"
				<< "c=false}var b;if(window.getComputedStyle){b=window.getComputedStyle(a[j],null).getPropertyValue('display')"
				<< "}else if(a[j].currentStyle){b=a[j].currentStyle.display}if(p!='none'&&b!='none'){n=false}}}"
				<< "if((document.getElementById('noMessageModFilter').checked&&n)||"
				<< "(document.getElementById('ghostModFilter').checked&&g)||(document.getElementById('cleanModFilter').checked&&c))"
				<< "{m[i].style.display='none'}else{m[i].style.display='block'}}}}"<<endl
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
				<< "</script>"<<endl;
		}
	}

	string GetGameIniString() {
		if (game == 0)
			return "auto";
		else if (game == 1)
			return "Oblivion";
		else if (game == 2)
			return "Fallout3";
		else if (game == 3)
			return "Nehrim";
		else if (game == 4)
			return "FalloutNV";
		else if (game == 5)
			return "Skyrim";
		else
			return "";
	}

	//Generate a default BOSS.ini
	bool GenerateIni() {
		ofstream ini(ini_path.c_str(), ios_base::trunc);
		if (ini.fail())
			return false;
		ini <<  '\xEF' << '\xBB' << '\xBF'  //Write UTF-8 BOM to ensure the file is recognised as having the UTF-8 encoding.
			<<	"#---------------" << endl
			<<	"# BOSS Ini File" << endl
			<<	"#---------------" << endl
			<<	"# Settings with names starting with 'b' are boolean and accept values of 'true' or 'false'." << endl
			<<	"# Settings with names starting with 'i' are unsigned integers and accept varying ranges of whole numbers." << endl
			<<	"# Settings with names starting with 's' are strings and their accepted values vary." << endl
			<<	"# See the BOSS ReadMe for details on what each setting does and the accepted values for integer and string settings." << endl << endl

			<<	"[BOSS.GeneralSettings]" << endl
			<<	"bDoStartupUpdateCheck    = " << BoolToString(do_startup_update_check) << endl << endl

			<<	"[BOSS.InternetSettings]" << endl
			<<	"sProxyType               = " << proxy_type << endl
			<<	"sProxyHostname           = " << proxy_host << endl
			<<	"iProxyPort               = " << proxy_port << endl << endl

			<<	"[BOSS.RunOptions]" << endl
			<<	"sGame                    = " << GetGameIniString() << endl
			<<	"sBOSSLogFormat           = " << log_format << endl
			<<	"iRunType                 = " << IntToString(run_type) << endl
			<<	"iDebugVerbosity          = " << IntToString(verbosity) << endl
			<<	"iRevertLevel             = " << IntToString(revert) << endl
			<<	"bUpdateMasterlist        = " << BoolToString(update) << endl
			<<	"bOnlyUpdateMasterlist    = " << BoolToString(update_only) << endl
			<<	"bSilentRun               = " << BoolToString(silent) << endl
			<<	"bNoVersionParse          = " << BoolToString(skip_version_parse) << endl
			<<	"bDebugWithSourceRefs     = " << BoolToString(debug) << endl
			<<	"bDisplayCRCs             = " << BoolToString(show_CRCs) << endl
			<<	"bDoTrialRun              = " << BoolToString(trial_run) << endl
			<<	"bRecordDebugOutput       = " << BoolToString(record_debug_output) << endl << endl

			<<	"[BOSSLog.Filters]" << endl
			<<	"bUseDarkColourScheme     = " << BoolToString(UseDarkColourScheme) << endl
			<<	"bHideVersionNumbers      = " << BoolToString(HideVersionNumbers) << endl
			<<	"bHideGhostedLabel        = " << BoolToString(HideGhostedLabel) << endl
			<<	"bHideChecksums           = " << BoolToString(HideChecksums) << endl
			<<	"bHideMessagelessMods     = " << BoolToString(HideMessagelessMods) << endl
			<<	"bHideGhostedMods         = " << BoolToString(HideGhostedMods) << endl
			<<	"bHideCleanMods           = " << BoolToString(HideCleanMods) << endl
			<<	"bHideRuleWarnings        = " << BoolToString(HideRuleWarnings) << endl
			<<	"bHideAllModMessages      = " << BoolToString(HideAllModMessages) << endl
			<<	"bHideNotes               = " << BoolToString(HideNotes) << endl
			<<	"bHideBashTagSuggestions  = " << BoolToString(HideBashTagSuggestions) << endl
			<<	"bHideRequirements        = " << BoolToString(HideRequirements) << endl
			<<	"bHideIncompatibilities   = " << BoolToString(HideIncompatibilities) << endl << endl

			<<	"[BOSSLog.Styles]" << endl
			<<	"# A style with nothing specified uses the coded defaults." << endl
			<<	"# These defaults are given in the BOSS ReadMe as with the rest of the ini settings." << endl
			<<	"\"body\"                                     = " << CSSBody << endl
			<<	"\"#filters\"                                 = " << CSSFilters << endl
			<<	"\"#filters > li\"                            = " << CSSFiltersList << endl
			<<	"\"body > div:first-child\"                   = " << CSSTitle << endl
			<<	"\"body > div:first-child + div\"             = " << CSSCopyright << endl
			<<	"\"h3 + *\"                                   = " << CSSSections << endl
			<<	"\"h3\"                                       = " << CSSSectionTitle << endl
			<<	"\"h3 > span\"                                = " << CSSSectionPlusMinus << endl
			<<	"\"#end\"                                     = " << CSSLastSection << endl
			<<	"\"td\"                                       = " << CSSTable << endl
			<<	"\"ul\"                                       = " << CSSList << endl
			<<	"\"ul li\"                                    = " << CSSListItem << endl
			<<	"\"li ul\"                                    = " << CSSSubList << endl
			<<	"\"input[type='checkbox']\"                   = " << CSSCheckbox << endl
			<<	"\"blockquote\"                               = " << CSSBlockquote << endl
			<<	"\".error\"                                   = " << CSSError << endl
			<<	"\".warn\"                                    = " << CSSWarning << endl
			<<	"\".success\"                                 = " << CSSSuccess << endl
			<<	"\".version\"                                 = " << CSSVersion << endl
			<<	"\".ghosted\"                                 = " << CSSGhost << endl
			<<	"\".crc\"                                     = " << CSSCRC << endl
			<<	"\".tagPrefix\"                               = " << CSSTagPrefix << endl
			<<	"\".dirty\"                                   = " << CSSDirty << endl
			<<	"\".message\"                                 = " << CSSQuotedMessage << endl
			<<	"\".mod\"                                     = " << CSSMod << endl
			<<	"\".tag\"                                     = " << CSSTag << endl
			<<	"\".note\"                                    = " << CSSNote << endl
			<<	"\".req\"                                     = " << CSSRequirement << endl
			<<	"\".inc\"                                     = " << CSSIncompatibility;
		ini.close();
		return true;
	}
}