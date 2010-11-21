#pragma once

#include <boost/spirit/include/qi_no_case.hpp>
//#include <boost/format.hpp>

#include <algorithm>

// bring on the public stuff
#include "Parsing.h"

// private stuff only
#include "Private/Defines.h"
#include "Private/Userlist/Grammar/Grammar.h"

namespace boss { namespace parsing { namespace detail {

	using namespace qi::labels;
	using qi::lit;
	using qi::lexeme;
	using qi::char_;
	using qi::eol;
	using qi::eoi;
	using qi::skip;
	using qi::eps;
	using iso8859_1::no_case;
	
	using qi::fail;
	using qi::on_error;

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
			%=	operation_keyword
			>	lit(":") 
			>	text
			;

		body
			%=	header
			>	eol		// eat the header terminator: expect an EOLN before the sort lines start
			>	actions
			;

		// SORT_LINEs :== list of SORT_LINE separated by EOLN
		actions 
			%=	*eol	// skip any dangling EOLN
			>>	action % eol
			;

		// SORT_LINE :== KEYWORD followed by ':' and then the header argument.
		action
			%=	action_keyword
			>	lit(":")
			>	text
			// Don't eat the EOLN terminator!
			;

		// TEXT :== sequence of any chars until an EOLN
		text 
			%=	skip[eps] 
			>	string
			;

		string
			%=	lexeme[				
					+(char_ - eol)
				]
			;

		operation_keyword 
			%=	no_case[operation_]
			;

		action_keyword 
			%=	no_case[action_]
			;

		BOOST_SPIRIT_DEBUG_NODE(operation_keyword);
		BOOST_SPIRIT_DEBUG_NODE(action_keyword);
		BOOST_SPIRIT_DEBUG_NODE(rule);
		BOOST_SPIRIT_DEBUG_NODE(rules);
		BOOST_SPIRIT_DEBUG_NODE(header);
		BOOST_SPIRIT_DEBUG_NODE(body);
		BOOST_SPIRIT_DEBUG_NODE(action);
		BOOST_SPIRIT_DEBUG_NODE(actions);
		BOOST_SPIRIT_DEBUG_NODE(text);


		on_error<fail>(
			rules,
			phoenix::bind(&Grammar<Iterator, Skipper>::SyntaxError, this, _1, _2, _3, _4)
		);

		on_error<fail>(
			rule,
			phoenix::bind(&Grammar<Iterator, Skipper>::SyntaxError, this, _1, _2, _3, _4)
		);

		on_error<fail>(
			header,
			phoenix::bind(&Grammar<Iterator, Skipper>::SyntaxError, this, _1, _2, _3, _4)
		);
		
		on_error<fail>(
			body,
			phoenix::bind(&Grammar<Iterator, Skipper>::SyntaxError, this, _1, _2, _3, _4)
		);		

		on_error<fail>(
			actions,
			phoenix::bind(&Grammar<Iterator, Skipper>::SyntaxError, this, _1, _2, _3, _4)
		);
		
		on_error<fail>(
			action,
			phoenix::bind(&Grammar<Iterator, Skipper>::SyntaxError, this, _1, _2, _3, _4)
		);
		
		on_error<fail>(
			text,
			phoenix::bind(&Grammar<Iterator, Skipper>::SyntaxError, this, _1, _2, _3, _4)
		);		

		on_error<fail>(
			operation_keyword,
			phoenix::bind(&Grammar<Iterator, Skipper>::SyntaxError, this, _1, _2, _3, _4)
		);		

		on_error<fail>(
			action_keyword,
			phoenix::bind(&Grammar<Iterator, Skipper>::SyntaxError, this, _1, _2, _3, _4)
		);		
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

	template <typename Iterator, typename Skipper>
	void Grammar<Iterator, Skipper>::SyntaxError(Iterator const& first, Iterator const& last, Iterator const& errorpos, info const& what)
	{			
		const std::string context(errorpos, errorpos + 10);
		std::cerr << "Syntax Error: Unable to parse: '" << what << "' from this context: '" << context << "'." << std::endl;
	}

}}}
