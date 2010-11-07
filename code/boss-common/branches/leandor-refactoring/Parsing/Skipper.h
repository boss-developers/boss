#ifndef __PARSING_HPP__
#define __PARSING_HPP__

#ifdef _MSC_VER
# pragma once
#endif

#include "Parsing.h"

namespace boss { namespace library { namespace parsing {

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
		} 

		qi::rule<Iterator> start;
		qi::rule<Iterator> spc;
		qi::rule<Iterator> string;
		qi::rule<Iterator> comment;
		qi::rule<Iterator> comments;
	};
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
		} 

		qi::rule<Iterator> start;
		qi::rule<Iterator> spc;
		qi::rule<Iterator> string;
		qi::rule<Iterator> comment;
		qi::rule<Iterator> comments;
	};

}}};

#endif
