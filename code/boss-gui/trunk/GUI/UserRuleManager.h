/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __RULEMANAGER__HPP__
#define __RULEMANAGER__HPP__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include <wx/hyperlink.h>
#include <wx/progdlg.h>

class UserRulesManagerFrame : public wxFrame {
public:
	UserRulesManagerFrame(const wxChar *title, wxFrame *parent, int x, int y, int width, int height);
private:

};

#endif