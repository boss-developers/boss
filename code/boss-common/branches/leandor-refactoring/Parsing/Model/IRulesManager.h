#pragma once

#include "Parsing.h"

namespace boss { namespace parsing {

	// Rules management interface which used by the parser to define the elements as they are parsed.
	class IRulesManager {
		virtual void Add() = 0;
		virtual void Override() = 0;
		virtual void For() = 0;
	};

}}