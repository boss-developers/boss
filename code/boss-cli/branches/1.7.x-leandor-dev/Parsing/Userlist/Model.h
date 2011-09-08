#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <boost/foreach.hpp>

#include "Userlist/Keywords.h"

namespace boss { namespace parsing {


	struct RuleAction
	{
		ActionKeyword		key;
		std::string			argument;
	};

	struct RuleActions : public std::vector<RuleAction> 
	{
	};


	struct RuleHeader
	{
		OperationKeyword	key;
		std::string			subject;
	};

	struct Rule 
	{
		RuleHeader			header;
		RuleActions			actions;
	};

	struct Rules : public std::vector<Rule>
	{
	};

	//- Helper operators for printing the resulting structure
	std::ostream& operator<< (std::ostream& os, ActionKeyword const& action);
	std::ostream& operator<< (std::ostream& os, OperationKeyword const& operation);
	std::ostream& operator<< (std::ostream& os, RuleHeader const& header);
	std::ostream& operator<< (std::ostream& os, RuleAction const& action);
	std::ostream& operator<< (std::ostream& os, RuleActions const& actions);
	std::ostream& operator<< (std::ostream& os, Rule const& rule);
	std::ostream& operator<< (std::ostream& os, Rules const& rules);
}}