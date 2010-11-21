#pragma once

#include <boost/spirit/include/qi_no_case.hpp>

// bring on the public stuff
#include "Parsing.h"

// private stuff only
#include "Private/Defines.h"
#include "Private/Userlist/Grammar/Grammar.h"

namespace boss { namespace parsing { namespace detail {

	using qi::lit;
	using qi::lexeme;
	using qi::char_;
	using qi::eol;
	using qi::eoi;
	using qi::skip;
	using qi::eps;
	using iso8859_1::no_case;

	template <typename Iterator, typename Skipper>
	Grammar<Iterator, Skipper>::Grammar(IRulesManager& manager)
		: Grammar::base_type(rules, "userlist"), manager(manager)
	{
	
		// USERLIST :== list of RULE_LINE
		rules 
			%= rule  %	eol
			;  

		// RULE_LINE :== RULE HEADER followed by one or more SORT_LINEs
		rule 
			%=	*eol	// skip any dangling EOLN
			>>	(
					body [ phoenix::bind(&Grammar<Iterator, Skipper>::AddRule, this, qi::_1)  ]	// either a rule body
				|	eoi		// or we're done. (EOI :== END OF INPUT)
				)
			;

		// The RULE HEADER :== KEYWORD followed by ':' and then an object name just before the EOLN.
		header 
			%=	no_case[operation_]
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
			%=	no_case[action_]
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
	Grammar<Iterator, Skipper>::~Grammar()
	{
	}

	template <typename Iterator, typename Skipper>
	void Grammar<Iterator, Skipper>::AddRule(Rule const& rule)
	{
		manager.AddRule(rule);
	}

}}}
