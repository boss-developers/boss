
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
				=	lit("\\") 
				>>	!lit("BeginGroup")
				>>	!lit("EndGroup")
				>> -string
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

	typedef 
		std::string 
		mod_type;

	struct Group;

	typedef 
		Group 
		group_type;

	typedef
		boost::variant<
				boost::recursive_wrapper<group_type>
			,	mod_type
			>
		node_type;

	typedef 
		std::vector<node_type> 
		nodes_type;

	struct Group
	{
		string name;
		nodes_type nodes;
	};

	typedef 
		std::vector<group_type> 
		groups_type;

	typedef 
		groups_type 
		masterlist_type;

};

namespace test {

	int const tabsize = 4;

	void tab(int indent)
	{
		for (int i = 0; i < indent; ++i)
			std::cout << ' ';
	}

	struct group_printer
	{
		group_printer(int indent = 0)
			: indent(indent)
		{
		}

		void operator()(group_type const& group) const;

		int indent;
	};

	struct node_printer : boost::static_visitor<>
	{
		node_printer(int indent = 0)
			: indent(indent)
		{
		}

		void operator()(group_type const& group) const
		{
			group_printer(indent+tabsize)(group);
		}

		void operator()(mod_type const& mod) const
		{
			tab(indent+tabsize);
			std::cout << "mod: \"" << mod << '"' << std::endl;
		}

		int indent;
	};

	void group_printer::operator()(group_type const& group) const
	{
		tab(indent);
		std::cout << "group: " << group.name << std::endl;
		tab(indent);
		std::cout << "begin" << std::endl;

		BOOST_FOREACH(node_type const& node, group.nodes)
		{
			boost::apply_visitor(node_printer(indent), node);
		}

		tab(indent);
		std::cout << "end" << std::endl;
	}

	void print(groups_type const& groups)
	{
		group_printer printer;

		BOOST_FOREACH(group_type const& group, groups)
		{
			printer(group);
		}
	}
};

BOOST_FUSION_ADAPT_STRUCT(
	test::Group,
	(std::string, name)
	(test::nodes_type, nodes)
	)


namespace test {

	template <typename Iterator, typename Skipper>
	struct Grammar
		: qi::grammar<Iterator, groups_type(), Skipper>
	{
		Grammar()
			: Grammar::base_type(masterlist, "masterlist")
		{
			using iso8859_1::char_;
			using namespace qi::labels;
			using qi::space;
			using qi::on_error;
			using qi::fail;
			using qi::no_skip;
			using phoenix::construct;
			using phoenix::val;

			text
				%=	skip[string];

			masterlist 
				%=  group % eol >> *eol
				;

			string
				=	+(char_ - eol)
				;

			mod %=	!endgroup 
				>>	!begingroup
				>>	text;

			//mods 
			//	%=	mod % eol >> -eol;

			node
				%=	*eol 
				>>	(
						group
					|	mod
					)
				;

			nodes
				%=	node % eol >> *eol
				;

			begingroup 
				%=	lit("\\BeginGroup\\:")
				>>	text 
				>	eol
				;

			endgroup
				=	lit("\\EndGroup\\\\");

			group
				%=	*eol
				>>	begingroup
				>>	nodes
				>	endgroup
				;

			//groups 
			//	%= group % eol >> -eol
			//	;

			masterlist.name("masterlist");
			string.name("string");
			text.name("text");
			mod.name("mod");
			//mods.name("mods");
			group.name("group");
			node.name("node");
			nodes.name("nodes");
			begingroup.name("begin_group");
			endgroup.name("end_group");

			debug(masterlist);
			debug(group);
			//debug(groups);
			debug(nodes);
			debug(node);
			debug(begingroup);
			debug(endgroup);
			debug(string);
			debug(text);
			//debug(mods);
			debug(mod);

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
		qi::rule<Iterator, std::string(), Skipper> string;
		qi::rule<Iterator, std::string(), Skipper> text;
		qi::rule<Iterator, mod_type(), Skipper> mod;
		//qi::rule<Iterator, mods_type()> mods;
		qi::rule<Iterator, node_type(), Skipper> node;
		qi::rule<Iterator, nodes_type(), Skipper> nodes;
		qi::rule<Iterator, group_type(), Skipper> group;
		//qi::rule<Iterator, groups_type()> groups;
		qi::rule<Iterator, std::string(), Skipper> begingroup;
		qi::rule<Iterator, Skipper> endgroup;
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

	test::groups_type data;

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

