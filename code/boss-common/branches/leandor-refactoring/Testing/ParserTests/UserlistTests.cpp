#define BOOST_SPIRIT_ASSERT_EXCEPTION
#define	BOOST_SPIRIT_DEBUG
#define	BOOST_SPIRIT_DEBUG_TRACENODE
#define BOOST_SPIRIT_DEBUG_OUT std::cout

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_no_skip.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/foreach.hpp>
#include <boost/progress.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Parsing.h"
#include "Grammar/Skipper.h"
#include "Grammar/Userlist.h"
#include "../Common/Masterlist.h"


// Definitions
namespace test {

	using namespace std;

	namespace fusion = boost::fusion;
	namespace phoenix = boost::phoenix;
	namespace qi = boost::spirit::qi;
	namespace iso8859_1 = boost::spirit::iso8859_1;

	using qi::lit;
	using qi::lexeme;
	using qi::skip;
	using qi::eol;
	using qi::eoi;
	using qi::omit;
	using fusion::unused_type;
};

int main(int argc, char* argv[])
{
	namespace qi = boost::spirit::qi;
	namespace iso8859_1 = boost::spirit::iso8859_1;

	using qi::eol;
	using qi::space_type;
	using iso8859_1::space;

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
		std::cerr << "Error: Could not open input file: "
			<< filename << std::endl;
		return 1;
	}

	std::string storage; // We will read the contents here.
	in.unsetf(std::ios::skipws); // No white space skipping!
	std::copy(
		std::istream_iterator<char>(in),
		std::istream_iterator<char>(),
		std::back_inserter(storage));

	typedef boss::parsing::Skipper<std::string::const_iterator> Skipper;
	typedef boss::parsing::Userlist<std::string::const_iterator, Skipper> UserlistGrammar;

	Skipper skipper;
	UserlistGrammar grammar; // Our grammar

	std::string::const_iterator iter = storage.begin();
	std::string::const_iterator end = storage.end();

	
	bool r;
	{
		boost::progress_timer timer(std::cout);
		r = phrase_parse(iter, end, grammar, skipper);
	}
	if (r && iter == end)
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

