/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Header file for userlist grammar definitions.

/* Need to:
1. Get error handling working properly (currently throws silent exceptions and doesn't specify area).
2. Make error reports useful.
2. Add content checks.
3. Make content checks actually affect final parsing product.
*/

#ifndef __BOSS_USERLIST_GRAM_H__
#define __BOSS_USERLIST_GRAM_H__

#include "Parsing/Data.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

namespace boss {
	namespace ascii = boost::spirit::ascii;
	namespace phoenix = boost::phoenix;
	namespace qi = boost::spirit::qi;

	using qi::skip;
	using qi::eol;
	using qi::eoi;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;
	using qi::lit;

	using ascii::char_;
	using ascii::space;
	using ascii::space_type;
	using ascii::no_case;

	using boost::spirit::info;

////////////////////////////
//Userlist Grammar.
////////////////////////////

	template <typename Iterator>
	struct userlist_skipper : qi::grammar<Iterator> {

		userlist_skipper() : userlist_skipper::base_type(start, "userlist_skipper") {

			start = 
				spc
				| comment
				| eof;

			spc = space - eol;

			comment	
				= lit("//") 
				>> *(char_ - eol);

			eof = *eol >> eoi;
		}

		qi::rule<Iterator> start;
		qi::rule<Iterator> spc;
		qi::rule<Iterator> comment;
		qi::rule<Iterator> eof;
	};

/* NOTES: USERLIST GRAMMAR RULES.

	Below are the grammar rules that the parser must follow. Noted here until they are implemented.

S	1. Userlist must be encoded in UTF-8 or UTF-8 compatible ANSI. Use checking functions and abort the userlist if mangled line found.
S	2. All lines must contain a recognised keyword and an object. If one is missing or unrecognised, abort the rule.
T	3. If a rule object is a mod, it must be installed. If not, abort the rule.
T	4. Groups cannot be added. If a rule tries, abort it.
T	5. The 'ESMs' group cannot be sorted. If a rule tries, abort it.
T	6. The game's main master file cannot be sorted. If a rule tries, abort it.
	7. A rule with a FOR rule keyword must not contain a sort line. If a rule tries, ignore the line and print a warning message.
	8. A rule may not reference a mod and a group unless its sort keyword is TOP or BOTTOM and the mod is the rule object.  If a rule tries, abort it.
T	9. No group may be sorted before the 'ESMs' group. If a rule tries, abort it.
T	10. No mod may be sorted before the game's main master file. If a rule tries, abort it.
T	11. No mod may be inserted into the top of the 'ESMs' group. If a rule tries, abort it.
	12. No rule can insert a group into anything or insert anything into a mod. If a rule tries, abort it.
	13. No rule may attach a message to a group. If a rule tries, abort it.
S	14. The first line of a rule must be a rule line. If there is a valid line before the first rule line, ignore it and print a warning message.
*/

	template <typename Iterator>
	struct userlist_grammar : qi::grammar<Iterator, std::vector<rule>(), userlist_skipper<Iterator> > {
		userlist_grammar() : userlist_grammar::base_type(list, "userlist_grammar") {

			list %= rule % eol; //A list is a vector of rules, terminating in an end of input. Rules are separated by line endings.

			rule %=
				*eol
				> (((headerKey
				> ':'
				> object)
				> eol
				> body));

			body %= 
				*eol
				> (bodyLine % +eol);

			bodyLine %=
				bodyKey
				> ':'
				> object;

			headerKey %= no_case[ruleKeys];

			object %= lexeme[skip("//" >> *(char_ - eol))[+(char_ - eol)]];

			bodyKey %= no_case[lineKeys];

			//Give each rule names.
			list.name("list");
			rule.name("rule");
			body.name("body");
			bodyLine.name("bodyLine");
			headerKey.name("headerKey");
			object.name("object");
			bodyKey.name("bodyKey");
		
			on_error<fail>(list,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(rule,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(body,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(bodyLine,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(headerKey,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(object,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
			on_error<fail>(bodyKey,phoenix::bind(&userlist_grammar<Iterator>::SyntaxError,this,qi::_1,qi::_2,qi::_3,qi::_4));
		}

		typedef userlist_skipper<Iterator> skipper;

		qi::rule<Iterator, std::vector<rule>(), skipper> list;
		qi::rule<Iterator, rule(), skipper> rule;
		qi::rule<Iterator, std::vector<line>(), skipper> body;
		qi::rule<Iterator, line(), skipper> bodyLine;
		qi::rule<Iterator, keyType(), skipper> headerKey,bodyKey;
		qi::rule<Iterator, std::string(), skipper> object;
	
		void SyntaxError(Iterator const& first, Iterator const& last, Iterator const& errorpos, info const& what) {
			std::cout
				<< "Error! Expecting "
				<< what                            // what failed?
				<< " here: \""
				<< phoenix::construct<std::string>(errorpos,last)   // iterators to error-pos, end
				<< "\""
				<< std::endl;
		}
	};
}

/*
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
	*/
#endif