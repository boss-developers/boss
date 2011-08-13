/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Header file for ini grammar definition.

#ifndef __BOSS_INI_GRAM_H__
#define __BOSS_INI_GRAM_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include "Parsing/Data.h"
#include "Parsing/Skipper.h"
#include "Common/Globals.h"
#include "Support/Helpers.h"
#include "Support/Logger.h"
#include "Common/BOSSLog.h"

#include <sstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

namespace boss {
	namespace unicode = boost::spirit::unicode;
	namespace phoenix = boost::phoenix;
	namespace qi = boost::spirit::qi;

	using namespace std;
	using namespace qi::labels;

	using qi::eol;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;

	using unicode::char_;
	using unicode::space;

	using boost::spirit::info;
	using boost::format;

	////////////////////////////
	//Ini Grammar.
	////////////////////////////

	string heading;  //The current ini section heading.

	static format IniParsingErrorFormat("<li><span class='error'>Ini Parsing Error: Expected a %1% at:</span>"
		"<blockquote>%2%</blockquote>"
		"<span class='error'>Ini parsing aborted. Some or all of the options may not have been set correctly.</span></li>\n");

	void StoreHeading(string hdng) {
		heading = hdng;
	}

	void SetVar(string var, string value) {
		int intVal;
		bool bval;
		if (heading == "GUI.LastOptions" || heading == "GUI.Settings")
			return;
		if (heading == "BOSS.InternetSettings") {
			if (var == "ProxyType")
				proxy_type = value;
			else if (var == "ProxyHostname")
				proxy_host = value;
			else if (var == "ProxyPort")
				proxy_port = value;
			return;
		} else if (heading == "BOSS.RunOptions") {
			if (var == "Game") {
				if (value == "Oblivion")
					game = 1;
				else if (value == "Nehrim")
					game = 3;
				else if (value == "Fallout3")
					game = 2;
				else if (value == "FalloutNV")
					game = 4;
				return;
			} else if (var == "BOSSlogFormat") {
				if (value == "html" || value == "text")
					log_format = value;
				return;
			} else if (var == "ProxyType")
				proxy_type = value;
			else if (var == "ProxyHostname")
				proxy_host = value;
			else if (var == "ProxyPort")
				proxy_port = value;

			intVal = atoi(value.c_str());
			if (intVal == 0)
				bval = false;
			else
				bval = true;

			if (var == "UpdateMasterlist")
				update = bval;
			else if (var == "OnlyUpdateMasterlist")
				update_only = bval;
			else if (var == "DisableMasterlistUpdate") {
				if (bval)
					update = false;
			} else if (var == "SilentRun")
				silent = bval;
			else if (var == "NoVersionParse")
				skip_version_parse = bval;
			else if (var == "Debug")
				debug = bval;
			else if (var == "DisplayCRCs")
				show_CRCs = bval;
			else if (var == "DoTrialRun")
				trial_run = bval;
			else if (var == "RevertLevel") {
				if (intVal >= 0 && intVal < 3)
					revert = intVal;
			} else if (var == "CommandLineVerbosity") {
				if (intVal >= 0 && intVal < 4)
					verbosity = intVal;
			}
			return;
		} else if (heading == "BOSSlog.Filters") {
			intVal = atoi(value.c_str());
			if (intVal == 0)
				bval = false;
			else
				bval = true;
			if (var == "UseDarkColourScheme") {
				UseDarkColourScheme = bval;
			} else if (var == "HideVersionNumbers") {
				HideVersionNumbers = bval;
			} else if (var == "HideGhostedLabel") {
				HideGhostedLabel = bval;
			} else if (var == "HideChecksums") {
				HideChecksums = bval;
			} else if (var == "HideMessagelessMods") {
				HideMessagelessMods = bval;
			} else if (var == "HideGhostedMods") {
				HideGhostedMods = bval;
			} else if (var == "HideCleanMods") {
				HideCleanMods = bval;
			} else if (var == "HideRuleWarnings") {
				HideRuleWarnings = bval;
			} else if (var == "HideAllModMessages") {
				HideAllModMessages = bval;
			} else if (var == "HideNotes") {
				HideNotes = bval;
			} else if (var == "HideBashTagSuggestions") {
				HideBashTagSuggestions = bval;
			} else if (var == "HideRequirements") {
				HideRequirements = bval;
			} else if (var == "HideIncompatibilities") {
				HideIncompatibilities = bval;
			}
			return;
		} else if (heading == "BOSSlog.Styles") {
			if (value == "")
				return;
			else if (var == "body")
				CSSBody = value;
			else if (var == ".filters")
				CSSFilters = value;
			else if (var == ".filters > li")
				CSSFiltersList = value;
			else if (var == "body > div:first-child")
				CSSTitle = value;
			else if (var == "body > div:first-child + div")
				CSSCopyright = value;
			else if (var == "body > div")
				CSSSections = value;
			else if (var == "body > div > span:first-child")
				CSSSectionTitle = value;
			else if (var == "body > div > span:first-child > span")
				CSSSectionPlusMinus = value;
			else if (var == "div > ul")
				CSSTopLevelList = value;
			else if (var == "body > div:last-child")
				CSSLastSection = value;
			else if (var == "body > div:last-child > span:first-child")
				CSSLastSectionTitle = value;
			else if (var == "div > ul > li")
				CSSTopLevelListItem = value;
			else if (var == "ul")
				CSSList = value;
			else if (var == "ul li")
				CSSListItem = value;
			else if (var == "li ul")
				CSSItemList = value;
			else if (var == "input[type='checkbox']")
				CSSCheckbox = value;
			else if (var == "blockquote")
				CSSBlockquote = value;
			else if (var == "#unrecognised > li")
				CSSUnrecognisedList = value;
			else if (var == "#summary > div")
				CSSSummaryRow = value;
			else if (var == "#summary > div > div")
				CSSSummaryCell = value;
			else if (var == ".error")
				CSSError = value;
			else if (var == ".warn")
				CSSWarning = value;
			else if (var == ".success")
				CSSSuccess = value;
			else if (var == ".version")
				CSSVersion = value;
			else if (var == ".ghosted")
				CSSGhost = value;
			else if (var == ".crc")
				CSSCRC = value;
			else if (var == ".tagPrefix")
				CSSTagPrefix = value;
			else if (var == ".dirty")
				CSSDirty = value;
			else if (var == ".message")
				CSSQuotedMessage = value;
			else if (var == ".mod")
				CSSMod = value;
			else if (var == ".tag")
				CSSTag = value;
			else if (var == ".note")
				CSSNote = value;
			else if (var == ".req")
				CSSRequirement = value;
			else if (var == ".inc")
				CSSIncompatibility = value;
		}
	}

