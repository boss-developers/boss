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

	

	namespace { // Using an anonymous so local private declarations are only usable from inside this file.
		using namespace std;
		using boost::algorithm::to_lower_copy;
		using boost::algorithm::to_lower;
		using boost::algorithm::to_upper_copy;
		using boost::algorithm::trim_copy;
		using boost::format;

		// Error messages for rule validation
		static boost::format ESortLineInForRule("It includes a sort line in a rule with a FOR rule keyword.");
		static boost::format ERuleHasUndefinedObject("The line with keyword '%1%' has an undefined object.");
		static boost::format EPluginNotInstalled("'%1%' is not installed.");
		static boost::format EAddingModGroup("It tries to add a group.");
		static boost::format ESortingGroupEsms("It tries to sort the group \"ESMs\".");
		static boost::format ESortingMasterEsm("It tries to sort the master .ESM file.");
		static boost::format EReferencingModAndGroup("It references a mod and a group.");
		static boost::format ESortingGroupBeforeEsms("It tries to sort a group before the group \"ESMs\".");
		static boost::format ESortingModBeforeGameMaster("It tries to sort a mod before the master .ESM file.");
		static boost::format EInsertingToTopOfEsms("It tries to insert a mod into the top of the group \"ESMs\", before the master .ESM file.");
		static boost::format EInsertingGroupToGroupOrModToMod("It tries to insert a group or insert a mod into another mod.");
		static boost::format EAttachingMessageToGroup("It tries to attach a message to a group.");
		static boost::format EUnrecognisedKeyword("The line \"%1%: %2%\" does not contain a recognised keyword. If this line is the start of a rule, that rule will also be skipped.");
		static boost::format EAppearsBeforeFirstRule("The line \"%1%: %2%\" appears before the first recognised rule line. Line skipped.");
		static boost::format EUnrecognizedKeywordBeforeFirstRule("The line \"%1%: %2%\" does not contain a recognised keyword, and appears before the first recognised rule line. Line skipped.");

		static boost::format MessageParagraphFormat(
			"<p style='margin-left:40px; text-indent:-40px;'\n"
			 "	The rule beginning \" %1%: %2%\" has been skipped as it has the following problem(s):\n"
			 "	<br/>\n"
			 "	%3%\n"
			 "	<br/>\n"
			 "</p>\n"
			);

		static boost::format MessageSpanFormat(
			"	<span class='%1%'>\n"
			 "		%2%\n"
			 "	</span>\n"
			);

		// Used to throw as exception when signaling a rule parsing error, in order to make the code a bit more compact.
		struct failure
		{
			failure(bool skipped, string const& rule, string const& object, boost::format const& message) 
				: skipped(skipped), object(object), rule(rule), message(message)
			{}

			string object;
			string rule;
			format message;
			bool skipped;
		};
	}

	namespace boss {

	//Add rules from userlist.txt into the rules object.
	//Then checks rule syntax and discards rules with incorrect structures.
	//Also checks if the mods referenced by rules are in your Data folder, and discards rule that reference missing mods.
	//Generates error messages for rules that are discarded.
	void Rules::AddRules() {
		ifstream userlist;
		string line,key,object;
		size_t pos;
		bool skip = false;
		messages += "<p>";
		userlist.open(userlist_path.c_str());
		while (!userlist.eof()) {
			char cbuffer[MAXLENGTH];
			userlist.getline(cbuffer,MAXLENGTH);
			line=cbuffer;

			if (line.length() == 0) continue;			

			if (line.substr(0,2) == "//") continue;

			pos = line.find(":");
			if (pos == string::npos) continue;

			key = to_upper_copy(trim_copy(line.substr(0,pos)));
			object = trim_copy(line.substr(pos+1));

			try {
				if (key=="add" || key=="override" || key=="for") {

					if (skip) {
						keys.erase(keys.begin() + rules.back(), keys.end());
						objects.erase(objects.begin() + rules.back(), objects.end());
						rules.pop_back();
					}

					keys.push_back(key);
					objects.push_back(object);
					rules.push_back((int)keys.size() - 1);
					skip = false;

					const string rule = keys[rules.back()];
					const string subject = objects[rules.back()];

					if (object.empty()) {
						throw failure(skip, rule, subject, ERuleHasUndefinedObject % key);
					}
				
					if (IsPlugin(object) && !fs::exists(data_path / fs::path(object))) {
						throw failure(skip, rule, subject, EPluginNotInstalled % object);
					} 
				
					if (key=="add" && !IsPlugin(object)) {
						throw failure(skip, rule, subject, EAddingModGroup);
					} 
					
					if (key=="override") {
						if (Tidy(object)=="esms") {
							throw failure(skip, rule, subject, ESortingGroupEsms);
						}
						
						if (Tidy(object)=="oblivion.esm" || Tidy(object)=="fallout3.esm" || Tidy(object)=="nehrim.esm" || Tidy(object)=="falloutnv.esm") {
							throw failure(skip, rule, subject, ESortingMasterEsm);
						}
					}
				

				} else if (!rules.empty()) {

					if ((key=="before" || key=="after")) {
						keys.push_back(key);
						objects.push_back(object);

						const string rule = keys[rules.back()];
						const string subject = objects[rules.back()];

						if (rule=="for") {
							throw failure(skip, rule, subject, ESortLineInForRule);
						}

						if (object.empty()) {
							throw failure(skip, rule, subject, ERuleHasUndefinedObject % key);
						} 

						if (subject.length() > 0 && ((IsPlugin(object) && !IsPlugin(subject)) || (!IsPlugin(object) && IsPlugin(subject)))) {
							throw failure(skip, rule, subject, EReferencingModAndGroup);
						}

						if (key=="before") {
							if (Tidy(object)=="esms") {
								throw failure(skip, rule, subject, ESortingGroupBeforeEsms);
							} 
							
							if (Tidy(object)=="oblivion.esm" || Tidy(object)=="fallout3.esm" || Tidy(object)=="nehrim.esm" || Tidy(object)=="falloutnv.esm") {
								throw failure(skip, rule, subject, ESortingModBeforeGameMaster);
							}
						}

					} else if ((key=="top" || key=="bottom")) {
						keys.push_back(key);
						objects.push_back(object);

						const string rule = keys[rules.back()];
						const string subject = objects[rules.back()];

						if (rule=="for") {
							throw failure(skip, rule, subject, ESortLineInForRule);
						}

						if (object.empty()) {
							throw failure(skip, rule, subject, ERuleHasUndefinedObject % key);
						} 
						
						if (Tidy(object)=="esms" && key=="top") {
							throw failure(skip, rule, subject, EInsertingToTopOfEsms);
						}
						
						if (subject.length() > 0 && !IsPlugin(subject) || IsPlugin(object)) {
							throw failure(skip, rule, subject, EInsertingGroupToGroupOrModToMod);
						}

					} else if ((key=="append" || key=="replace")) {					
						keys.push_back(key);
						objects.push_back(object);

						const string rule = keys[rules.back()];
						const string subject = objects[rules.back()];

						if (object.empty()) {
							throw failure(skip, rule, subject, ERuleHasUndefinedObject % key);
						}

						if (!IsPlugin(subject)) {
							throw failure(skip, rule, subject, EAttachingMessageToGroup);
						}

					} else {
						const string rule = keys[rules.back()];
						const string subject = objects[rules.back()];

						//Line does not contain a recognised keyword. Skip it and the rule containing it. If it is a rule line, then the previous rule will also be skipped.
						throw failure(skip, rule, subject, EUnrecognisedKeyword % key % object);
					}

				} else {
					//Line is not a rule line, and appears before the first rule line, so does not belong to a rule. Skip it.
					if (key=="before" || key=="after" || key=="top" || key=="bottom" || key=="append" || key=="replace") 
						AddError(EAppearsBeforeFirstRule % key % object);
					else
						AddError(EUnrecognizedKeywordBeforeFirstRule % key % object);
				}

			} catch(failure const& e) {
				skip = true;
				AddError(e.skipped, e.rule, e.object, e.message);
			}
		}

		userlist.close();
		messages += "</p>";
		if (skip) {
			keys.erase(keys.begin()+rules.back(), keys.end());
			objects.erase(objects.begin()+rules.back(), objects.end());
			rules.pop_back();
		}
	}

	const string Rules::FormatMesssage(string const& class_, string const& rule, string const& object, format const& message)
	{
		string const span = FormatMesssage(class_ , message);
		return (MessageParagraphFormat % rule % object % span).str();
	}

	const string Rules::FormatMesssage(string const& class_, format const& message)
	{
		return (MessageSpanFormat % class_ % message.str()).str();
	}

	void Rules::AddMessage(bool skipped, string const& rule, string const& object, format const& message, string const& class_)
	{
		if (!skipped) {
			messages += FormatMesssage(class_, rule, object, message) ;
		} else {
			messages += FormatMesssage(class_, message) ;
		}
	}

	void Rules::AddError(bool skipped, string const& rule, string const& object, format const& message)
	{
		AddMessage(skipped, rule, object, message, "error");
	}

	void Rules::AddError(format const& message)
	{
		messages += FormatMesssage("error", message);
	}

	void Rules::AddRule(parsing::Rule const& rule)
	{
		/*
		A rule has been parsed in full:

			Here goes: 
				- the validation code found above in AddRules()
				- Add the Rule definition to our local arrays for later use.			
		*/
	};

	// Called when an error is detected while parsing the input file.
	void Rules::SyntaxError(
			string::const_iterator const& begin, 
			string::const_iterator const& end, 
			string::const_iterator const& error_pos, 
			string const& what) 
	{
		std::string context(error_pos, std::min(error_pos + 50, end));
		boost::trim_left(context);
		boost::replace_all(context, "\n", "<EOL>");

		std::cerr << "Syntax error while trying to parse Userlist.txt: '" << what << "' near this input: '" << context << "'." << std::endl;
	};

	void Rules::ParsingFailed(
			string::const_iterator	const& begin, 
			string::const_iterator	const& end, 
			string::const_iterator	const& error_pos, 
			string::difference_type lineNo)
	{
		string context(error_pos, std::min(error_pos + 50, end));
		boost::trim_left(context);
		boost::replace_all(context, "\n", "<EOL>");

		std::cerr << "Userlist.txt parsing error at line#: " << lineNo << " while reading near this input: '" << context << "'." << std::endl;
	};
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