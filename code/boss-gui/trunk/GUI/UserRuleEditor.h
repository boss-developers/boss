/*	Better Oblivion Sorting Software

	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011  WrinklyNinja & the BOSS development team. 
	Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __RULEEDITOR__HPP__
#define __RULEEDITOR__HPP__

#include "BOSS-Common.h"

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include <wx/rearrangectrl.h>
#include <wx/srchctrl.h>
#include <wx/treectrl.h>

using namespace boss;
using namespace std;

/* Editor window handles all the modlist/masterlist stuff and rule creator/editor stuff itself.

It treats the rule list it contains as a black box except from the following:
1. When a rule is selected in the rule list, it triggers an event that can be detected in the editor window. 
   The event can then be used to access the Rule object for the currently selected rule, which is passed to the Editor window and used to fill in the editor form.
2. When the editor form's Save or Create or Delete buttons are pressed, related functions in the rule list are called, with the edited Rule, new Rule or nothing passed as parameters respectively.
   The rule list functions then perform the necessary changes.

The rule list is composed of the actual list (which is scrollable and made of items of variable height), and two "move item" buttons, which move the currently selected item up or down.
All changes enacted by the GUI are performed on the RuleList object, from which the resulting changes to the GUI representation of the rules are then drawn.
No original information is stored in GUI structures themselves.


*/

class RuleBoxClass : public wxFrame {
public:
	RuleBoxClass();
	RuleBoxClass(Rule currentRule);
	void ToggleEnabled(bool isEnabled);		//Doesn't handle RuleList modification, only greying out of UI element.
private:
	wxBoxSizer *checkboxSizer;
	wxBoxSizer *contentSizer;
	wxStaticText *ruleContent;
	wxCheckBox *ruleCheckbox;
};

class RuleListFrameClass : public wxFrame {
public:
	RuleListFrameClass(ItemList &masterlist);		//Initialise the RuleListFrameClass object.
	void SaveUserlist(const fs::path path);					//Save the changes made to the userlist.
	
	Rule GetSelectedRule();								//Returns the currently selected rule.
	
	void AppendRule(Rule newRule);			//Append to RuleList object and update GUI.
	void SaveEditedRule(Rule editedRule);   //Get the index from current selection internally. Also update RuleList object.
	void DeleteSelectedRule();				//Remove from GUI and RuleList object, getting index from current selection internally.

	void OnRuleOrderChange(wxCommandEvent& event);
	void OnToggleRule(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
private:

	void ReDrawRuleList();								//Empties the RuleListScroller and then re-populates it with RuleBoxClass objects for the rules in the RuleList object.

	RuleList userlist;

	wxButton *MoveRuleUp;
	wxButton *MoveRuleDown;
	wxScrolled<wxPanel> *RuleListScroller;
};

class UserRulesEditorFrame : public wxFrame {
public:
	UserRulesEditorFrame(const wxChar *title, wxFrame *parent);
	void OnOKQuit(wxCommandEvent& event);
	void OnCancelQuit(wxCommandEvent& event);
	void OnSearchList(wxCommandEvent& event);
	void OnCancelSearch(wxCommandEvent& event);
	void OnSelectModInMasterlist(wxCommandEvent& event);
	void OnSortingCheckToggle(wxCommandEvent& event);
	void OnMessageAddToggle(wxCommandEvent& event);
	void OnSortInsertChange(wxCommandEvent& event);

	void OnRuleCreate(wxCommandEvent& event);
	void OnRuleEdit(wxCommandEvent& event);
	void OnRuleDelete(wxCommandEvent& event);

	void OnRuleSelection(wxCommandEvent& event);
	
	DECLARE_EVENT_TABLE()
private:
	void LoadLists();
	Rule GetRuleFromForm();

	ItemList masterlist;

	wxArrayString ModlistMods;

	wxButton *NewRuleButton;
	wxButton *EditRuleButton;
	wxButton *DeleteRuleButton;
	wxButton *AddModlistModAsRuleMod;
	wxButton *AddModlistModAsSortMod;
	wxButton *AddMasterlistModAsRuleMod;
	wxButton *AddMasterlistModAsSortMod;
	wxRadioButton *SortModOption;
	wxRadioButton *InsertModOption;
	wxCheckBox *AddMessagesCheckBox;
	wxCheckBox *ReplaceMessagesCheckBox;
	wxTreeCtrl *InstalledModsList;
	wxTreeCtrl *MasterlistModsList;
	wxSearchCtrl *ModlistSearch;
	wxSearchCtrl *MasterlistSearch;
	wxTextCtrl *ModMessagesBox;
	wxChoice *BeforeAfterChoiceBox;
	wxChoice *TopBottomChoiceBox;
	wxTextCtrl *NewModMessagesBox;
	RuleListFrameClass *RulesList;
	wxTextCtrl *RuleModBox;
	wxTextCtrl *SortModBox;
	wxCheckBox *SortModsCheckBox;
	wxTextCtrl *InsertModBox;
};

#endif