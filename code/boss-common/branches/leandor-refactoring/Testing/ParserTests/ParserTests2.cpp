
#define BOOST_SPIRIT_ASSERT_EXCEPTION
#define	BOOST_SPIRIT_DEBUG
#define	BOOST_SPIRIT_DEBUG_TRACENODE

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

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "../Common/Masterlist.h"


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
};


namespace test {
	
	// Skipper grammar

	template <typename Iterator>
	struct Skipper
		: qi::grammar<Iterator>
	{
		Skipper()
			: Skipper::base_type(start, "skipper")
		{
			using qi::eol;
			using iso8859_1::char_;
			using namespace qi::labels;
			using iso8859_1::space;

			start 
				=	spc
				|	comments >> *eol
				;

			spc 
				= space - eol;

			comment	
				=	lit("\\") >> -string
				;
				;

			comments
				=	comment % eol
				;

			string
				=	+(char_ - eol)
				;


			start.name("skip");
			spc.name("space");
			string.name("chars");
			comment.name("comment");
			comments.name("comments");

			//debug(start);
			//debug(spc);
			//debug(string);
			//debug(comment);
			//debug(comments);
		} 

		qi::rule<Iterator> start;
		qi::rule<Iterator> spc;
		qi::rule<Iterator> string;
		qi::rule<Iterator> comment;
		qi::rule<Iterator> comments;
	};

	// Real grammar

	typedef std::string text_type;
	typedef std::vector<text_type> masterlist_type;


	void print_text(std::string const& x)
	{
		std::cout << x << std::endl;
	}

	void print(masterlist_type const& x)
	{
		for (masterlist_type::const_iterator it = x.begin(); it != x.end(); it++)
		{
			print_text(*it);
		}
	}

	template <typename Iterator, typename Skipper>
	struct Grammar
		: qi::grammar<Iterator, masterlist_type(), Skipper>
	{
		Grammar()
			: Grammar::base_type(masterlist, "masterlist")
		{
			using iso8859_1::char_;
			using namespace qi::labels;
			using qi::on_error;
			using qi::fail;
			using qi::no_skip;
			using phoenix::construct;
			using phoenix::val;

			text
				%=	*eol >> no_skip[string[&test::print_text]];

			masterlist 
				%=  text % eol >> *eol
				;

			string
				=	+(char_ - eol)
				;


			masterlist.name("masterlist");
			string.name("string");
			text.name("text");

			//debug(masterlist);
			//debug(string);
			//debug(text);

			on_error<fail>(masterlist, 
				std::cout
				<< val("Error! Expecting ")
				<< _4 // what failed?
				<< val(" here: \"")
				<< construct<std::string>(_3, _2) // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);

			on_error<fail>(text, 
				std::cout
				<< val("Error! Expecting ")
				<< _4 // what failed?
				<< val(" here: \"")
				<< construct<std::string>(_3, _2) // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);
		}

		qi::rule<Iterator, masterlist_type(), Skipper> masterlist;
		qi::rule<Iterator, std::string()> string;
		qi::rule<Iterator, text_type()> text;
	};
};


namespace test {

};

int main(int argc, char* argv[])
{
	namespace iso8859_1 = boost::spirit::iso8859_1;
	namespace qi = boost::spirit::qi;

	using qi::eol;
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

	typedef test::Skipper<std::string::const_iterator> Skipper;
	typedef test::Grammar<std::string::const_iterator, Skipper> Grammar;

	Grammar grammar; // Our grammar
	Skipper skipper; // Our skipper

	test::masterlist_type data;

	std::string::const_iterator iter = storage.begin();
	std::string::const_iterator end = storage.end();
	bool r = phrase_parse(iter, end, grammar, skipper, data);

	if (r && iter == end)
	{
		std::cout << "-------------------------\n";
		std::cout << "Parsing succeeded\n";
		std::cout << "-------------------------\n";
		test::print(data);
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

