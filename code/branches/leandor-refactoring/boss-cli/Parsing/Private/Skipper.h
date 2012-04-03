#pragma once

#include "Private/Defines.h"

namespace boss { namespace parsing { namespace detail {


	template <typename Iterator>
	struct Skipper
		: qi::grammar<Iterator>
	{
		Skipper()
			: Skipper::base_type(start, "skipper")
		{
			using qi::lit;
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
				=	lit("//") 
				>> -string
				;

			comments
				=	comment % eol
				;

			string
				=	+(char_ - eol)
				;

			#ifdef DEBUG_SKIPPER_GRAMMAR

			BOOST_SPIRIT_DEBUG_NODE(start);
			BOOST_SPIRIT_DEBUG_NODE(spc);
			BOOST_SPIRIT_DEBUG_NODE(string);
			BOOST_SPIRIT_DEBUG_NODE(comment);
			BOOST_SPIRIT_DEBUG_NODE(comments);
			
			#endif
		} 

		qi::rule<Iterator> start;
		qi::rule<Iterator> spc;
		qi::rule<Iterator> string;
		qi::rule<Iterator> comment;
		qi::rule<Iterator> comments;
	};

}}}
