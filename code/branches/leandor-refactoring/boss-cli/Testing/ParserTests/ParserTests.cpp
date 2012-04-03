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


	struct Masterlist;
	struct Group;
	struct Mod;
	struct Rule;


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
	
		struct Type
		{
			enum TypeValue
			{
				Unknown,
				Comment				=	'\\',
				Remark				=	'?',
				Tags				=	'%',
				Error				=	'*',	
				Requirement			=	':',
				Incompatibility		=	'"',
			};


			TypeValue value;


			Type() : value(Unknown) { };
			Type(const TypeValue value) : value(value) { };
			Type(const Type& type) : value(type.value) { };
		};
	
		Scope	scope;
		Type	type;
		string	text;
	};

	struct Mod
	{
		string name;
		vector<Rule> rules;
	};

	struct Group
	{
		string name;
		vector<Mod> mods;
	};

	struct Masterlist 
	{
		vector<Group> groups;
	};
}


namespace test {

	int const tabsize = 4;


	void tab(int indent)
	{
		for (int i = 0; i < indent * tabsize; ++i)
			std::cout << ' ';
	}


	struct masterlist_printer
	{
		masterlist_printer(int indent = 0) 
			: indent(indent)
		{
		}

		void operator()(Masterlist const& data) const;

		int indent;
	};

	struct rule_printer : public boost::static_visitor<>
	{
		rule_printer(int indent = 0) 
			: indent(indent)
		{
		}

		void operator()(Rule const& data) const
		{
			tab(indent);
			std::cout << " * RULE " << data.scope.value << ':' << data.type.value << " -> " << data.text << std::endl;
		}

		int indent;
	};

	struct mod_printer : public boost::static_visitor<>
	{
		mod_printer(int indent = 0) 
			: indent(indent)
		{
		}

		void operator()(Mod const& data) const
		{
			tab(indent);
			std::cout << "MOD: \"" << data.name << """ BEGIN: " << std::endl;

			rule_printer printer(indent + 1);
			for(vector<Rule>::const_iterator iter = data.rules.begin(); iter != data.rules.end(); iter++)
			{
				printer(*iter);
			}

			tab(indent);
			std::cout << "END MOD" << std::endl;
		}

		int indent;
	};

	struct group_printer : public boost::static_visitor<>
	{
		group_printer(int indent = 0) 
			: indent(indent)
		{
		}

		void operator()(Group const& data) const
		{
			tab(indent);
			std::cout << "GROUP \"" << data.name << """ BEGIN:" << std::endl;

			mod_printer printer(indent + 1);
			for(vector<Mod>::const_iterator iter = data.mods.begin(); iter != data.mods.end(); iter++)
			{
				printer(*iter);
			}

			tab(indent);
			std::cout << "END" << std::endl;
		}

		int indent;
	};

	void masterlist_printer::operator()(Masterlist const& data) const
	{
		tab(indent);
		std::cout << "BEGIN MASTERLIST" << std::endl;

		group_printer printer(indent + 1);
		for(vector<Group>::const_iterator iter = data.groups.begin(); iter != data.groups.end(); iter++)
		{
			printer(*iter);
		}

		tab(indent);
		std::cout << "END" << std::endl;
	}
};

BOOST_FUSION_ADAPT_STRUCT(
	test::Rule,
	(test::Rule::Scope, scope)
	(test::Rule::Type, type)
	(std::string, text)
	)

BOOST_FUSION_ADAPT_STRUCT(
	test::Mod,
	(std::string, name)
	(std::vector<test::Rule>, rules)
)

BOOST_FUSION_ADAPT_STRUCT(
	test::Group,
	(std::string, name)
	(std::vector<test::Mod>, mods)
)

