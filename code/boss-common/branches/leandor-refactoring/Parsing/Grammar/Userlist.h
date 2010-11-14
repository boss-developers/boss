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
#include "Model/Keywords.h"
#include "Model/IRulesManager.h"
#include "Model/Model.h"


/*
 TODO: Make operation and action keywords case-insensitive.
 TODO: Fix the grammar to be more EOLN insensitive. 
	Just need to use the fact that text strings that acts as arguments 
	for operations and actions should contain only valid characters not EOLN, 
	but the rest of the grammar should be independent of EOLN placement.
 */

namespace boss { namespace parsing {

	// Defines the mapping from the strings to the keywords.
	struct rule_operation_ : qi::symbols<char, OperationKeyword>
	{
		rule_operation_()
		{
			add
				("ADD"			, OperationKeyword::ADD)
				("OVERRIDE"		, OperationKeyword::OVERRIDE)
				("FOR"			, OperationKeyword::FOR)
				;
		}

	} operation_;


	// Defines the mapping from the strings to the keywords.
	struct rule_action_ : qi::symbols<char, ActionKeyword>
	{
		rule_action_()
		{
			add
				("BEFORE"		, ActionKeyword::BEFORE)
				("AFTER"		, ActionKeyword::AFTER)
				("TOP"			, ActionKeyword::TOP)
				("BOTTOM"		, ActionKeyword::BOTTOM)
				("APPEND"		, ActionKeyword::APPEND)
				("REPLACE"		, ActionKeyword::REPLACE)
				;
		}

	} action_;

	template <typename Iterator, typename Skipper>
	class Userlist: public qi::grammar<Iterator, Skipper>
	{
	public:
		Userlist(IRulesManager& manager);
		~Userlist()
		{
		}


	private:
		void AddRule(Rule const& rule);

	private:
		IRulesManager& manager;

	private:
		qi::rule<Iterator, Skipper> rules;
		qi::rule<Iterator, Skipper> rule;
		qi::rule<Iterator, RuleHeader(), Skipper> header;
		qi::rule<Iterator, Rule(), Skipper> body;
		qi::rule<Iterator, RuleAction(), Skipper> action;
		qi::rule<Iterator, RuleActions(), Skipper> actions;
		qi::rule<Iterator, std::string(), Skipper> text;
		qi::rule<Iterator, std::string(), Skipper> string;
	};

}}

BOOST_FUSION_ADAPT_STRUCT(
	boss::parsing::Rule,
	(boss::parsing::RuleHeader, header)
	(boss::parsing::RuleActions, actions)
)

BOOST_FUSION_ADAPT_STRUCT(
	boss::parsing::RuleHeader,
	(boss::parsing::OperationKeyword, key)
	(std::string, subject)
)

BOOST_FUSION_ADAPT_STRUCT(
	boss::parsing::RuleAction,
	(boss::parsing::ActionKeyword, key)
	(std::string, argument)
)

namespace boss { namespace parsing {

	using qi::lit;
	using qi::lexeme;
	using qi::char_;
	using qi::eol;
	using qi::eoi;
	using qi::skip;
	using qi::eps;

	template <typename Iterator, typename Skipper>
	inline Userlist<Iterator, Skipper>::Userlist(IRulesManager& manager)
		: Userlist::base_type(rules, "userlist"), manager(manager)
	{
	
		// USERLIST :== list of RULE_LINE
		rules 
			%= rule  %	eol
			;  

		// RULE_LINE :== RULE HEADER followed by one or more SORT_LINEs
		rule 
			%=	*eol	// skip any dangling EOLN
			>>	(
					body [ phoenix::bind(&Userlist<Iterator, Skipper>::AddRule, this, qi::_1)  ]	// either a rule body
				|	eoi		// or we're done. (EOI :== END OF INPUT)
				)
			;

		// The RULE HEADER :== KEYWORD followed by ':' and then an object name just before the EOLN.
		header 
			%=	operation_
			>	lit(":") 
			>	text
			;

		body
			%=	header
			>	eol		// eat the header terminator: expect an EOLN before the sort lines start
			>>	actions
			;

		// SORT_LINEs :== list of SORT_LINE separated by EOLN
		actions 
			%=	*eol	// skip any dangling EOLN
			>>	action % eol
			;

		// SORT_LINE :== KEYWORD followed by ':' and then the header argument.
		action
			%=	action_
			>	lit(":")
			>	text
			// Don't eat the EOLN terminator!
			;

		// TEXT :== sequence of any chars until an EOLN
		text 
			%=	skip[eps] 
			>>	string
			;

		string
			%=	lexeme[				
					+(char_ - eol)
				]
			;

		BOOST_SPIRIT_DEBUG_NODE(rule);
		BOOST_SPIRIT_DEBUG_NODE(rules);
		BOOST_SPIRIT_DEBUG_NODE(header);
		BOOST_SPIRIT_DEBUG_NODE(body);
		BOOST_SPIRIT_DEBUG_NODE(action);
		BOOST_SPIRIT_DEBUG_NODE(actions);
		BOOST_SPIRIT_DEBUG_NODE(text);
	}

	template <typename Iterator, typename Skipper>
	inline void Userlist<Iterator, Skipper>::AddRule(Rule const& rule)
	{
		manager.AddRule(rule);
	}
}}

#endif
