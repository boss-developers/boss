#ifndef __PARSING_HPP__

#ifdef _MSC_VER
# pragma once
#endif

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>

namespace boss { namespace library { namespace parsing {


	namespace fusion	= boost::fusion;
	namespace phoenix	= boost::phoenix;
	namespace qi		= boost::spirit::qi;
	namespace iso8859_1 = boost::spirit::iso8859_1;


}}};

#endif
