/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains conversions for list data structures and data types so that the parser can access them.

#include "Parsing\Data.h"

/////////////////////////////////////////
// Modlist/masterlist data conversions
/////////////////////////////////////////

namespace boss {
	namespace qi = boost::spirit::qi;

	boost::unordered_set<string> setVars;  //Vars set by masterlist. Also referenced by userlist parser.
	boost::unordered_map<string,unsigned int> fileCRCs;  //CRCs calculated. Referenced by modlist and userlist parsers.
	masterlistMsgKey_ masterlistMsgKey;
	typeKey_ typeKey;
	metaKey_ metaKey;
	ruleKeys_ ruleKeys;
	messageKeys_ sortOrMessageKeys;

	masterlistMsgKey_::masterlistMsgKey_() {
		add
			//New Message keywords.
			("say",SAY)
			("tag",TAG)
			("req",SPECIFIC_REQ)
			("inc", SPECIFIC_INC)
			("dirty",DIRTY)
			("warn",WARN)
			("error",ERR)
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

	typeKey_::typeKey_() {
		add
			//Group keywords.
			("begingroup:",BEGINGROUP)  //Needs the colon there unfortunately.
			("endgroup:",ENDGROUP)  //Needs the colon there unfortunately.
			("endgroup",ENDGROUP)
			("\\BeginGroup\\:",BEGINGROUP)
			("\\EndGroup\\\\",ENDGROUP)
			("mod:", MOD)  //Needs the colon there unfortunately.
			("regex:", REGEX)
		;
	}

	metaKey_::metaKey_() {
		add
			//Condition keywords.
			("if", IF)
			("ifnot", IFNOT)
		;
	}

//////////////////////////////
//Userlist Data Conversions
//////////////////////////////


	ruleKeys_::ruleKeys_() {
		add
			("add",ADD)
			("override",OVERRIDE)
			("for",FOR)
		;
	}

	messageKeys_::messageKeys_() {
		add
			("append",APPEND)
			("replace",REPLACE)
			("before",BEFORE)
			("after",AFTER)
			("top",TOP)
			("bottom",BOTTOM)
		;
	}
}