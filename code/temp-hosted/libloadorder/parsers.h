/*	liblo

	A library for reading and writing the load order of plugin files for
	TES IV: Oblivion, Nehrim - At Fate's Edge, TES V: Skyrim, Fallout 3 and
	Fallout: New Vegas.

    Copyright (C) 2009-2012    WrinklyNinja

	This file is part of liblo.

    liblo is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    liblo is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with liblo.  If not, see 
	<http://www.gnu.org/licenses/>.
*/

#ifndef LIBLO_PARSERS_H
#define LIBLO_PARSERS_H

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include "plugins.h"
#include <utility>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/unordered_map.hpp>

//////////////////////////////
// Data conversions
//////////////////////////////

BOOST_FUSION_ADAPT_STRUCT(
    liblo::Plugin,
    (std::string, name)
)

namespace liblo {

	typedef boost::spirit::istream_iterator grammarIter;

	//////////////////////////////
	// Skipper grammar
	//////////////////////////////

	class pluginlist_skipper : public boost::spirit::qi::grammar<grammarIter> {
		pluginlist_skipper();
	private:
		boost::spirit::qi::rule<grammarIter> start, spc, eof, comment;
	};

	//////////////////////////////
	// PluginList grammar
	//////////////////////////////

	class pluginlist_grammar : public boost::spirit::qi::grammar<grammarIter, std::vector<Plugin>(), pluginlist_skipper> {
	public:
		pluginlist_grammar();
	private:
		boost::spirit::qi::rule<grammarIter, std::vector<Plugin>(), pluginlist_skipper> pluginList;
		boost::spirit::qi::rule<grammarIter, Plugin()> plugin;
		boost::spirit::qi::rule<grammarIter, std::string()> charString;
	
		void SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, boost::spirit::info const& what);
	};
}
#endif