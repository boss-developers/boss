#pragma once

#include <string>

#include "Userlist/Model.h"

namespace boss { namespace parsing {

	using std::string;

	// Rules management interface which used by the parser to define the elements as they are parsed.
	class IRulesManager
	{
	public:
		// Called when a Rule is fully parsed successfully
		virtual void AddRule(Rule const& rule) = 0;

		// Called when an error is detected while parsing the input file.
		virtual void SyntaxError(
				string::const_iterator const& begin, 
				string::const_iterator const& end, 
				string::const_iterator const& error_pos, 
				string const& what) = 0;

		// Called when parsing finalized with failure
		virtual void ParsingFailed(
				string::const_iterator const& begin, 
				string::const_iterator const& end, 
				string::const_iterator const& error_pos, 
				string::difference_type lineNo) = 0;
	};

}}