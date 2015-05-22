#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/progress.hpp>

#include "Parsing.h"

namespace boss { namespace test {

	using std::string;

	class RulesMgr : public virtual boss::parsing::IRulesManager
	{
	protected:
		virtual void AddRule(boss::parsing::Rule const& rule);

		virtual void RulesMgr::SyntaxError(
				string::const_iterator const& begin, 
				string::const_iterator const& end, 
				string::const_iterator const& error_pos, 
				string const& what);

		virtual void ParsingFailed(
				string::const_iterator const& begin, 
				string::const_iterator const& end, 
				string::const_iterator const& error_pos, 
				string::difference_type lineNo);
	};


	void RulesMgr::AddRule(boss::parsing::Rule const& rule)
	{
		std::cout << rule << std::endl;
	};

	// Called when an error is detected while parsing the input file.
	void RulesMgr::SyntaxError(
			string::const_iterator const& begin, 
			string::const_iterator const& end, 
			string::const_iterator const& error_pos, 
			string const& what) 
	{
		std::string context(error_pos, std::min(error_pos + 50, end));
		boost::trim_left(context);
		boost::replace_all(context, "\n", "<EOL>");

		std::cerr << "Syntax error while trying to parse: '" << what << "' near this input: '" << context << "'." << std::endl;
	};

	void RulesMgr::ParsingFailed(
			string::const_iterator const& begin, 
			string::const_iterator const& end, 
			string::const_iterator const& error_pos, 
			string::difference_type lineNo)
	{
		string context(error_pos, std::min(error_pos + 40, end));
		boost::trim_left(context);
		boost::replace_all(context, "\n", "<EOL>");

		std::cerr << "Parsing error at line#: " << lineNo << " while reading near this input: '" << context << "'." << std::endl;
	};
}}

int main(int argc, char* argv[])
{
	using namespace boss::test;

	if (argc < 1)
	{
		std::cerr << "Error: You must specify a file to open in the command line." << std::endl;
		return 1;
	}

	std::ifstream in(argv[1], std::ios_base::in);

	if (!in)
	{
		std::cerr << "Error: Could not open input file: " << argv[1] << std::endl;
		return 1;
	}

	RulesMgr mgr;

	bool r = false;
	{
		boost::progress_timer timer(std::cout);
		r = boss::parsing::Userlist::Parse(in, mgr);
	}

	if (r)
	{
		std::cout << "-------------------------\n";
		std::cout << "Parsing succeeded\n";
		std::cout << "-------------------------\n";

		return 0;
	}
	else
	{
		std::cout << "-------------------------\n";
		std::cout << "Parsing failed\n";
		std::cout << "-------------------------\n";
		return 1;
	}

	return 0;
}

