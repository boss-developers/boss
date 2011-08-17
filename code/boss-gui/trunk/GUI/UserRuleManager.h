/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __RULEMANAGER__HPP__
#define __RULEMANAGER__HPP__

#include "wx/wxprec.h"
#include <vector>
#include <string>

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

using namespace std;

enum keyType {
	NONE,
	//Userlist keywords.
	ADD,
	OVERRIDE,
	FOR,
	BEFORE,
	AFTER,
	TOP,
	BOTTOM,
	APPEND,
	REPLACE
};

extern vector<string> userlistErrorBuffer;  //Holds any error messages generated during parsing for printing later.

	////////////////////////////////////////
	// Userlist data structures
	////////////////////////////////////////

	//Userlist data structure.
	struct line {
		keyType key;
		string object;
	};

	struct rule {
		keyType ruleKey;
		string ruleObject;
		vector<line> lines;
	};

	vector<rule> userlist;

	////////////////////////////////////////
	// GUI Stuff
	////////////////////////////////////////

class UserRulesManagerFrame : public wxFrame {
public:
	UserRulesManagerFrame(const wxChar *title, wxFrame *parent, int x, int y, int width, int height);
private:
	wxButton *NewRuleButton;
	wxButton *EditRuleButton;
	wxButton *DeleteRuleButton;
	wxRadioButton *SortModOption;
	wxRadioButton *InsertModOption;
	wxCheckBox *AddMessageCheckBox;
	wxCheckBox *ReplaceMessagesCheckBox;
	wxListBox *InstalledModsList;
	wxListBox *MasterlistModsList;
	wxStaticText *ModMessagesBox;
	wxComboBox *BeforeAfterChoiceBox;
	wxComboBox *TopBottomChoiceBox;
	wxTextCtrl *NewModMessagesBox;
};

#endif