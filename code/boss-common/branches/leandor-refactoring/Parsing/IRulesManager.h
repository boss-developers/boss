#pragma once

#include "Model.h"

namespace boss { namespace parsing {

	// Rules management interface which used by the parser to define the elements as they are parsed.
	class IRulesManager 
	{
	public:
		virtual void AddRule(Rule const& rule) = 0;
	};

}}