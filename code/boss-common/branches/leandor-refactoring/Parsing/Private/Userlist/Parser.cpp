
// bring on the public stuff
#include "Parsing.h"

#include "Private/Defines.h"

#include "Private/Skipper.h"
#include "Private/Userlist/Grammar/Grammar.h"


namespace boss { namespace parsing {

	bool Userlist::Parse(std::istream& in, IRulesManager& manager)
	{
		std::string storage; // We will read the contents here.
		in.unsetf(std::ios::skipws); // No white space skipping!
		std::copy(
			std::istream_iterator<char>(in),
			std::istream_iterator<char>(),
			std::back_inserter(storage));

		return Parse(storage, manager);
	}

	bool Userlist::Parse(std::string& storage, IRulesManager& manager)
	{
		typedef detail::Skipper<std::string::const_iterator> Skipper;
		typedef detail::Grammar<std::string::const_iterator, Skipper> Grammar;

		Skipper skipper;
		Grammar grammar(manager);

		std::string::const_iterator iter = storage.begin();
		std::string::const_iterator end = storage.end();

		return qi::phrase_parse(iter, end, grammar, skipper) && (iter == end);
	}

}}