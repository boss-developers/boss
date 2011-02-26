/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains conversions for list data structures and data types so that the parser can access them.

#ifndef __BOSS_DATA_H__
#define __BOSS_DATA_H__

#include <string>
#include <vector>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi.hpp>
#include "Common/Lists.h"
#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;

/////////////////////////////////////////
// Modlist/masterlist data conversions
/////////////////////////////////////////

BOOST_FUSION_ADAPT_STRUCT(
    boss::message,
    (boss::keyType, key)
    (std::string, data)
)

BOOST_FUSION_ADAPT_STRUCT(
    boss::item,
	(boss::itemType, type)
    (fs::path, name)
    (std::vector<boss::message>, messages)
)

namespace boss {
	namespace qi = boost::spirit::qi;

	struct messageKey_ : qi::symbols<char, boss::keyType> {
		messageKey_() {
			add
				//Message keywords.
				("SAY",boss::SAY)
				("TAG",boss::TAG)
				("REQ",boss::REQ)
				("WARN",boss::WARN)
				("ERROR",boss::ERR)
			;
		}
	} messageKey;

	//There must be a better way to do this...
	struct groupKey_ : boost::spirit::qi::symbols<char, boss::itemType> {
		groupKey_() {
			add
				//Group keywords.
				("BEGINGROUP",boss::BEGINGROUP)
				("ENDGROUP",boss::ENDGROUP)
			;
		}
	} groupKey;

	struct condKey_ : boost::spirit::qi::symbols<char, boss::keyType> {
		condKey_() {
			add
				("IF",boss::IF)
				("IFNOT",boss::IFNOT)
				;
		}
	} condKey;

	struct condOp_ : boost::spirit::qi::symbols<char, boss::keyType> {
		condOp_() {
			add
				("||",boss::OR)
				("&&",boss::AND)
				;
		}
	} condOp;

	struct varKey_ : boost::spirit::qi::symbols<char, boss::keyType> {
		varKey_() {
			add
				("SET",boss::OR)
				;
		}
	} varKey;
}

//////////////////////////////
//Userlist Data Conversions
//////////////////////////////

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

namespace boss {
	namespace qi = boost::spirit::qi;

	struct ruleKeys_ : qi::symbols<char, keyType> {
		ruleKeys_() {
			add
				//Userlist keywords.
				("add",ADD)
				("override",OVERRIDE)
				("for",FOR)
			;
		}
	} ruleKeys;

	struct lineKeys_ : qi::symbols<char, keyType> {
		lineKeys_() {
			add
				//Userlist keywords.
				("before",BEFORE)
				("after",AFTER)
				("top",TOP)
				("bottom",BOTTOM)
				("append",APPEND)
				("replace",REPLACE)
			;
		}
	} lineKeys;
}
#endif