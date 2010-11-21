#pragma once

//#define	BOOST_SPIRIT_DEBUG
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
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace boss { namespace parsing { 
	
	namespace detail {

		namespace spirit	= boost::spirit;
		namespace qi		= boost::spirit::qi;
		namespace fusion	= boost::fusion;
		namespace phoenix	= boost::phoenix;
		namespace iso8859_1 = boost::spirit::iso8859_1;

		using fusion::unused_type;
	}

	namespace qi		= boost::spirit::qi;

}}

