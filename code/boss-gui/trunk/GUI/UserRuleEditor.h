/*	Better Oblivion Sorting Software

	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011 BOSS Development Team. Copyright license:
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
#include <wx/dataobj.h>
#include <wx/dnd.h>

using namespace boss;
using namespace std;

class TextDropTarget : public wxTextDropTarget {  //Class to override virtual functions.
public:
	TextDropTarget(wxTextCtrl *owner);
	virtual bool OnDropText(wxCoord x, wxCoord y, const wxString &data);
private:
	wxTextCtrl *targetOwner;
};

class RuleBoxClass : public wxPanel {
public:
	RuleBoxClass(wxScrolled<wxPanel> *parent, Rule currentRule, unsigned int index, bool isSelected);
	void ToggleEnabled(wxCommandEvent& event);		//Doesn't handle RuleList modification, only greying out of UI element.
	void OnSelect(wxMouseEvent& event);
	void Highlight(bool highlight);
	DECLARE_EVENT_TABLE()
private:
	wxStaticText *ruleContent;
	wxCheckBox *ruleCheckbox;
	unsigned int ruleIndex;
};

class RuleListFrameClass : public wxPanel {
public:
	RuleListFrameClass(wxFrame *parent, wxWindowID id, ItemList &masterlist);		//Initialise the RuleListFrameClass object.
	void SaveUserlist(const fs::path path);	//Save the changes made to the userlist.
	Rule GetSelectedRule();					//Returns the currently selected rule.
	void AppendRule(Rule newRule);			//Append to RuleList object and update GUI.
	void SaveEditedRule(Rule editedRule);   //Get the index from current selection internally. Also update RuleList object.
	void DeleteSelectedRule();				//Remove from GUI and RuleList object, getting index from current selection internally.
	void MoveRule(wxWindowID id);
	void OnToggleRule(wxCommandEvent& event);
	void OnRuleSelection(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
private:
	void ReDrawRuleList();					//Empties the RuleListScroller and then re-populates it with RuleBoxClass objects for the rules in the RuleList object.
	RuleList userlist;
	unsigned int selectedRuleIndex;
	wxScrolled<wxPanel> *RuleListScroller;
};

class UserRulesEditorFrame : public wxFrame {
public:
	UserRulesEditorFrame(const wxChar *title, wxFrame *parent);
	void OnOKQuit(wxCommandEvent& event);
	void OnCancelQuit(wxCommandEvent& event);
	void OnSearchList(wxCommandEvent& event);
	void OnCancelSearch(wxCommandEvent& event);
	void OnSelectModInMasterlist(wxTreeEvent& event);
	void OnSortingCheckToggle(wxCommandEvent& event);
	void OnMessageAddToggle(wxCommandEvent& event);
	void OnSortInsertChange(wxCommandEvent& event);
	void OnRuleCreate(wxCommandEvent& event);
	void OnRuleEdit(wxCommandEvent& event);
	void OnRuleDelete(wxCommandEvent& event);
	void OnRuleOrderChange(wxCommandEvent& event);
	void OnRuleSelection(wxCommandEvent& event);

	void OnDragStart(wxTreeEvent& event);

	DECLARE_EVENT_TABLE()

	friend class TextDropTarget;
private:
	void LoadLists();
	Rule GetRuleFromForm();

	ItemList masterlist;

	wxArrayString ModlistMods;

	wxTextDataObject *dragData;
	wxDropSource *dragSource;
	wxDragResult dragResult;
	TextDropTarget *ForDropTarget;
	TextDropTarget *SortDropTarget;
	TextDropTarget *InsertDropTarget;

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