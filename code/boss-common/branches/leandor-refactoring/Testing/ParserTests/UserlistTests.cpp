#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/progress.hpp>

#include "Parsing.h"


class RulesMgr : public virtual boss::parsing::IRulesManager
{
protected:
	virtual void AddRule(boss::parsing::Rule const& rule);
};


void RulesMgr::AddRule(boss::parsing::Rule const& rule)
{
	std::cout << rule << std::endl;
};

int main(int argc, char* argv[])
{
	char const* filename;
	if (argc > 1)
	{
		filename = argv[1];
	}
	else
	{
		filename = "BOSS\\masterlist.txt";
	}

	std::ifstream in(filename, std::ios_base::in);

	if (!in)
	{
		std::cerr << "Error: Could not open input file: " << filename << std::endl;
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

