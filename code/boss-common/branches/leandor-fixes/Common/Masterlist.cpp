/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#ifdef WIN32
#include <Support/Helpers.h>
#else
#include "../Support/Helpers.h"
#endif

#include "Masterlist.h"

namespace boss {

	bool IsMod(string textbuf) {
		return (((textbuf[0]!='\\') && (textbuf[0]!='*') && (textbuf[0]!='\?') && (textbuf[0]!='%') && (textbuf[0]!=':') && (textbuf[0]!='$') &&(textbuf[0]!='^') && (textbuf[0]!='"')));
	}
	
	bool IsMessage(string textbuf) {
		return (((textbuf[0]=='\?') || (textbuf[0]=='*') || (textbuf[0]=='%') || (textbuf[0]==':') || (textbuf[0]=='$') || (textbuf[0]=='^') || (textbuf[0]=='"')));
	}
	
	bool IsValidLine(string textbuf) {
		return ((textbuf.length()>1) && (Tidy(textbuf)!="oblivion.esm") && (Tidy(textbuf)!="fallout3.esm") && (Tidy(textbuf)!="nehrim.esm"));
	}
}
