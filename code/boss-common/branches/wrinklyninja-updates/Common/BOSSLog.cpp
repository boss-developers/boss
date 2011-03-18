/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "BOSSLog.h"
#include <boost/algorithm/string.hpp>

/*
BOSSLog output notes.

The generic symbols could be given in the form: {<type>}
Symbols for formatting which wraps text could be given as {<type>] ... [<type>}
Some wrapping formatting needs specific type identifiers (ie. classes, links). Perhaps: 
{<type>=<class>]
{a=<link>]
HTML element names are used because HTML is both precisely defined and is the default formatting.
Plaintext syntax is not derived straight from HTML tags because the generic syntax is simply shorter.

Elements:

|| HTML Syntax 			|| Plaintext Syntax || Generic Syntax 	||
//////////////////////////////////////////////////////////////////
|| <br />	   			|| \n			   	|| {br}			  	||
|| </body></html>		|| None				|| {end}			||
|| <div>...</div>		|| ...\n			|| {div]			||
|| <p>..</p>			|| ...\n\n			|| {p]				||
|| <ul>...</ul>			|| \n...\n			|| {ul]				||
|| <li>...</li>			|| * ...\n			|| {li]				||
|| <a href='.'>..</a>	|| .. (.)			|| {a="."]			||
|| <span>...</span>	 	|| ...				|| {span]			||
|| <b>...</b>			|| ...				|| {b]				||
|| <i>...</i>			|| ...				|| {i]				||


Only the opening general syntax is shown, all but {br} and {end} also have a closing counterpart.
Elements can be supplied with classes, these do nothing for plaintext output, but are used as CSS classes for HTML output.
Not sure how to translate non-class CSS into plaintext formatting yet.

Special characters:
|| Generic	 || Plaintext				|| HTML   ||
////////////////////////////////////////////////////
|| {c}		 || c						|| &copy; ||
|| {&}		 || &						|| &amp;  ||

The ouput strings can be passed to a function that will somehow convert the general syntax into format-specific syntax.
Spirit.Karma may be used. ATM it's just replace functions.

Links are a bit tricky. They are currently broken.
*/

namespace boss {
	using namespace std;
	using boost::algorithm::replace_all;
	using boost::algorithm::replace_first;

	void ShowMessage(ofstream &log, string format, message currentMessage) {
		size_t pos1,pos2;
		string link;
		//Wrap web addresses in generic link format. Skip those already in generic format.
		pos1 = currentMessage.data.find("\"http");  //Start of a link.
		while (pos1 != string::npos) {
			if (currentMessage.data[pos1-1] != '=') {  //Link is (probably) not in generic format.
				pos2 = currentMessage.data.find("\"",pos1+1)+1;  //First character after the end of the link.
				link = currentMessage.data.substr(pos1,pos2-pos1);
				link = "{a=" + link + "]" + link.substr(1,link.length()-2) + "[a}";  //Text is now: {a="link"]link[a}
				currentMessage.data.replace(pos1,pos2-pos1,link);
			}
			pos1 = currentMessage.data.find("http",pos1 + link.length());
		}
		//Select message formatting.
		switch(currentMessage.key) {
		case TAG:
			Output(log, format, "{li]{span=tags]Bash Tag suggestion(s):[span} " + currentMessage.data + "[li}");
			break;
		case SAY:
			Output(log, format, "{li]Note: " + currentMessage.data + "[li}");
			break;
		case REQ:
			Output(log, format, "{li]Requires: " + currentMessage.data + "[li}");
			break;
		case INC:
			Output(log, format, "{li]Incompatible with: " + currentMessage.data + "[li}");
			break;
		case WARN:
			Output(log, format, "{li=warn]Warning: " + currentMessage.data + "[li}");
			break;
		case ERR:
			Output(log, format, "{li=error]ERROR: " + currentMessage.data + "[li}");
			break;
		default:
			Output(log, format, "{li]Note: " + currentMessage.data + "[li}");
			break;
		}
	}

