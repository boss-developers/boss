#pragma once

#include <iostream>
#include <string>

#include "Parsing.h"

namespace boss { namespace parsing {

	class Userlist 
	{
	public:
		static bool Parse(std::istream& in, IRulesManager& manager);
		static bool Parse(std::string& text, IRulesManager& manager);
	};

}}