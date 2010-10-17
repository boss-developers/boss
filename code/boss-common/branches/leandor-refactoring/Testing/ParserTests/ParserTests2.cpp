
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

// Skipper
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
}

// AST
namespace test {
	
	struct Group;
	struct Mod;
	struct Rule;

	typedef
		std::vector<Rule>
		Rules;

	typedef
		boost::variant<
				boost::recursive_wrapper<Group>
			,	Mod
			,	unused_type
			>
		Node;

	typedef 
		std::vector<Node> 
		Nodes;

	struct Group
	{
		string name;
		Nodes nodes;
	};

	struct Mod 
	{
		string name;
		Rules rules;
	};

	struct Rule 
	{
		struct Scope 
		{
			enum ScopeValue
			{
				Always,
				Fcom				=	'>',
				NonFcom				=	'<',
				OOO					=	'$',
				BC					=	'^',
			}; 

			ScopeValue value;

			Scope() : value(Always) { };
			Scope(const ScopeValue value) : value(value) { };
			Scope(const Scope& scope) : value(scope.value) { };
		};

		struct Kind
		{
			enum KindValue
			{
				Unknown,
				Remark				=	'?',
				Tags				=	'%',
				Error				=	'*',	
				Requirement			=	':',
				Incompatibility		=	'"',
			};

			KindValue value;

			Kind() : value(Unknown) { };
			Kind(const KindValue value) : value(value) { };
			Kind(const Kind& type) : value(type.value) { };
		};

		Scope	scope;
		Kind	kind;
		string	text;
	};

	ostream& operator<< (ostream& os, Rule const& rule)
	{
		return os << rule.text;
	}

	ostream& operator<< (ostream& os, Rule::Scope const& scope)
	{
		switch (scope.value)
		{
			case Rule::Scope::Fcom :
				return os << "FCOM only";
			case Rule::Scope::NonFcom :
				return os << "Non-FCOM only";
			case Rule::Scope::OOO :
				return os << "OOO only";
			case Rule::Scope::BC :
				return os << "Better Cities only";
			case Rule::Scope::Always :
				return os << "Always";
		}

		return os;
	}

	ostream& operator<< (ostream& os, Rule::Kind const& kind)
	{
		switch (kind.value)
		{
			case Rule::Kind::Error :
				return os << "ERROR";
			case Rule::Kind::Incompatibility :
				return os << "INCOMPATIBILITY";
			case Rule::Kind::Remark :
				return os << "REMARK";
			case Rule::Kind::Requirement :
				return os << "REQUIREMENT";
			case Rule::Kind::Tags :
				return os << "TAG";
				
			return os;
		}

		return os;
	}

	typedef 
		std::vector<Group> 
		Groups;

	typedef 
		Groups
		Masterlist;
};

// Print functors
namespace test {

	int const tabsize = 4;

	void tab(int indent)
	{
		for (int i = 0; i < indent * tabsize; ++i)
			std::cout << ' ';
	}

	struct group_printer
	{
		group_printer(int indent = 0)
			: indent(indent)
		{
		}

		void operator()(Group const& group) const;

		int indent;
	};

	struct rule_printer
	{
		rule_printer(int indent)
			: indent(indent)
		{
		}

		void operator()(Rule const& rule) const
		{
			tab(indent);
			std::cout << "RULE [" << rule.scope  << "] " << rule.kind << ": \"" << rule.text << "\"" << std::endl;
		}

		int indent;
	};

	struct mod_printer
	{
		mod_printer(int indent)
			: indent(indent)
		{
		}

		void operator()(Mod const& mod) const
		{
			tab(indent); std::cout << "MOD: \"" << mod.name << '"' << std::endl;

			print(mod.rules);
		}

		void print(Rules const& rules) const
		{
			rule_printer printer(indent + 1);
			BOOST_FOREACH(Rule const& rule, rules)
			{
				printer(rule);
			}
		}

		int indent;
	};

	struct node_printer : boost::static_visitor<>
	{
		node_printer(int indent = 0)
			: indent(indent)
		{
		}

		void operator()(Group const& group) const
		{
			group_printer printer(indent);
			printer(group);
		}

		void operator()(Mod const& mod) const
		{
			mod_printer printer(indent);
			printer(mod);
		}

		void operator()(unused_type const&) const
		{
		}

		int indent;
	};

