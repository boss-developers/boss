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

#include "parsers.h"
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

namespace liblo {

	namespace qi = boost::spirit::qi;
	namespace unicode = boost::spirit::unicode;
	namespace phoenix = boost::phoenix;

	using namespace qi::labels;

	using qi::skip;
	using qi::eol;
	using qi::eoi;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;
	using qi::lit;
	using qi::omit;
	using qi::eps;
	using qi::hex;
	using qi::bool_;
	using qi::uint_;
	
	using unicode::char_;
	using unicode::no_case;
	using unicode::space;
	using unicode::xdigit;

	///////////////////////////////
	//Skipper Grammars
	///////////////////////////////

	ini_skipper::ini_skipper() : ini_skipper::base_type(start, "ini skipper grammar") {
		start =
			spc
			| comment
			| eof;

		spc = space - eol;

		comment = ';' >> *(char_ - eol);

		eof = *(spc | comment | eol) >> eoi;
	}

	pluginlist_skipper::pluginlist_skipper() : pluginlist_skipper::base_type(start, "ini skipper grammar") {
		start =
			spc
			| comment
			| eof;

		spc = space - eol;

		comment = '#' >> *(char_ - eol);

		eof = *(spc | comment | eol) >> eoi;
	}


	//////////////////////////////
	// PluginList grammar
	//////////////////////////////


	pluginlist_grammar::pluginlist_grammar() : pluginlist_grammar::base_type(pluginList, "pluginlist grammar") {

		pluginList %= *eol > plugin % +eol;

		plugin %= charString;

		charString %= +(char_ - eol);

		pluginList.name("pluginList");
		plugin.name("plugin");
		charString.name("charString");

		qi::on_error<fail>(pluginList,	phoenix::bind(&pluginlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		qi::on_error<fail>(plugin,		phoenix::bind(&pluginlist_grammar::SyntaxError, this, _1, _2, _3, _4));
		qi::on_error<fail>(charString,	phoenix::bind(&pluginlist_grammar::SyntaxError, this, _1, _2, _3, _4));
	}
	
	void pluginlist_grammar::SyntaxError(grammarIter const& /*first*/, grammarIter const& last, grammarIter const& errorpos, boost::spirit::info const& what) {

	}
}