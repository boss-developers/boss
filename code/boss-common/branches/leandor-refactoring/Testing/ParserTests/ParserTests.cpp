#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
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
	namespace ascii = boost::spirit::ascii;

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
			enum
			{
				Always,
				Fcom				=	'>',
				NonFcom				=	'<',
				OOO					=	'$',
				BC					=	'^',
			}  value;

		};
	
		struct Type
		{
			enum
			{
				Comment				=	'\\',
				Remark				=	'?',
				Tags				=	'%',
				Error				=	'*',	
				Requirement			=	':',
				Incompatibility		=	'"',
			} value;

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
			std::cout << " * RULE " << data.scope << ':' << data.type << " -> " << data.text << std::endl;
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
	struct Grammar
		: qi::grammar<Iterator, Masterlist(), ascii::space_type>
	{
		Grammar()
			: Grammar::base_type(masterlist, "masterlist")
		{
			using ascii::char_;
			using ascii::string;
			using namespace qi::labels;

			text
				%=	lexeme[+char_ - eol];

			mod
				%=	- comments
				>>	text 
				>>	eol
				>>	rules
				;

			mods
				%=	mod 
				%	eol
				;

			rule
				%=	- comments
				>>	(rule_scope | lit(Rule::Always))
				>>	rule_type
				>>	text
				;

			rules		
				%=	rule 
				%	eol
				;

			group		
				%=	- comments
				>>	begin_group
				>>	mods
				>>	end_group
				;

			groups		
				%=	group 
				%	eol
				;

			masterlist 
				%= - comments
				>>	groups
				>> - comments
				>> eoi
				;

			comment	
				%= lit("\\") 
				>> !lit("BeginGroup")
				>> !lit("EndGroup")
				>> omit[text]
				;

			comments 
				%=	*comment
				>>	-eol
				;


			begin_group	
				%=	lit('\\')
				>> 	lit("BeginGroup")
				>> 	lit('\\')
				>> 	lit(':')
				>>	text
				>>	eol
				;

			end_group	
				%=	lit('\\')
				>> 	lit("EndGroup")
				>>	+lit('\\')
				>> 	lit(':')
				>>	eol
				;

		}

		qi::rule<Iterator, Masterlist(), ascii::space_type> masterlist;
		qi::rule<Iterator, Rule(), ascii::space_type> rule;
		qi::rule<Iterator, std::vector<Rule>(), ascii::space_type> rules;
		qi::rule<Iterator, Mod(), ascii::space_type> mod;
		qi::rule<Iterator, std::vector<Mod>(), ascii::space_type> mods;
		qi::rule<Iterator, Group(), ascii::space_type> group;
		qi::rule<Iterator, std::vector<Group>(), ascii::space_type> groups;
		qi::rule<Iterator, std::string(), ascii::space_type> text;
		qi::rule<Iterator, void(), ascii::space_type> comment;
		qi::rule<Iterator, void(), ascii::space_type> comments;
		qi::rule<Iterator, std::string(), ascii::space_type> begin_group;
		qi::rule<Iterator, void(), ascii::space_type> end_group;
	};
};


namespace test {

};

int main(int argc, char* argv[])
{

	//namespace fusion = boost::fusion;
	//namespace phoenix = boost::phoenix;
	namespace qi = boost::spirit::qi;
	//namespace ascii = boost::spirit::ascii;

	using qi::eol;

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

	typedef test::Grammar<std::string::const_iterator> Grammar;
	Grammar grammar; // Our grammar
	test::Masterlist data; // Our tree

	using boost::spirit::ascii::space;
	std::string::const_iterator iter = storage.begin();
	std::string::const_iterator end = storage.end();
	bool r = phrase_parse(iter, end, grammar, space - eol, data);

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

