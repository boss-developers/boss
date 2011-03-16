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

Bosslog output should be done via an interface that provides a conversion between generic formatting symbols
and the required specific formatting for the given output type. It must also be streamed to the log, not
held in a buffer, because in the event of a crash, the logged information is useful.

It should be concise and flexible, with no limit to the number of formatting symbols.

Using a generic symbol format means we can use BOOST's Spirit.Karma as an output generator, which is quite neat as we use Qi for input.
It's almost certainly simpler than doing replace_all functions for every possible permutation of symbols.

The generic symbols could be given in the form: $<type>$
Symbols for formatting which wraps text could be given as $<type>$ ... $-<type>$
Some wrapping formatting needs specific type identifiers (ie. classes, links). Perhaps: 
$<type>|class=<class>$
$link=<link>$

Elements:

|| HTML Syntax 			|| Plaintext Syntax || General Syntax 	||
//////////////////////////////////////////////////////////////////
|| <br />	   			|| \n			   	|| $br$			  	||
|| <div>...</div>		|| ...\n			|| $block$			||
|| <p>..</p>			|| ...\n\n			|| $para$			||
|| <span>...</span>	 	|| ...\n===			|| $span$			|| //A span with no class. Plaintext is underlined. Remove underline if there is a class.
|| <ul>...</ul>			|| \n...\n			|| $ul$				||
|| <li>...</li>			|| * ...\n			|| $li$				||
|| <a href=".">..</a>	|| .. (.)			|| $link=<link>$	||
|| <b>...</b>			|| ...				|| $b$				||
|| <i>...</i>			|| ...				|| $i$				||
|| </body></html>		|| None				|| $end$			||

Only the opening general syntax is shown, all but $br$ and $end$ also have a closing counterpart.

Classes:

|| Class	|| Plaintext Effect 		|| HTML Effect 												||
//////////////////////////////////////////////////////////////////////////////////////////////////////
|| title	|| ===\n\n text \n\n ===	|| font-size:2.4em; font-weight:bold; text-align: center;	||
|| center	|| None						|| text-align:center;										||
|| error	|| Uppercase text			|| color:red;												||
|| success	|| None						|| color:green;												||
|| warn		|| None						|| color:#FF6600;											||
|| version	|| None						|| color:teal;												||
|| ghosted	|| None						|| font-style:italic; color:grey;							||
|| tags		|| None						|| color:maroon;											||
*/

namespace boss {
	using namespace std;
	using boost::algorithm::replace_all;

	void ShowMessage(ofstream &log, formatType format, message currentMessage) {
		size_t pos1,pos2;
		string link;
		//Wrap web addresses in generic link format.
		pos1 = currentMessage.data.find("http");  //Start of link.
		while (pos1 != string::npos) {
			pos2 = currentMessage.data.find(" ",pos1);  //End of link.
			link = currentMessage.data.substr(pos1,pos2-pos1);
			link = "$link=" + link + "$" + link + "$-link$";
			currentMessage.data.replace(pos1,pos2-pos1,link);
			pos1 = currentMessage.data.find("http",pos1 + link.length());
		}
		//Select message formatting.
		switch(currentMessage.key) {
		case TAG:
			Output(log, format, "$li$ $span|class=tags$ Bash Tag suggestion(s):$-span$ " + currentMessage.data + "$-li$");
			break;
		case SAY:
			Output(log, format, "$li$ Note: " + currentMessage.data + "$-li$");
			break;
		case REQ:
			Output(log, format, "$li$ Requires: " + currentMessage.data + "$-li$");
			break;
		case INC:
			Output(log, format, "$li$ Incompatible with: " + currentMessage.data + "$-li$");
			break;
		case WARN:
			Output(log, format, "$li|class=warn$ Warning: " + currentMessage.data + "$-li$");
			break;
		case ERR:
			Output(log, format, "$li|class=error$ ERROR: " + currentMessage.data + "$-li$");
			break;
		}
	}

	//Prints header if format is HTML, else nothing.
	void OutputHeader(ofstream &log, formatType format) {
		if (format == HTML) {
			log << "<!DOCTYPE html>"<<endl<<"<html>"<<endl<<"<head>"<<endl<<"<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>"<<endl
				<< "<title>BOSS Log</title>"<<endl<<"<style type='text/css'>"<<endl
				<< "body {font-family:Calibri,Arial,Verdana,sans-serifs;}"<<endl
				<< ".title {font-size:2.4em; font-weight:bold; text-align: center;}"<<endl
				<< "div > span:first-child {font-weight:bold; font-size:1.3em;}"<<endl
				<< "ul {margin-top:0px; list-style:none; margin-bottom:1.1em;}"<<endl
				<< "ul li {margin-left:-1em; margin-bottom:0.4em;}"<<endl
				<< ".center {text-align:center;}" <<endl
				<< ".error {color:red;}"<<endl
				<< ".success {color:green;}"<<endl
				<< ".warn {color:#FF6600;}"<<endl
				<< ".version {color:teal;}"<<endl
				<< ".ghosted {font-style:italic; color:grey;}"<<endl
				<< ".tags {color:maroon;}"<<endl
				<< "</style>"<<endl<<"</head>"<<endl<<"<body>"<<endl;
		}
	}

	//Prints ouptut with formatting according to format.
	void Output(ofstream &log, formatType format, string text) {
		if (format == HTML) {
			replace_all(text, "$li$", "<li>");
			replace_all(text, "$li|class=warn$", "<li class='warn'>");
			replace_all(text, "$li|class=error$", "<li class='error'>");
			replace_all(text, "$-li$", "</li>");
			replace_all(text, "$span$", "</span>");
			replace_all(text, "$span|class=tags$", "<span class='tags'>");
			replace_all(text, "$-span$", "</span>");
		} else {

		}
		log << text;
	}
}