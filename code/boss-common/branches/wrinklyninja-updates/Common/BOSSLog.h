/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains the various functions/classes required for varied BOSSlog output formattings,etc.
//Still at the brainstorming stage.
//The idea is to separate the unformatted text from the formatting. Unformatted text is generated in main() and passed through something found
//here to be formatted appropriately.

#ifndef __BOSS_BOSSLOG_H__
#define __BOSS_BOSSLOG_H__

#include <fstream>
#include <string>
#include "Lists.h"

#include "Globals.h"
#include "Support/Helpers.h"
#include "Parsing.h"
#include <string>
#include <fstream>
#include <vector>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace boss {
	using namespace std;

	enum formatType {
		HTML,
		PLAINTEXT
	};

	//Prints a given message to the bosslog. Not format-safe ATM.
	void ShowMessage(message message, ofstream &log);

	//Prints ouptut with formatting according to format.
	void Output(ofstream &log, formatType format, string text);

	//Prints header if format is HTML, else nothing.
	void OutputHeader(ofstream &log, formatType format);

	//Userlist parser dump for now.
	//Class to store userlist rules.
	class Rules : public parsing::IRulesManager  {
	protected:
		virtual void AddRule(parsing::Rule const& rule);

		virtual void SyntaxError(
				string::const_iterator const& begin, 
				string::const_iterator const& end, 
				string::const_iterator const& error_pos, 
				string const& what);

		virtual void ParsingFailed(
				string::const_iterator const& begin, 
				string::const_iterator const& end, 
				string::const_iterator const& error_pos, 
				string::difference_type lineNo);

		private:
			static const string FormatMesssage(string const& class_, string const& rule, string const& object, boost::format const& message);
			static const string FormatMesssage(string const& class_, boost::format const& message);

			void AddMessage(bool skipped, string const& rule, string const& object, boost::format const& message, string const& class_);
			void AddError(bool skipped, string const& rule, string const& object, boost::format const& message);
			void AddError(boost::format const& message);
		public:
			vector<string> keys,objects;			//Holds keys and objects for each rule line.
			vector<int> rules;						//Tells BOSS where each rule starts.
			string messages;						//Stores output messages.
			void AddRules();						//Populates object vectors with rules from userlist.txt.
			void PrintMessages(ofstream& output);	//Prints the output messages.
		};
}

/*

namespace boss {
	using namespace std;

	class BOSSLog {
	private:
		int logFormat;
		ofstream bosslog;
	public:
		enum logType {
			HTML = 0,
			PLAINTEXT = 1
		};
		enum textType {
			NO_TYPE = 0,
			TITLE = 1,
			LINK = 2,
			ERR = 3,
			SUCCESS = 4,
			WARN = 5,
			GHOST = 6,
			VER = 7,
			TAG = 8,
			SUBTITLE = 9,
			LI = 10
		};
		enum divType {
			DIV = 0,
			PARA = 1
		};
		void setLogType(logType type);
		void startLog();  //Print header for HTML, nothing for plaintext.
		void writeText(string text, textType type);
		void writeLink(string link, string text);
		void endl(int number);
		void startDiv(divType type);
		void endDiv(divType type);
		void endLog(); //Print closing tags for HTML, nothing for plaintext.
		bool open(boost::filesystem::path file);
	}

	//Now a dumping ground for un-sorted testing code.
	//Try implementing a BOSSLogger function or something.

	enum styleType {
		NO_STYLE = 0,
		TITLE = 1,
		LINK = 2,
		ERR = 3,
		SUCCESS = 4,
		WARN = 5,
		GHOST = 6,
		VER = 7,
		TAG = 8,
		SUBTITLE = 9,
		LI = 10
	};

	void printHTMLHead(ofstream& log);

	void printBOSSLogText(ofstream& log, string text, logFormat format, attr attribute, styleType style);
	void printBOSSLogText(ofstream& log, string text, string link, logFormat format, attr attribute, styleType style);
}*/
#endif