	template <typename Iterator>
	struct ini_grammar : qi::grammar<Iterator, Ini_Skipper<Iterator>> {
		ini_grammar() : ini_grammar::base_type(ini, "ini grammar") {

			ini =
				section % +eol;

			section =
				heading[phoenix::bind(&StoreHeading, _1)]
				> +eol
				> setting % +eol;

			heading %= 
				lit("[")
				>> (+(char_ - "]"))
				>> lit("]");

			setting =
				(var
				> '='
				> value)[phoenix::bind(&SetVar, _1, _2)];

			var %=
				lexeme[(lit("\"") >> +(char_ - lit("\"")) >> lit("\""))]
				|
				(!lit("[") >> +(char_ - '='));

			value %=
				(lit("{") >> lexeme[*(char_ - lit("}"))] >> lit("}"))
				|
				+(char_ - eol);

			//Give each rule names.
			ini.name("ini");
			section.name("section");
			heading.name("heading");
			setting.name("setting");
			var.name("variable");
			value.name("value");
		
			//Error handling.
			on_error<fail>(ini,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(section,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(heading,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(setting,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(var,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(value,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
		}

		typedef Ini_Skipper<Iterator> skipper;

		qi::rule<Iterator, skipper> ini, section, setting;
		qi::rule<Iterator, string(), skipper> var, value, heading;
	
		void SyntaxError(Iterator const& /*first*/, Iterator const& last, Iterator const& errorpos, info const& what) {
			ostringstream out;
			out << what;
			string expect = out.str().substr(1,out.str().length()-2);
			if (expect == "eol")
				expect = "end of line";

			string context(errorpos, min(errorpos +50, last));
			boost::trim_left(context);

			LOG_ERROR("Ini Parsing Error: Expected a %s at \"%s\". Ini parsing aborted. No further settings will be applied.", expect.c_str(), context.c_str());
			
			expect = "&lt;" + expect + "&gt;";
			boost::replace_all(context, "\n", "<br />\n");
			string msg = (IniParsingErrorFormat % expect % context).str();
			iniErrorBuffer.push_back(msg);
			return;
		}
	};
}


#endif