#include <algorithm>

// bring on the public stuff
#include "Parsing.h"

#include "Private/Defines.h"

#include "Private/Skipper.h"
#include "Private/Userlist/Grammar/Grammar.h"


namespace boss { namespace parsing {

	using std::istream;
	using std::string;

	bool Userlist::Parse(istream& in, IRulesManager& manager)
	{
		using std::ios;
		using std::istream_iterator;
		using std::copy;
		using std::back_inserter;

		string storage; // We will read the contents here.
		in.unsetf(ios::skipws); // No white space skipping!
		copy(
			istream_iterator<char>(in),
			istream_iterator<char>(),
			back_inserter(storage));

		return Parse(storage, manager);
	}

	bool Userlist::Parse(string& storage, IRulesManager& manager)
	{
		using std::count;
		typedef detail::Skipper<string::const_iterator> Skipper;
		typedef detail::Grammar<string::const_iterator, Skipper> Grammar;

		Skipper skipper;
		Grammar grammar(manager);

		string::iterator iter = storage.begin();
		string::iterator end = storage.end();

		bool success = qi::phrase_parse(iter, end, grammar, skipper) && (iter == end);

		if (!success) {
			while (*iter == '\n') {
				iter++;
			}

			string::difference_type lines = 1 + count(storage.begin(), iter, '\n');
			manager.ParsingFailed(storage.begin(), end, iter, lines);
		}

		return success;
	}
}}