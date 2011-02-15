/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains the various functions/classes required for varied BOSSlog output formattings,etc.

#ifndef __BOSS_BOSSLOG_H__
#define __BOSS_BOSSLOG_H__

#include <fstream>
#include <string>

namespace boss {
	using namespace std;

	//Now a dumping ground for un-sorted testing code.
	//Try implementing a BOSSLogger function or something.
	enum attr {
		NO_ATTR = 0,
		BR = 1,
		START_DIV = 2,
		START_PARA = 3,
		END_DIV = 4,
		END_PARA = 5,
		END_LOG = 6
	};

	enum styleType {
		NO_STYLE = 0,
		TITLE = 1,
		LINK = 2,
		ERR = 3,
		SUCCESS = 4,
		WARN = 5,
		GHOST = 6,
		VER = 7,
		TAG = 8,
		SUBTITLE = 9,
		LI = 10
	};

	enum logFormat {
		HTML = 0,
		PLAINTEXT = 1
	};

	void printHTMLHead(ofstream& log);

	void printLogText(ofstream& log, string text, logFormat format, attr attribute, styleType style);
	void printLogText(ofstream& log, string text, string link, logFormat format, attr attribute, styleType style);
}
#endif