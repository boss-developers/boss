#pragma once

#include "Parsing.h"

namespace boss { namespace parsing {

	// Defines the Rule starting keywords
	struct OperationKeyword
	{
		enum OperationValue
		{
			Empty,
			ADD,
			OVERRIDE,
			FOR
		}; 

		OperationValue Value;

		OperationKeyword() : Value(OperationKeyword::Empty) { };
		OperationKeyword(const OperationValue value) : Value(value) { };
		OperationKeyword(const OperationKeyword& keyword) : Value(keyword.Value) { };
	};

	// Defines the Rule starting keywords
	struct ActionKeyword
	{
		enum ActionValue
		{
			Empty,
			BEFORE,
			AFTER,
			TOP,
			BOTTOM,
			APPEND,
			REPLACE,
		}; 

		ActionValue Value;

		ActionKeyword() : Value(ActionKeyword::Empty) { };
		ActionKeyword(const ActionValue value) : Value(value) { };
		ActionKeyword(const ActionKeyword& keyword) : Value(keyword.Value) { };
	};

}}