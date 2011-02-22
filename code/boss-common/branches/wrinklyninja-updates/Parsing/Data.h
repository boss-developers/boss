/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains conversions for list data structures and data types so that the parser can access them.

#ifndef BOOST_FILESYSTEM_VERSION
#define BOOST_FILESYSTEM_VERSION 3
#endif

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif

#include <string>
#include <vector>
#include "boost/filesystem.hpp"
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi.hpp>
#include "../Common/Lists.h"

namespace fs = boost::filesystem;
namespace qi = boost::spirit::qi;

//////////////////////////////
//Userlist Data Conversions
//////////////////////////////

/////////////////////////////////////////////
//BOOST Fusion conversions of above structs
/////////////////////////////////////////////

//Userlist structures.
BOOST_FUSION_ADAPT_STRUCT(
	boss::line,
	(boss::keyType, key)
	(std::string, object)
)

BOOST_FUSION_ADAPT_STRUCT(
	boss::rule,
	(boss::keyType, ruleKey)
	(std::string, ruleObject)
	(std::vector<boss::line>, lines)
)

//////////////////////////////////////////////////
//BOOST Spirit symbol conversion of keyType enum
//////////////////////////////////////////////////

struct ruleKeys_ : qi::symbols<char, boss::keyType> {
	ruleKeys_() {
		add
			//Userlist keywords.
			("add",boss::ADD)
			("override",boss::OVERRIDE)
			("for",boss::FOR)
		;
	}
} ruleKeys;

struct lineKeys_ : qi::symbols<char, boss::keyType> {
	lineKeys_() {
		add
			//Userlist keywords.
			("before",boss::BEFORE)
			("after",boss::AFTER)
			("top",boss::TOP)
			("bottom",boss::BOTTOM)
			("append",boss::APPEND)
			("replace",boss::REPLACE)
		;
	}
} lineKeys;