BOOST_FUSION_ADAPT_STRUCT(
	test::Masterlist,
	(std::vector<test::Group>, groups)
)

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

	struct rule_type_ : qi::symbols<char, Rule::Type>
	{
		rule_type_()
		{
			add
				("?"    , Rule::Type::Remark)
				("%"    , Rule::Type::Tags)
				("*"    , Rule::Type::Error)
				(":"    , Rule::Type::Requirement)
				("\""   , Rule::Type::Incompatibility)
				;
		}

	} rule_type;


	template <typename Iterator>
	struct Skipper
		: qi::grammar<Iterator, void(), iso8859_1::space_type>
	{
		Skipper()
			: Skipper::base_type(start, "skipper")
		{
			using qi::eol;
			using iso8859_1::space;

			start %= space - eol;
		} 

		qi::rule<Iterator, void(), iso8859_1::space_type> start;
	};

	template <typename Iterator, typename Skipper>
	struct Grammar
		: qi::grammar<Iterator, Masterlist(), Skipper>
	{
		Grammar()
			: Grammar::base_type(masterlist, "masterlist")
		{
			using iso8859_1::char_;
			using iso8859_1::string;
			using namespace qi::labels;
			using qi::on_error;
			using qi::fail;
			using phoenix::construct;
			using phoenix::val;
			using boost::spirit::no_skip;

			text
				%=	lexeme[+char_] > eol;

			mod
				%=	comments
				>>	text >	eol
				>>	-rules
				;

			mods
				%=	mod % eol
				;

			rule
				%=	comments
				>>	-rule_scope
				>>	rule_type
				>>	text
				;

			rules		
				%=	rule % eol
				;

			group		
				%=	comments
				>	begin_group > eol
				>>	mods
				>	end_group	> eol
				;

			groups		
				%=	group % eol
				;

			masterlist 
				%=  comments
				>>	groups
				>>  comments
				;

			comment	
				%= lit("\\") 
				>> !lit("BeginGroup")
				>> !lit("EndGroup")
				>> omit[text]
				;

			comments 
				%=	comment % eol
				;


			begin_group	
				%=	lit('\\')
				> 	lit("BeginGroup")
				> 	lit('\\')
				> 	lit(':')
				>>	text
				;

			end_group	
				%=	lit('\\')
				> 	lit("EndGroup")
				>>	+lit('\\')
				> 	lit(':')
				;


			masterlist.name("MASTERLIST");
			text.name("TEXT");
			comments.name("COMMENTS");
			comment.name("COMMENT");
			mods.name("MODS");
			mod.name("MOD");
			rules.name("RULES");
			rule.name("RULE");
			groups.name("GROUPS");
			end_group.name("GROUP END");
			begin_group.name("GROUP START");
			group.name("GROUP");


			on_error<fail>(masterlist, 
				std::cout
				<< val("Error! Expecting ")
				<< _4 // what failed?
				<< val(" here: \"")
				<< construct<std::string>(_3, _2) // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);

			on_error<fail>(rule, 
				std::cout
				<< val("Error! Expecting ")
				<< _4 // what failed?
				<< val(" here: \"")
				<< construct<std::string>(_3, _2) // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);

			on_error<fail>(rules, 
				std::cout
				<< val("Error! Expecting ")
				<< _4 // what failed?
				<< val(" here: \"")
				<< construct<std::string>(_3, _2) // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);

			on_error<fail>(mod, 
				std::cout
				<< val("Error! Expecting ")
				<< _4 // what failed?
				<< val(" here: \"")
				<< construct<std::string>(_3, _2) // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);

			on_error<fail>(mods, 
				std::cout
				<< val("Error! Expecting ")
				<< _4 // what failed?
				<< val(" here: \"")
				<< construct<std::string>(_3, _2) // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);

			on_error<fail>(group, 
				std::cout
				<< val("Error! Expecting ")
				<< _4 // what failed?
				<< val(" here: \"")
				<< construct<std::string>(_3, _2) // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);

			on_error<fail>(groups, 
				std::cout
				<< val("Error! Expecting ")
				<< _4 // what failed?
				<< val(" here: \"")
				<< construct<std::string>(_3, _2) // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);

			on_error<fail>(begin_group, 
				std::cout
				<< val("Error! Expecting ")
				<< _4 // what failed?
				<< val(" here: \"")
				<< construct<std::string>(_3, _2) // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);

			on_error<fail>(end_group, 
				std::cout
				<< val("Error! Expecting ")
				<< _4 // what failed?
				<< val(" here: \"")
				<< construct<std::string>(_3, _2) // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);

			on_error<fail>(comment, 
				std::cout
				<< val("Error! Expecting ")
				<< _4 // what failed?
				<< val(" here: \"")
				<< construct<std::string>(_3, _2) // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);

			on_error<fail>(comments, 
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
		qi::rule<Iterator, Rule()> rule;
		qi::rule<Iterator, std::vector<Rule>()> rules;
		qi::rule<Iterator, Mod()> mod;
		qi::rule<Iterator, std::vector<Mod>()> mods;
		qi::rule<Iterator, Group()> group;
		qi::rule<Iterator, std::vector<Group>()> groups;
		qi::rule<Iterator, std::string()> text;
		qi::rule<Iterator, void()> comment;
		qi::rule<Iterator, void()> comments;
		qi::rule<Iterator, std::string()> begin_group;
		qi::rule<Iterator, void()> end_group;
	};
};


namespace test {

};

int main(int argc, char* argv[])
{
	//namespace fusion = boost::fusion;
	//namespace phoenix = boost::phoenix;
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
	typedef test::Grammar<std::string::const_iterator, qi::space_type> Grammar;

	Grammar grammar; // Our grammar
	Skipper skipper; // Our grammar
	test::Masterlist data; // Our tree

	std::string::const_iterator iter = storage.begin();
	std::string::const_iterator end = storage.end();
	bool r = phrase_parse(iter, end, grammar, qi::space, data);

	if (r && iter == end)
	{
		std::cout << "-------------------------\n";
		std::cout << "Parsing succeeded\n";
		std::cout << "-------------------------\n";
		test::masterlist_printer printer;
		printer(data);
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

