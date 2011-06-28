/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/


	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __GUI_GRAMMARS_H__
#define __GUI_GRAMMARS_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include <sstream>
#include "helpers.h"

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

	using qi::skip;
	using qi::eol;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;
	using qi::lit;
	using qi::eoi;

	using unicode::char_;
	using unicode::no_case;
	using unicode::space;

	using boost::spirit::info;
	using boost::format;

	////////////////////////
	//Skipper Grammar
	////////////////////////

	template <typename Iterator>
	struct Ini_Skipper : qi::grammar<Iterator> {

		Ini_Skipper() : Ini_Skipper::base_type(start, "Ini Skipper") {

			start = 
				spc
				| UTF8
				| comment
				| eof;
			
			spc = space - eol;

			UTF8 = char_("\xef") >> char_("\xbb") >> char_("\xbf"); //UTF8 BOM

			comment	= 
				lit("#") 
				>> *(char_ - eol);

			eof = *(spc | comment | eol) >> eoi;
		}

		qi::rule<Iterator> start, spc, eof, comment, UTF8;
	};

	////////////////////////////
	//Ini Grammar.
	////////////////////////////

	string heading;

	void StoreHeading(string hdng) {
		heading = hdng;
	}

	void SetVar(string var, string value) {
		int intVal;
		bool bval;
		if (heading == "BOSS.RunOptions" || heading == "BOSSlog.Filters" || heading == "BOSSlog.Styles")
			return;
		if (heading == "GUI.LastOptions") {
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
			}
			intVal = atoi(value.c_str());
			if (intVal == 0)
				bval = false;
			else
				bval = true;
			if (var == "RunType")
				run_type = intVal;
			else if (var == "SilentRun")
				silent = bval;
			else if (var == "Debug")
				debug = bval;
			else if (var == "LogCLOutput")
				logCL = bval;
			else if (var == "CLVerbosity")
				verbosity = intVal;
			else if (var == "UpdateMasterlist")
				update = bval;
			else if (var == "SortNoVersionParse")
				sort_skip_version_parse = bval;
			else if (var == "SortDisplayCRCs")
				sort_show_CRCs = bval;
			else if (var == "DoTrialRun")
				trial_run = bval;
			else if (var == "RevertLevel")
				revert = intVal;
			else if (var == "RevertNoVersionParse")
				revert_skip_version_parse = bval;
			else if (var == "RevertDisplayCRCs")
				revert_show_CRCs = bval;
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
			section.name("section");
			setting.name("setting");
			var.name("variable");
			value.name("value");
		
			on_error<fail>(section,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
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

			expect = "&lt;" + expect + "&gt;";
			boost::replace_all(context, "\n", "<br />\n");
			string msg = "Ini parsing error occured, expected: " + expect + ", at: " + context;
			return;
		}
	};
}


#endif