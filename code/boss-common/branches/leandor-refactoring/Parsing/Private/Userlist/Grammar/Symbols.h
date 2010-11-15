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

}}}