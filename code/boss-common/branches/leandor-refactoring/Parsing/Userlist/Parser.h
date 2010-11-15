#pragma once

#include <iostream>
#include <string>

#include "Parsing.h"

namespace boss { namespace parsing {

	class Userlist 
	{
	public:
		static void Parse(std::istream& is, IRulesManager& manager);
		static void Parse(std::string& text, IRulesManager& manager);
	};

}}