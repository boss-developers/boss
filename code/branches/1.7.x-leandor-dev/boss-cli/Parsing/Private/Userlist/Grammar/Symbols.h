#pragma once


// bring on the public stuff
#include "Parsing.h"

// private stuff only
#include "Private/Defines.h"


namespace boss { namespace parsing { namespace detail {

	// Defines the mapping from the strings to the keywords.
	struct rule_operation_ : qi::symbols<char, OperationKeyword>
	{
		rule_operation_()
		{
			add
				("add"			, OperationKeyword::ADD)
				("override"		, OperationKeyword::OVERRIDE)
				("for"			, OperationKeyword::FOR)
				;
		}

	} operation_;


	// Defines the mapping from the strings to the keywords.
	struct rule_action_ : qi::symbols<char, ActionKeyword>
	{
		rule_action_()
		{
			add
				("before"		, ActionKeyword::BEFORE)
				("after"		, ActionKeyword::AFTER)
				("top"			, ActionKeyword::TOP)
				("bottom"		, ActionKeyword::BOTTOM)
				("append"		, ActionKeyword::APPEND)
				("replace"		, ActionKeyword::REPLACE)
				;
		}

	} action_;

}}}