	//Prints header if format is HTML, else nothing.
	void OutputHeader(ofstream &log, string format) {
		if (format == "HTML") {
			log << "<!DOCTYPE html>"<<endl<<"<html>"<<endl<<"<head>"<<endl<<"<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>"<<endl
				<< "<title>BOSS Log</title>"<<endl<<"<style type='text/css'>"<<endl
				<< "body {font-family:Calibri,Arial,Verdana,sans-serifs;}"<<endl
				<< "body > div:first-child {font-size:2.4em; font-weight:bold; text-align: center; margin-bottom:20px;}"<<endl
				<< "body > div:first-child + div {text-align:center;}" <<endl
				<< "div > span:first-child {font-weight:bold; font-size:1.3em;}"<<endl
				<< "ul {margin-top:0px; list-style:none; margin-bottom:1.1em;}"<<endl
				<< "ul li {margin-left:-1em; margin-bottom:0.4em;}"<<endl
				<< ".error {color:red;}"<<endl
				<< ".success {color:green;}"<<endl
				<< ".warn {color:#FF6600;}"<<endl
				<< ".version {color:teal;}"<<endl
				<< ".ghosted {font-style:italic; color:grey;}"<<endl
				<< ".tags {color:maroon;}"<<endl
				<< "</style>"<<endl<<"</head>"<<endl<<"<body>"<<endl;
		}
	}

	//Converts an integer to a string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
	string IntToString(int n) {
		string out;
		back_insert_iterator<string> sink(out);
		karma::generate(sink,karma::int_,n);
		return out;
	}

	//Prints ouptut with formatting according to output format.
	void Output(ofstream &log, string format, string text) {
		if (format == "HTML") {
			//Yes. This really is as horrific as it looks. It's only temporary though. I hope...
			replace_first(text, "{end}", "</body>\n</html>");
			replace_first(text, "{c}", "&copy;");
			replace_first(text, "{&}", "&amp;");
			replace_first(text, "{ul]", "<ul>");
			replace_first(text, "[ul}", "</ul>\n");
			replace_first(text, "{b]", "<b>");
			replace_first(text, "[b}", "</b>");
			replace_first(text, "{i]", "<i>");
			replace_first(text, "[i}", "</i>");

			replace_first(text, "=warn]", " class='warn'>");
			replace_first(text, "=error]", " class='error'>");
			replace_first(text, "=success]", " class='success'>");
			replace_first(text, "=tags]", " class='tags'>");
			replace_first(text, "=ghosted]", " class='ghosted'>");
			replace_first(text, "=version]", " class='version'>");
			replace_first(text, "=title]", " class='title'>");
			replace_first(text, "=center]", " class='center'>");

			replace_all(text, "{br}", "<br />\n");

			replace_first(text, "{li]", "<li>");
			replace_first(text, "{li", "<li");
			replace_first(text, "[li}", "</li>\n");
			
			replace_all(text, "{span]", "<span>");
			replace_all(text, "{span", "<span");
			replace_all(text, "[span}", "</span>");
			replace_all(text, "{div]", "<div>");
			replace_all(text, "{div", "<div");
			replace_all(text, "[div}", "</div>\n");
			replace_all(text, "{p]", "<p>");
			replace_all(text, "{p", "<p");
			replace_all(text, "[p}", "</p>\n\n");	

			//Convert from generic format into HTML hyperlinks.
			size_t pos1,pos2;
			string link;
			pos1 = text.find("{a="); //Start of a link
			while (pos1 != string::npos) {
				pos2 = text.find("]",pos1);
				text.replace(pos2,1,">");
				text.replace(pos1,3,"<a href=");
				pos1 = text.find("[a}");
				text.replace(pos1,3,"</a>");
				pos1 = text.find("http",pos1 + 4);
			}
		}
		/*bosslog_html_grammar<back_insert_iterator<string>> grammar;
		string generated;
		back_insert_iterator<string> sink(generated);
		karma::generate(sink,grammar,text);
		log << generated;
		*/log << text;
	}
}