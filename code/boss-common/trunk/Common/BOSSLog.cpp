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
			Output(log, format, "<li><span class='tags'>Bash Tag suggestion(s):</span> " + currentMessage.data + "</li>\n");
			break;
		case SAY:
			Output(log, format, "<li>Note: " + currentMessage.data + "</li>\n");
			break;
		case REQ:
			Output(log, format, "<li>Requires: " + currentMessage.data + "</li>\n");
			break;
		case INC:
			Output(log, format, "<li>Incompatible with: " + currentMessage.data + "</li>\n");
			break;
		case WARN:
			Output(log, format, "<li class='warn'>Warning: " + currentMessage.data + "</li>\n");
			break;
		case ERR:
			Output(log, format, "<li class='error'>ERROR: " + currentMessage.data + "</li>\n");
			break;
		case DIRTY:
			Output(log, format, "<li class='dirty'>Dirty mod: " + currentMessage.data + "</li>\n");
			break;
		default:
			Output(log, format, "<li>Note: " + currentMessage.data + "</li>\n");
			break;
		}
	}

	//Prints header if format is HTML, else nothing.
	void OutputHeader(ofstream &log, string format) {
		if (format == "html") {
			log << "<!DOCTYPE html>"<<endl<<"<html>"<<endl<<"<head>"<<endl<<"<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>"<<endl
				<< "<title>BOSS Log</title>"<<endl<<"<style type='text/css'>"<<endl
				<< "body {font-family:Calibri,Arial,Verdana,sans-serifs;}"<<endl
				<< "body > div:first-child {font-size:2.4em; font-weight:bold; text-align: center; margin-bottom:20px;}"<<endl
				<< "body > div:first-child + div {text-align:center;}" <<endl
				<< "div > span:first-child {font-weight:bold; font-size:1.3em;}"<<endl
				<< "ul {margin-top:0px; list-style:none; margin-bottom:1.1em;}"<<endl
				<< "ul li {margin-left:-1em; margin-bottom:0.4em;}"<<endl
				<< "blockquote {font-style:italic;}"<<endl
				<< ".error {color:red;}"<<endl
				<< ".success {color:green;}"<<endl
				<< ".warn {color:#FF6600;}"<<endl
				<< ".version {color:teal;}"<<endl
				<< ".ghosted {font-style:italic; color:grey;}"<<endl
				<< ".tags {color:maroon;}"<<endl
				<< ".dirty {color:#996600;}"<<endl
				<< "</style>"<<endl<<"</head>"<<endl<<"<body>"<<endl;
		}
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
			
			replace_first(text, "<ul>", "");
			replace_first(text, "</ul>", "");
			replace_first(text, "<b>", "");
			replace_first(text, "</b>", "");
			replace_first(text, "<i>", "");
			replace_first(text, "</i>", "");

			replace_first(text, "<span>", "======================================\n");

			replace_first(text, " class='warn'>", ">");
			replace_all(text, " class='error'>", ">");
			replace_first(text, " class='success'>", ">");
			replace_first(text, " class='tags'>", ">");
			replace_first(text, " class='ghosted'>", ">");
			replace_first(text, " class='version'>", ">");
			replace_first(text, " style='color: grey;'>", ">");
			replace_first(text, " class='dirty'>", ">");

			replace_first(text, "<blockquote>", "\n\n");
			replace_first(text, "</blockquote>", "\n\n");
			replace_first(text, "<li>", "*  ");
			replace_first(text, "</li>", "");
			
			replace_first(text, "</div>", "");
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