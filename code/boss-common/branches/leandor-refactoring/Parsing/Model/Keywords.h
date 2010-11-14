#pragma once

#include "Parsing.h"

namespace boss { namespace parsing {

	// Defines the Rule starting keywords
	struct RuleOperation
	{
		enum Operation
		{
			ADD,
			OVERRIDE,
			FOR
		}; 

		Operation Value;

		RuleOperation(const Operation value) : Value(value) { };
		RuleOperation(const RuleOperation& keyword) : Value(keyword.Value) { };
	};

	// Defines the Rule starting keywords
	struct RuleAction
	{
		enum Action
		{
			BEFORE,
			AFTER,
			TOP,
			BOTTOM,
			APPEND,
			REPLACE,
		}; 

		Action Value;

		RuleAction(const Action value) : Value(value) { };
		RuleAction(const RuleAction& keyword) : Value(keyword.Value) { };
	};

}}