	void group_printer::operator()(Group const& group) const
	{
		tab(indent); std::cout << "GROUP: \"" << group.name << "\"" << std::endl;
		tab(indent); std::cout << "BEGIN" << std::endl;

		node_printer printer(indent + 1);
		BOOST_FOREACH(Node const& node, group.nodes)
		{
			boost::apply_visitor(printer, node);
		}

		tab(indent); std::cout << "END" << std::endl;
	}

	void print(Groups const& groups)
	{
		group_printer printer;

		BOOST_FOREACH(Group const& group, groups)
		{
			printer(group);
		}
	}
};

BOOST_FUSION_ADAPT_STRUCT(
	test::Group,
	(std::string, name)
	(test::Nodes, nodes)
	)

BOOST_FUSION_ADAPT_STRUCT(
	test::Mod,
	(std::string, name)
	(test::Rules, rules)
	)

BOOST_FUSION_ADAPT_STRUCT(
	test::Rule,
	(test::Rule::Scope, scope)
	(test::Rule::Kind, kind)
	(std::string, text)
	)

// Real grammar
namespace test {

	struct rule_scope_ : qi::symbols<char, Rule::Scope>
	{
		rule_scope_()
		{
			add
				(">"    , Rule::Scope::Fcom)
				("<"	, Rule::Scope::NonFcom)
				("$"	, Rule::Scope::OOO)
				("^"	, Rule::Scope::BC)
				;
		}

	} rule_scope;

	struct rule_type_ : qi::symbols<char, Rule::Kind>
	{
		rule_type_()
		{
			add
				("?"    , Rule::Kind::Remark)
				("%"    , Rule::Kind::Tags)
				("*"    , Rule::Kind::Error)
				(":"    , Rule::Kind::Requirement)
				("\""   , Rule::Kind::Incompatibility)
				;
		}

	} rule_type;

	template <typename Iterator, typename Skipper>
	struct Grammar
		: qi::grammar<Iterator, Masterlist(), Skipper>
	{
		Grammar()
			: Grammar::base_type(masterlist, "masterlist")
		{
			using iso8859_1::char_;
			using namespace qi::labels;
			using qi::space;
			using qi::on_error;
			using qi::eps;
			using qi::fail;
			using qi::no_skip;
			using phoenix::construct;
			using phoenix::val;

			text
				%=	skip[eps] >> string;

			masterlist 
				%=  group % eol >> *eol
				;

			string
				=	no_skip[+(char_ - eol)]
				;

			mod %=	!endgroup 
				>>	!begingroup
				>>	text
				>>	-rules;

			rules
				%=	rule % eol
				;

			rule
				%=	*eol
				>>	-rule_scope
				>>	rule_type
				>>	text
				;

			node
				%=	*eol 
				>>	(
						group
					|	mod
					|	eoi
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
				=	lit("\\EndGroup\\\\")
				|	eoi;

			group
				%=	*eol
				>>	begingroup
				>>	nodes
				>	endgroup
				;

			masterlist.name("masterlist");
			string.name("string");
			text.name("text");
			mod.name("mod");
			rule.name("rule");
			rules.name("rules");
			group.name("group");
			node.name("node");
			nodes.name("nodes");
			begingroup.name("begin_group");
			endgroup.name("end_group");

			debug(masterlist);
			debug(group);
			debug(nodes);
			debug(node);
			debug(begingroup);
			debug(endgroup);
			debug(string);
			debug(text);
			debug(mod);
			debug(rules);
			debug(rule);

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

		qi::rule<Iterator, Masterlist(), Skipper> masterlist;
		qi::rule<Iterator, std::string(), Skipper> string;
		qi::rule<Iterator, std::string(), Skipper> text;
		qi::rule<Iterator, Mod(), Skipper> mod;
		qi::rule<Iterator, Rules(), Skipper> rules;
		qi::rule<Iterator, Rule(), Skipper> rule;
		qi::rule<Iterator, Node(), Skipper> node;
		qi::rule<Iterator, Nodes(), Skipper> nodes;
		qi::rule<Iterator, Group(), Skipper> group;
		qi::rule<Iterator, std::string(), Skipper> begingroup;
		qi::rule<Iterator, Skipper> endgroup;
	};
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

	test::Masterlist masterlist;

	std::string::const_iterator iter = storage.begin();
	std::string::const_iterator end = storage.end();
	bool r = phrase_parse(iter, end, grammar, skipper, masterlist);

	if (r && iter == end)
	{
		std::cout << "-------------------------\n";
		std::cout << "Parsing succeeded\n";
		std::cout << "-------------------------\n";
		test::print(masterlist);
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

