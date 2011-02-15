/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "BOSSLog.h"


namespace boss {
	using namespace std;

//What it says on the tin.
	void printHTMLHead(ofstream& log) {
		log << "<!DOCTYPE html>"<<endl<<"<html>"<<endl<<"<head>"<<endl<<"<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>"<<endl
			<< "<title>BOSS Log</title>"<<endl<<"<style type='text/css'>"<<endl
			<< "body {font-family:Calibri,Arial,Verdana,sans-serifs;}"<<endl
			<< "#title {font-size:2.4em; font-weight:bold; text-align: center;}"<<endl
			<< "#title + div {text-align: center;}"<<endl
			<< "div > span:first-child {font-weight:bold; font-size:1.3em;}"<<endl
			<< "ul {margin-top:0px; list-style:none; margin-bottom:1.1em;}"<<endl
			<< "ul li {margin-left:-1em; margin-bottom:0.4em;}"<<endl
			<< ".error {color:red;}"<<endl
			<< ".success {color:green}"<<endl
			<< ".warn {color:#FF6600;}"<<endl
			<< ".version {color:teal;}"<<endl
			<< ".ghosted {font-style:italic; color:grey;}"<<endl
			<< ".tags {color:maroon;}"<<endl
			<< "</style>"<<endl<<"</head>"<<endl<<"<body>"<<endl;
	}

	void printLogText(ofstream& log, string text, logFormat format, attr attribute, styleType style) {
		printLogText(log,text,"",format,attribute, style);
	}

	void printLogText(ofstream& log, string text, string link, logFormat format, attr attribute, styleType style) {
		if (format == 0) {
			//Apply early attributes.
			switch (attribute) {
			case START_DIV:
				log << "<div>" << endl;
				break;
			case START_PARA:
				log << "<p>" << endl;
				break;
			}
			//Apply text CSS style.
			switch (style) {
			case TITLE:
				log << "<div id='title'>" << text << "</div>" << endl;
				break;
			case LINK:
				log << "<a href='" << link << "'>" << text << "</a>";
				break;
			case ERR:
				log << "<span class='error'>" << text << "</span>";
				break;
			case SUCCESS:
				log << "<span class='success'>" << text << "</span>";
				break;
			case WARN:
				log << "<span class='warn'>" << text << "</span>";
				break;
			case GHOST:
				log << "<span class='ghosted'>" << text << "</span>";
				break;
			case VER:
				log << "<span class='version'>" << text << "</span>";
				break;
			case TAG:
				log << "<span class='tags'>" << text << "</span>";
				break;
			case SUBTITLE:
				log << "<span>" << text << "</span>";
			case LI:
				log << "<li>" << text << "</li>";
			default:
				log << text;
				break;
			}
			//Apply closing formatting.
			switch (attribute) {
			case BR:
				log << "<br />" << endl;
				break;
			case END_DIV:
				log << "</div>" << endl;
				break;
			case END_PARA:
				log << "</p>" << endl;
				break;
			case END_LOG:
				log << "</body>" << endl << "</html>" << endl;
				break;
			}
		} else if (format == 1) {
			size_t pos;
			if ((pos = text.find("&copy;")) != string::npos)
				text.replace(pos,pos+6,"(c)");
			if ((pos = text.find("&amp;")) != string::npos)
				text.replace(pos,pos+5,"&");
			switch (attribute) {
			case TITLE:
				log << "-----------------------------------------------------------" << endl
					<< text << endl
					<< "-----------------------------------------------------------" << endl;
				break;
			case LINK:
				log << text << " (" << link << ") " << endl;
				break;
			}
		}
	}
}