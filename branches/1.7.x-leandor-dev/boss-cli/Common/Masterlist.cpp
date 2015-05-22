/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2002 $, $Date: 2010-12-05 21:35:32 +0000 (Sun, 05 Dec 2010) $
*/


#include "Support/Helpers.h"
#include "Masterlist.h"


namespace boss {

	bool IsMod(wstring textbuf) {
		return (((textbuf[0]!='\\') && (textbuf[0]!='*') && (textbuf[0]!='\?') && (textbuf[0]!='%') && (textbuf[0]!=':') && (textbuf[0]!='$') &&(textbuf[0]!='^') && (textbuf[0]!='"')));
	}
	
	bool IsMessage(wstring textbuf) {
		return (((textbuf[0]=='\?') || (textbuf[0]=='*') || (textbuf[0]=='%') || (textbuf[0]==':') || (textbuf[0]=='$') || (textbuf[0]=='^') || (textbuf[0]=='"')));
	}
	
	bool IsValidLine(wstring textbuf) {
		return ((textbuf.length()>1) && (Tidy(textbuf)!=L"oblivion.esm") && (Tidy(textbuf)!=L"fallout3.esm") && (Tidy(textbuf)!=L"nehrim.esm") && (Tidy(textbuf)!=L"falloutnv.esm"));
	}
}
