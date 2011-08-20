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

#include "Common/Lists.h"
#include <string>
#include <vector>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

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

	extern boost::unordered_set<string> setVars;  //Vars set by masterlist. Also referenced by userlist parser.
	extern boost::unordered_map<string,unsigned int> fileCRCs;  //CRCs calculated. Referenced by modlist and userlist parsers.

	struct masterlistMsgKey_ : qi::symbols<char, keyType> {
		masterlistMsgKey_();
	} extern masterlistMsgKey;

	struct typeKey_ : qi::symbols<char, itemType> {
		typeKey_();
	} extern typeKey;

	struct metaKey_ : qi::symbols<char, metaType> {
		metaKey_();
	} extern metaKey;
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
		ruleKeys_();
	} extern ruleKeys;

	struct messageKeys_ : qi::symbols<char, keyType> {
		messageKeys_();
	} extern sortOrMessageKeys;
}
#endif