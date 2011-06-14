/*	Dirty Mod List Generator

    Outputs a list of dirty mods and their ITM/UDR counts and CRCs,
	using information from the Better Oblivion Sorting Software masterlist.

	Written using code adapted from Better Oblivion Sorting Software.
	All code (new and adapted), authored by WrinklyNinja.

    Copyright (C) 2011  WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2512 $, $Date: 2011-04-01 10:38:36 +0100 (Fri, 01 Apr 2011) $
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
				("REQ",REQ)
				("INC", INC)
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
#endif