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

	void ShowMessage(message message, ofstream &log) {
		size_t pos1,pos2,pos3;
		string link;
		//Replace web addresses with hyperlinks.
		pos1 = message.data.find("http");
		while (pos1 != string::npos) {
			pos2 = message.data.find(" ",pos1);
			link = message.data.substr(pos1,pos2-pos1);
			link = "<a href='"+link+"'>"+link+"</a>";
			message.data.replace(pos1,pos2-pos1,link);
			pos1 = message.data.find("http",pos1 + link.length());
		}
		//Select message formatting.
		if (message.key=="TAG") {
			//Insert spaces in tag list to allow wrapping.
			pos1 = message.data.find("{{BASH:");
			if (pos1 != string::npos) {
				pos2 = message.data.find("}}",pos1);
				pos3 = message.data.find(",",pos1);
				while (pos3 != string::npos && pos3 < pos2) {
					message.data.replace(pos3,1,", ");
					pos3 = message.data.find(",",pos3+9);
				}
			}
			log << "<li><span class='tags'>Bash Tag suggestion(s):</span> " << message.data << "</li>" << endl;
		} else if (message.key=="SAY") {
			log << "<li>Note: " << message.data << "</li>" << endl;
		} else if (message.key=="REQ") {
			log << "<li>Requires: " << message.data << "</li>" << endl;
		} else if (message.key=="WARN") {
			log << "<li class='warn'>Warning: " << message.data << "</li>" << endl;
		} else if (message.key=="ERROR") {
			log << "<li class='error'>!!! CRITICAL INSTALLATION ERROR: " << message.data << "</li>" << endl;
		}
	}

	//Prints ouptut with formatting according to format.
	void Output(ofstream &log, formatType format, string text) {
		size_t pos = 0;
		if (format == HTML) {
			log << text;
			if (text.find("</div>") != string::npos)
				log << endl;
			else if (text.find("</li>") != string::npos)
				log << endl;
			else if (text.find("</p>") != string::npos)
				log << endl;
			else if (text.find("</body>") != string::npos)
				log << endl;
			if (text.find("<br />") != string::npos)
				log << endl;
		} else {
		}
	}

	//Prints header if format is HTML, else nothing.
	void OutputHeader(ofstream &log, formatType format) {
		if (format == HTML) {
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
	}
}

/*
namespace boss {
	using namespace std;


		void BOSSLog::setLogType(logType type) {
			logFormat = type;
		}

		void BOSSLog::writeText(std::string text, textType type) {
			if (logFormat == HTML) {
				switch (type) {
					case TITLE:
						bosslog << "<div id='title'>" << text << "</div>" << endl;
						break;
					case ERR:
						bosslog << "<span class='error'>" << text << "</span>";
						break;
					case SUCCESS:
						bosslog << "<span class='success'>" << text << "</span>";
						break;
					case WARN:
						bosslog << "<span class='warn'>" << text << "</span>";
						break;
					case GHOST:
						bosslog << "<span class='ghosted'>" << text << "</span>";
						break;
					case VER:
						bosslog << "<span class='version'>" << text << "</span>";
						break;
					case TAG:
						bosslog << "<span class='tags'>" << text << "</span>";
						break;
					case SUBTITLE:
						bosslog << "<span>" << text << "</span>";
					case LI:
						bosslog << "<li>" << text << "</li>";
					default:
						bosslog << text;
						break;
					}
				}
			} else {

			}
		}

		void BOSSLog::writeLink(std::string text, textType type) {
			if (logFormat == HTML) {
				bosslog << "<a href='" << link << "'>" << text << "</a>";
			} else {
				bosslog << text << " (" << link << ") ";
			}
		}

		void BOSSLog::endl(int number) {
			if (logFormat == HTML)
				bosslog << "<br />" << endl;
			else
				bosslog << endl;
		}


		void BOSSLog::startDiv(divType type) {
			if (logFormat == HTML) {

			} else {

			}
		}


		void BOSSLog::endDiv(divType type) {
			if (logFormat == HTML) {

			} else {

			}
		}

		//Print closing tags for HTML, nothing for plaintext.
		void BOSSLog::endLog() {
			if (logFormat == HTML) {

			}
		}

		bool BOSSLog::open(boost::filesystem::path file) {
			bosslog.open(file.c_str());
			if (bosslog.fail())
				return false;
			else
				return true;
		}
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
				log << "===========================================================" << endl << endl
					<< text << endl
					<< "===========================================================" << endl << endl;
				break;
			case LINK:
				log << text << " (" << link << ") " << endl;
				break;
			case SUBTITLE:
				log << "-----------------------------------------------------------" << endl
					<< text << endl
					<< "-----------------------------------------------------------" << endl << endl;
			}
		}
	}
}*/