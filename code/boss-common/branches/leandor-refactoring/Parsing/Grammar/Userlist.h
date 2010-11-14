#ifndef __USERLIST_GRAMMAR_HPP__
#define __USERLIST_GRAMMAR_HPP__

#ifdef _MSC_VER
#pragma once
#endif

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_no_skip.hpp>

#include <string>
#include <vector>

#include "Parsing.h"
#include "Model/IRulesManager.h"
#include "Model/Keywords.h"


namespace boss { namespace parsing {

	// Defines the mapping from the strings to the keywords.
	struct rule_operation_ : qi::symbols<char, RuleOperation>
	{
		rule_operation_()
		{
			add
				("ADD"			, RuleOperation::ADD)
				("OVERRIDE"		, RuleOperation::OVERRIDE)
				("FOR"			, RuleOperation::FOR)
				;
		}

	} operation;


	// Defines the mapping from the strings to the keywords.
	struct rule_action_ : qi::symbols<char, RuleAction>
	{
		rule_action_()
		{
			add
				("BEFORE"		, RuleAction::BEFORE)
				("AFTER"		, RuleAction::AFTER)
				("TOP"			, RuleAction::TOP)
				("BOTTOM"		, RuleAction::BOTTOM)
				("APPEND"		, RuleAction::APPEND)
				("REPLACE"		, RuleAction::REPLACE)
				;
		}

	} action;

	template <typename Iterator, typename Skipper>
	class Userlist: public qi::grammar<Iterator, Skipper>
	{
	public:
		Userlist();
		~Userlist()
		{
		}

		

	private:
		qi::rule<Iterator, Skipper> rule;
		qi::rule<Iterator, Skipper> rules;
		qi::rule<Iterator, Skipper> header;
		qi::rule<Iterator, Skipper> body;
		qi::rule<Iterator, Skipper> directive;
		qi::rule<Iterator, Skipper> directives;
		qi::rule<Iterator, Skipper> message_line;
		qi::rule<Iterator, std::string(), Skipper> text;
		qi::rule<Iterator, std::string(), Skipper> string;
	};

}}


namespace boss { namespace parsing {


	class Rule 
	{

	private:
		RuleOperation		kind;
		std::string		object;
	};

	class Rules
	{


	private:
		std::vector<Rule> rules;
	};

}}


namespace boss { namespace parsing {

	using qi::lit;
	using qi::lexeme;
	using qi::char_;
	using qi::eol;
	using qi::eoi;
	using qi::skip;
	using qi::eps;

	template <typename Iterator, typename Skipper>
	inline Userlist<Iterator, Skipper>::Userlist()
		: Userlist::base_type(rules, "userlist")
	{
	
		// USERLIST :== list of RULE_LINE
		rules 
			= rule  %	eol
			;  

		// RULE_LINE :== RULE HEADER followed by one or more SORT_LINEs
		rule 
			=	*eol	// skip any dangling EOLN
			>>	(
					body	// either a rule body
				|	eoi		// or we're done. (EOI :== END OF INPUT)
				)
			;

		// The RULE HEADER :== KEYWORD followed by ':' and then an object name just before the EOLN.
		header 
			=	operation
			>	lit(":") 
			>	text
			;

		body
			=	header
			>	eol		// eat the header terminator: expect an EOLN before the sort lines start
			>>	directives
			;

		// SORT_LINEs :== list of SORT_LINE separated by EOLN
		directives 
			=	*eol	// skip any dangling EOLN
			>>	directive % eol
			;

		// SORT_LINE :== KEYWORD followed by ':' and then the header argument.
		directive
			=	action
			>	lit(":")
			>	text
			// Don't eat the EOLN terminator!
			;

		// TEXT :== sequence of any chars until an EOLN
		text 
			=	skip[eps] 
			>>	string
			;

		string
			=	lexeme[				
					+(char_ - eol)
				]
			;

		BOOST_SPIRIT_DEBUG_NODE(rule);
		BOOST_SPIRIT_DEBUG_NODE(rules);
		BOOST_SPIRIT_DEBUG_NODE(header);
		BOOST_SPIRIT_DEBUG_NODE(body);
		BOOST_SPIRIT_DEBUG_NODE(directive);
		BOOST_SPIRIT_DEBUG_NODE(directives);
		BOOST_SPIRIT_DEBUG_NODE(text);
	}

}}

#endif
