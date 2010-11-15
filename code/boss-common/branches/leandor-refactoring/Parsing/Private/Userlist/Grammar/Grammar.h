#ifndef __USERLIST_GRAMMAR_HPP__
#define __USERLIST_GRAMMAR_HPP__

#ifdef _MSC_VER
#pragma once
#endif

#include <string>
#include <vector>

// bring on the public stuff
#include "Parsing.h"

// private stuff only
#include "Private/Defines.h"
#include "Private/Userlist/Grammar/Symbols.h"

/*
 TODO: Make operation and action keywords case-insensitive.
 TODO: Fix the grammar to be more EOLN insensitive. 
	Just need to use the fact that text strings that acts as arguments 
	for operations and actions should contain only valid characters not EOLN, 
	but the rest of the grammar should be independent of EOLN placement.
 */

namespace boss { namespace parsing { namespace detail {

	template <typename Iterator, typename Skipper>
	class Grammar: public qi::grammar<Iterator, Skipper>
	{
	public:
		Grammar(IRulesManager& manager);
		~Grammar();

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

}}}

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

#include "Private/Userlist/Grammar/GrammarCode.h"

#endif


