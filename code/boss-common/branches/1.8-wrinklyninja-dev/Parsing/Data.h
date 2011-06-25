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

	struct masterlistMsgKey_ : qi::symbols<char, keyType> {
		masterlistMsgKey_() {
			add
				//New Message keywords.
				("SAY",SAY)
				("TAG",TAG)
				("REQ",SPECIFIC_REQ)
				("INC", SPECIFIC_INC)
				("DIRTY",DIRTY)
				("WARN",WARN)
				("ERROR",ERR)
				//Old message symbols.
				("?",SAY)
				("$",OOOSAY)  //OOO comment
				("^",BCSAY)  //BC comment
				("%",TAG)
				(":",REQ)
				("\"",INC) //Incompatibility
				("*",ERR) //FCOM install error.
			;
		}
	} masterlistMsgKey;

	struct typeKey_ : qi::symbols<char, itemType> {
		typeKey_() {
			add
				//Group keywords.
				("BEGINGROUP:",BEGINGROUP)  //Needs the colon there unfortunately.
				("ENDGROUP:",ENDGROUP)  //Needs the colon there unfortunately.
				("ENDGROUP",ENDGROUP)
				("\\BeginGroup\\:",BEGINGROUP)
				("\\EndGroup\\\\",ENDGROUP)
				("MOD:", MOD)  //Needs the colon there unfortunately.
				("REGEX:", REGEX)
			;
		}
	} typeKey;

	struct metaKey_ : qi::symbols<char, metaType> {
		metaKey_() {
			add
				//Condition keywords.
				("IF", IF)
				("IFNOT", IFNOT)
			;
		}
	} metaKey;
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
				("add",ADD)
				("override",OVERRIDE)
				("for",FOR)
			;
		}
	} ruleKeys;

	struct messageKeys_ : qi::symbols<char, keyType> {
		messageKeys_() {
			add
				("append",APPEND)
				("replace",REPLACE)
				("before",BEFORE)
				("after",AFTER)
				("top",TOP)
				("bottom",BOTTOM)
			;
		}
	} sortOrMessageKeys;
}
#endif