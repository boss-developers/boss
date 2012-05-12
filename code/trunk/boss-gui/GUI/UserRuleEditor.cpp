/*	BOSS

	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of BOSS.

    BOSS is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see 
	<http://www.gnu.org/licenses/>.

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "GUI/UserRuleEditor.h"

#include <wx/progdlg.h>

#include <boost/algorithm/string.hpp>

BEGIN_EVENT_TABLE( UserRulesEditorFrame, wxFrame )
	EVT_BUTTON ( BUTTON_OKExitEditor, UserRulesEditorFrame::OnOKQuit )
	EVT_BUTTON ( BUTTON_CancelExitEditor, UserRulesEditorFrame::OnCancelQuit )
	EVT_BUTTON ( BUTTON_NewRule, UserRulesEditorFrame::OnRuleCreate )
	EVT_BUTTON ( BUTTON_EditRule, UserRulesEditorFrame::OnRuleEdit )
	EVT_BUTTON ( BUTTON_DeleteRule, UserRulesEditorFrame::OnRuleDelete )
	EVT_BUTTON ( BUTTON_MoveRuleUp, UserRulesEditorFrame::OnRuleOrderChange )
	EVT_BUTTON ( BUTTON_MoveRuleDown, UserRulesEditorFrame::OnRuleOrderChange )
	EVT_CHECKBOX ( CHECKBOX_SortMods, UserRulesEditorFrame::OnSortingCheckToggle )
	EVT_CHECKBOX ( CHECKBOX_AddMessages, UserRulesEditorFrame::OnMessageAddToggle )
	EVT_TREE_SEL_CHANGED ( LIST_Masterlist, UserRulesEditorFrame::OnSelectModInMasterlist )
	EVT_TREE_BEGIN_DRAG ( LIST_Masterlist, UserRulesEditorFrame::OnDragStart )
	EVT_TREE_BEGIN_DRAG ( LIST_Modlist, UserRulesEditorFrame::OnDragStart )
	EVT_LISTBOX ( wxID_ANY, UserRulesEditorFrame::OnRuleSelection )
	EVT_RADIOBUTTON ( RADIO_SortMod, UserRulesEditorFrame::OnSortInsertChange )
	EVT_RADIOBUTTON ( RADIO_InsertMod, UserRulesEditorFrame::OnSortInsertChange )
	EVT_SEARCHCTRL_SEARCH_BTN ( SEARCH_Masterlist, UserRulesEditorFrame::OnSearchList )
	EVT_SEARCHCTRL_SEARCH_BTN ( SEARCH_Modlist, UserRulesEditorFrame::OnSearchList )
	EVT_SEARCHCTRL_CANCEL_BTN ( SEARCH_Masterlist, UserRulesEditorFrame::OnCancelSearch )
	EVT_SEARCHCTRL_CANCEL_BTN ( SEARCH_Modlist, UserRulesEditorFrame::OnCancelSearch )
	EVT_TEXT_ENTER ( SEARCH_Masterlist, UserRulesEditorFrame::OnSearchList )
	EVT_TEXT_ENTER ( SEARCH_Modlist, UserRulesEditorFrame::OnSearchList )
END_EVENT_TABLE()

BEGIN_EVENT_TABLE( RuleListFrameClass, wxPanel )
	EVT_CHECKBOX ( wxID_ANY, RuleListFrameClass::OnToggleRule )
	EVT_LISTBOX ( wxID_ANY, RuleListFrameClass::OnRuleSelection )
END_EVENT_TABLE()

BEGIN_EVENT_TABLE( RuleBoxClass, wxPanel )
	EVT_CHECKBOX ( wxID_ANY, RuleBoxClass::ToggleEnabled )
	EVT_LEFT_DOWN ( RuleBoxClass::OnSelect )
END_EVENT_TABLE()


using namespace boss;
using namespace std;

using boost::algorithm::trim_copy;

UserRulesEditorFrame::UserRulesEditorFrame(const wxChar *title, wxFrame *parent) : wxFrame(parent, wxID_ANY, title) {

	//Let's give this a progress bar.
	wxProgressDialog *progDia = new wxProgressDialog(wxT("BOSS: Working..."),wxT("Initialising User Rules Manager..."), 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_CAN_ABORT);
	progDia->Pulse();

	try{
		LoadLists();
	} catch(boss_error &e) {
		progDia->Destroy();
		this->Close();
		wxMessageBox(wxString::Format(
				wxT("Error: "+e.getString())
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
		return;
	}

	if (!progDia->Pulse())
		progDia->Destroy();

	//Some variable setup.
	wxString BeforeAfter[] = {
        wxT("before"),
        wxT("after")
    };

	wxString TopBottom[] = {
        wxT("top"),
        wxT("bottom")
    };

	//Set up stuff in the frame.
	SetBackgroundColour(wxColour(255,255,255));

	////////////////////////
	// Layout
	////////////////////////

	//Window
	wxBoxSizer *bigBox = new wxBoxSizer(wxVERTICAL);

	////Main content
	wxBoxSizer *mainBox = new wxBoxSizer(wxHORIZONTAL);

	//////First column
	wxBoxSizer *rulesBox = new wxBoxSizer(wxVERTICAL);
	try{
		rulesBox->Add(RulesList = new RuleListFrameClass(this, LIST_RuleList, masterlist), 1, wxEXPAND);
	} catch(boss_error &e) {
		progDia->Destroy();
		this->Close();
		wxMessageBox(wxString::Format(
				wxT("Error: "+e.getString())
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
		return;
	}

	if (!progDia->Pulse())
		progDia->Destroy();

	////////Rule Creator/Editor
	wxBoxSizer *editorMessagesBox = new wxBoxSizer(wxHORIZONTAL);
	wxStaticBoxSizer *ruleEditorTopBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Rule Editor"));  //Needs to go in an oulined box.
	wxBoxSizer *ruleEditorBox = new wxBoxSizer(wxVERTICAL);  //To get internal padding.
	wxBoxSizer *forBox = new wxBoxSizer(wxHORIZONTAL);
	forBox->Add(new wxStaticText(ruleEditorTopBox->GetStaticBox(), wxID_ANY, wxT("For")));
	forBox->Add(RuleModBox = new wxTextCtrl(ruleEditorTopBox->GetStaticBox(), TEXT_RuleMod, ""), 1, wxEXPAND|wxLEFT, 10);
	ruleEditorBox->Add(forBox, 0, wxEXPAND);
	ruleEditorBox->AddSpacer(10);
	ruleEditorBox->Add(SortModsCheckBox = new wxCheckBox(ruleEditorTopBox->GetStaticBox(), CHECKBOX_SortMods, wxT("Sort Item")));
	ruleEditorBox->AddSpacer(10);
	wxBoxSizer *sortModOptionBox = new wxBoxSizer(wxHORIZONTAL);
	sortModOptionBox->Add(SortModOption = new wxRadioButton(ruleEditorTopBox->GetStaticBox(), RADIO_SortMod, wxT("Sort")));
	sortModOptionBox->Add(BeforeAfterChoiceBox = new wxChoice(ruleEditorTopBox->GetStaticBox(), CHOICE_BeforeAfter, wxDefaultPosition, wxDefaultSize, 2, BeforeAfter), 0, wxLEFT, 10);
	sortModOptionBox->Add(SortModBox = new wxTextCtrl(ruleEditorTopBox->GetStaticBox(), TEXT_SortMod, ""), 1, wxEXPAND|wxLEFT, 10);
	ruleEditorBox->Add(sortModOptionBox, 0, wxEXPAND|wxLEFT, 20);
	ruleEditorBox->AddSpacer(10);
	wxBoxSizer *InsertOptionBox = new wxBoxSizer(wxHORIZONTAL);
	InsertOptionBox->Add(InsertModOption = new wxRadioButton(ruleEditorTopBox->GetStaticBox(), RADIO_InsertMod, wxT("Insert at the")));
	InsertOptionBox->Add(TopBottomChoiceBox = new wxChoice(ruleEditorTopBox->GetStaticBox(), CHOICE_TopBottom, wxDefaultPosition, wxDefaultSize, 2, TopBottom), 0, wxLEFT, 10);
	InsertOptionBox->Add(new wxStaticText(ruleEditorTopBox->GetStaticBox(), wxID_ANY, wxT("of")), 0, wxLEFT, 10);
	InsertOptionBox->Add(InsertModBox = new wxTextCtrl(ruleEditorTopBox->GetStaticBox(),TEXT_InsertMod), 1, wxEXPAND|wxLEFT, 10);
	ruleEditorBox->Add(InsertOptionBox, 0, wxEXPAND|wxLEFT, 20);
	ruleEditorBox->AddSpacer(10);
	ruleEditorBox->Add(AddMessagesCheckBox = new wxCheckBox(ruleEditorTopBox->GetStaticBox(), CHECKBOX_AddMessages, wxT("Add the following messages:")));
	ruleEditorBox->AddSpacer(10);
	ruleEditorBox->Add(NewModMessagesBox = new wxTextCtrl(ruleEditorTopBox->GetStaticBox(),TEXT_NewMessages, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE), 1, wxEXPAND|wxLEFT, 20);
	ruleEditorBox->AddSpacer(10);
	ruleEditorBox->Add(ReplaceMessagesCheckBox = new wxCheckBox(ruleEditorTopBox->GetStaticBox(), CHECKBOX_RemoveMessages, wxT("Replace existing messages")));
	ruleEditorTopBox->Add(ruleEditorBox, 1, wxEXPAND|wxALL, 10);
	editorMessagesBox->Add(ruleEditorTopBox, 0, wxEXPAND);
	editorMessagesBox->AddSpacer(10);
	wxStaticBoxSizer *messBox = new wxStaticBoxSizer(wxVERTICAL, this, "Default Plugin Messages");
	messBox->Add(ModMessagesBox = new wxTextCtrl(messBox->GetStaticBox(),TEXT_ModMessages,wxT(""),wxDefaultPosition,wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY), 1, wxEXPAND);
	editorMessagesBox->Add(messBox, 1, wxEXPAND);
	rulesBox->Add(editorMessagesBox, 1, wxEXPAND);
	mainBox->Add(rulesBox, 3, wxEXPAND);
	//////Second column.
	TabHolder = new wxNotebook(this, wxID_ANY);
	////////Modlist tab.
	ModlistTab = new wxPanel(TabHolder);
	wxBoxSizer *ModlistTabSizer = new wxBoxSizer(wxVERTICAL);
	ModlistTabSizer->Add(ModlistSearch = new wxSearchCtrl(ModlistTab, SEARCH_Modlist, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), 0, wxEXPAND);
	ModlistTabSizer->Add(InstalledModsList = new wxTreeCtrl(ModlistTab, LIST_Modlist, wxDefaultPosition, wxSize(100,550), wxTR_HAS_BUTTONS|wxTR_TWIST_BUTTONS|wxTR_NO_LINES|wxTR_FULL_ROW_HIGHLIGHT|wxTR_HIDE_ROOT), 1, wxEXPAND);
	ModlistTab->SetSizer(ModlistTabSizer);
	////////Masterlist tab.
	MasterlistTab = new wxPanel(TabHolder);
	wxBoxSizer *MasterlistTabSizer = new wxBoxSizer(wxVERTICAL);
	MasterlistTabSizer->Add(MasterlistSearch = new wxSearchCtrl(MasterlistTab, SEARCH_Masterlist, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), 0, wxEXPAND);
	MasterlistTabSizer->Add(MasterlistModsList = new wxTreeCtrl(MasterlistTab, LIST_Masterlist, wxDefaultPosition, wxSize(100,550), wxTR_HAS_BUTTONS|wxTR_TWIST_BUTTONS|wxTR_NO_LINES|wxTR_HIDE_ROOT), 1, wxEXPAND);
	MasterlistTab->SetSizer(MasterlistTabSizer);
	//Add tabs to window.
	TabHolder->AddPage(ModlistTab,wxT("Installed Plugins"),true);
	TabHolder->AddPage(MasterlistTab,wxT("Masterlist"));
	mainBox->AddSpacer(10);
	mainBox->Add(TabHolder, 2, wxEXPAND);
	bigBox->Add(mainBox, 1, wxALL|wxEXPAND, 10);

	////Window buttons
	wxBoxSizer *mainButtonBox = new wxBoxSizer(wxHORIZONTAL);
	mainButtonBox->Add(new wxButton(this, BUTTON_NewRule, wxT("Create New Rule")));
	mainButtonBox->Add(new wxButton(this, BUTTON_EditRule, wxT("Save Edited Rule")), 0, wxLEFT, 10);
	mainButtonBox->Add(new wxButton(this, BUTTON_DeleteRule, wxT("Delete Rule")), 0, wxLEFT, 10);
	mainButtonBox->AddStretchSpacer(2);
	mainButtonBox->Add(new wxButton(this, BUTTON_MoveRuleUp, wxT("Move Rule Up")));
	mainButtonBox->Add(new wxButton(this, BUTTON_MoveRuleDown, wxT("Move Rule Down")), 0, wxLEFT, 10);
	mainButtonBox->AddStretchSpacer(2);
	mainButtonBox->Add(new wxButton(this, BUTTON_OKExitEditor, wxT("Save and Exit")));
	mainButtonBox->Add(new wxButton(this, BUTTON_CancelExitEditor, wxT("Cancel")), 0, wxLEFT, 10);
	//Now add buttons to window sizer.
	bigBox->Add(mainButtonBox, 0, wxALL|wxEXPAND, 10);

	//Now set defaults.
	SortModOption->SetValue(true);
	SortModOption->Enable(false);
	InsertModOption->Enable(false);
	BeforeAfterChoiceBox->Enable(false);
	SortModBox->Enable(false);
	TopBottomChoiceBox->Enable(false);
	InsertModBox->Enable(false);
	BeforeAfterChoiceBox->SetSelection(0);
	TopBottomChoiceBox->SetSelection(0);

	NewModMessagesBox->Enable(false);
	ReplaceMessagesCheckBox->Enable(false);

	//Tooltips.
	NewModMessagesBox->SetToolTip("Messages must be entered in the correct format. See the User Rules Readme for more information.");

	//Set up drag 'n' drop.
	ForDropTarget = new TextDropTarget(RuleModBox);
	SortDropTarget = new TextDropTarget(SortModBox);
	InsertDropTarget = new TextDropTarget(InsertModBox);
	RuleModBox->SetDropTarget(ForDropTarget);
	SortModBox->SetDropTarget(SortDropTarget);
	InsertModBox->SetDropTarget(InsertDropTarget);

	//Fill modlist and masterlist.
	wxTreeItemId root = InstalledModsList->AddRoot("Installed Mods");
	for (size_t i=0;i<ModlistMods.size();i++) {
		InstalledModsList->AppendItem(root, ModlistMods[i]);
	}

	vector<wxTreeItemId> opengroups;
	opengroups.push_back(MasterlistModsList->AddRoot("Masterlist"));
	vector<Item> items = masterlist.Items();
	size_t max = items.size();
	for (size_t i=0;i<max;i++) {
		if (items[i].Type() == ENDGROUP)
			opengroups.pop_back();
		else {
			wxTreeItemId item = MasterlistModsList->AppendItem(opengroups.back(), items[i].Name());
			if (items[i].Type() == BEGINGROUP)
				opengroups.push_back(item);
		}
	}

	//Now set the layout and sizes.
	SetSizerAndFit(bigBox);

	progDia->Destroy();
}

void UserRulesEditorFrame::OnOKQuit(wxCommandEvent& event) {
	RulesList->SaveUserlist(gl_current_game.Userlist());
	this->Close();
}

void UserRulesEditorFrame::OnCancelQuit(wxCommandEvent& event) {
	this->Close();
}

void UserRulesEditorFrame::OnSearchList(wxCommandEvent& event) {
	if (event.GetId() == SEARCH_Modlist) {
		ModlistSearch->ShowCancelButton(true);
		string searchStr = ModlistSearch->GetValue();
		size_t length = searchStr.length();
		if (length == 0) {
			OnCancelSearch(event);
			return;
		}
		InstalledModsList->DeleteAllItems();
		wxTreeItemId root = InstalledModsList->AddRoot("Installed Mods");
		for (size_t i=0;i<ModlistMods.size();i++) {
			if (boost::iequals(ModlistMods[i].substr(0,length).ToStdString(), searchStr))
				InstalledModsList->AppendItem(root, ModlistMods[i]);
		}
	} else {
		MasterlistSearch->ShowCancelButton(true);
		string searchStr = MasterlistSearch->GetValue();
		size_t length = searchStr.length();
		if (length == 0) {
			OnCancelSearch(event);
			return;
		}
		MasterlistModsList->DeleteAllItems();
		vector<wxTreeItemId> opengroups;
		opengroups.push_back(MasterlistModsList->AddRoot("Masterlist"));
		vector<Item> items = masterlist.Items();
		size_t max = items.size();
		for (size_t i=0;i<max;i++) {
			if (boost::iequals(items[i].Name().substr(0,length), searchStr)) {
					wxTreeItemId item = MasterlistModsList->AppendItem(opengroups.back(), items[i].Name());
				if (items[i].Type() == BEGINGROUP)
					opengroups.push_back(item);
				else if (items[i].Type() == ENDGROUP)
					opengroups.pop_back();
			}
		}
	}
}

void UserRulesEditorFrame::OnCancelSearch(wxCommandEvent& event) {
	if (event.GetId() == SEARCH_Modlist) {
		ModlistSearch->ShowCancelButton(false);
		ModlistSearch->SetValue("");
		InstalledModsList->DeleteAllItems();
		wxTreeItemId root = InstalledModsList->AddRoot("Installed Mods");
		for (size_t i=0;i<ModlistMods.size();i++) {
			InstalledModsList->AppendItem(root, ModlistMods[i]);
		}
	} else {
		MasterlistSearch->ShowCancelButton(false);
		MasterlistSearch->SetValue("");
		MasterlistModsList->DeleteAllItems();
		vector<wxTreeItemId> opengroups;
		vector<Item> items = masterlist.Items();
		size_t max = items.size();
		opengroups.push_back(MasterlistModsList->AddRoot("Masterlist"));
		for (size_t i=0;i<max;i++) {
			wxTreeItemId item = MasterlistModsList->AppendItem(opengroups.back(), items[i].Name());
			if (items[i].Type() == BEGINGROUP)
				opengroups.push_back(item);
			else if (items[i].Type() == ENDGROUP)
				opengroups.pop_back();
		}
	}
}

void UserRulesEditorFrame::OnSelectModInMasterlist(wxTreeEvent& event) {
	//Need to find item in masterlist. :( Why can't tree lists store index number?
	size_t pos = masterlist.FindItem(MasterlistModsList->GetItemText(event.GetItem()).ToStdString());
	if (pos != masterlist.Items().size()) {
		string messagesOut = "";
		vector<Message> messages = masterlist.ItemAt(pos).Messages();
		for (vector<Message>::iterator messageIter = messages.begin(); messageIter != messages.end(); ++messageIter)
			messagesOut += messageIter->KeyToString() + ": " + messageIter->Data() + "\n\n";
		ModMessagesBox->SetValue(messagesOut.substr(0,messagesOut.length()-2));
	}
}

void UserRulesEditorFrame::OnSortingCheckToggle(wxCommandEvent& event) {
	if (event.IsChecked()) {
		SortModOption->Enable(true);
		InsertModOption->Enable(true);
		BeforeAfterChoiceBox->Enable(true);
		SortModBox->Enable(true);
		TopBottomChoiceBox->Enable(true);
		InsertModBox->Enable(true);
		SortModOption->SetValue(true);
	} else {
		SortModOption->Enable(false);
		InsertModOption->Enable(false);
		BeforeAfterChoiceBox->Enable(false);
		SortModBox->Enable(false);
		TopBottomChoiceBox->Enable(false);
		InsertModBox->Enable(false);
	}
}

void UserRulesEditorFrame::OnMessageAddToggle(wxCommandEvent& event) {
	NewModMessagesBox->Enable(event.IsChecked());
	ReplaceMessagesCheckBox->Enable(event.IsChecked());
}

void UserRulesEditorFrame::OnSortInsertChange(wxCommandEvent& event) {
	if (event.GetId() == RADIO_SortMod) {
		BeforeAfterChoiceBox->Enable(true);
		SortModBox->Enable(true);
		TopBottomChoiceBox->Enable(false);
		InsertModBox->Enable(false);
	} else {
		TopBottomChoiceBox->Enable(true);
		InsertModBox->Enable(true);
		BeforeAfterChoiceBox->Enable(false);
		SortModBox->Enable(false);
	}
}

void UserRulesEditorFrame::OnRuleCreate(wxCommandEvent& event) {
	try {
		Rule newRule = GetRuleFromForm();
		RulesList->AppendRule(newRule);
	} catch (boss_error &e) {
		wxMessageBox(wxString::Format(
				wxT("Rule Syntax Error: " + e.getString() + " Please correct the mistake before continuing.")
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
	}
		
}

void UserRulesEditorFrame::OnRuleEdit(wxCommandEvent& event) {
	wxMessageDialog *dlg = new wxMessageDialog(this,
			wxT("Are you sure you want to save your changes to the selected rule?")
			, wxT("BOSS: User Rules Editor"), wxYES_NO);

	if (dlg->ShowModal() != wxID_YES)  //User has chosen not to save.
		return;
	else {  //User has chosen to save.
		try {
			Rule newRule = GetRuleFromForm();
			RulesList->SaveEditedRule(newRule);
		} catch (boss_error &e) {
			wxMessageBox(wxString::Format(
					wxT("Rule Syntax Error: " + e.getString() + " Please correct the mistake before continuing.")
				),
				wxT("BOSS: Error"),
				wxOK | wxICON_ERROR,
				NULL);
		}
			
	}
}

void UserRulesEditorFrame::OnRuleDelete(wxCommandEvent& event) {
	wxMessageDialog *dlg = new wxMessageDialog(this,
			wxT("Are you sure you want to delete the selected rule?")
			, wxT("BOSS: User Rules Editor"), wxYES_NO);

	if (dlg->ShowModal() != wxID_YES)  //User has chosen not to delete.
		return;
	else  //User has chosen to delete.
		RulesList->DeleteSelectedRule();  //This doesn't work.
}

void UserRulesEditorFrame::OnRuleSelection(wxCommandEvent& event) {
	Rule currentRule = RulesList->GetSelectedRule();
	string messages = "";
	SortModOption->Enable(true);
	InsertModOption->Enable(true);
	SortModsCheckBox->SetValue(false);
	TopBottomChoiceBox->Enable(false);
	InsertModBox->Enable(false);
	BeforeAfterChoiceBox->Enable(false);
	SortModBox->Enable(false);
	AddMessagesCheckBox->SetValue(false);
	NewModMessagesBox->Enable(false);
	ReplaceMessagesCheckBox->Enable(false);
	ReplaceMessagesCheckBox->SetValue(false);
	RuleModBox->SetValue(currentRule.Object());
	SortModBox->SetValue("");
	InsertModBox->SetValue("");
	vector<RuleLine> lines = currentRule.Lines();
	size_t size = lines.size();
	for (size_t j=0;j<size;j++) {
		if (lines[j].Key() == BEFORE || lines[j].Key() == AFTER) {
			SortModsCheckBox->SetValue(true);
			SortModOption->SetValue(true);
			SortModBox->Enable(true);
			BeforeAfterChoiceBox->Enable(true);
			SortModBox->SetValue(lines[j].Object());
			if (lines[j].Key() == BEFORE)
				BeforeAfterChoiceBox->SetSelection(0);
			else
				BeforeAfterChoiceBox->SetSelection(1);
		} else if (lines[j].Key() == TOP || lines[j].Key() == BOTTOM) {
			SortModsCheckBox->SetValue(true);
			InsertModOption->SetValue(true);
			TopBottomChoiceBox->Enable(true);
			InsertModBox->Enable(true);
			InsertModBox->SetValue(lines[j].Object());
			if (lines[j].Key() == TOP)
				TopBottomChoiceBox->SetSelection(0);
			else
				TopBottomChoiceBox->SetSelection(1);
		} else if (lines[j].Key() == APPEND || lines[j].Key() == REPLACE) {
			AddMessagesCheckBox->SetValue(true);
			NewModMessagesBox->Enable(true);
			ReplaceMessagesCheckBox->Enable(true);
			if (lines[j].Key() == REPLACE)
				ReplaceMessagesCheckBox->SetValue(true);
			messages += lines[j].Object() + "\n";
		}
	}
	NewModMessagesBox->SetValue(messages);
}

void UserRulesEditorFrame::LoadLists() {
	ItemList modlist;
	size_t size;

	///////////////
	// Modlist
	///////////////

	//Need to parse userlist, masterlist and build modlist.
	try {
		modlist.Load(gl_current_game.DataFolder());
	} catch (boss_error &e) {
		throw boss_error(BOSS_ERROR_GUI_WINDOW_INIT_FAIL, "User Rules Editor", e.getString());
	}

	vector<Item> items = modlist.Items();
	size = items.size();
	for (size_t i=0;i<size;i++)
		ModlistMods.push_back(items[i].Name());

	////////////////
	// Masterlist
	////////////////

	//Parse masterlist/modlist backup into data structure.
	LOG_INFO("Starting to parse sorting file: %s", gl_current_game.Masterlist().string().c_str());
	try {
		masterlist.Load(gl_current_game.Masterlist());
		masterlist.EvalConditions();
		masterlist.EvalRegex();
	} catch (boss_error &e) {
		throw boss_error(BOSS_ERROR_GUI_WINDOW_INIT_FAIL, "User Rules Editor", e.getString());
	}
}

Rule UserRulesEditorFrame::GetRuleFromForm() {
	Rule newRule;
	//First validate.
	//Calling functions need to check for an enabled = false; rule as a failure.
	//Failure description is given in ruleObject.
	if (Item(RuleModBox->GetValue().ToStdString()).IsPlugin()) {
		if (SortModsCheckBox->IsChecked()) {
			if (SortModOption->GetValue()) {
				if (SortModOption->GetValue() && SortModBox->IsEmpty())
					throw boss_error("No mod is specified to sort relative to.", BOSS_ERROR_INVALID_SYNTAX);
				else if (!Item(SortModBox->GetValue().ToStdString()).IsPlugin())  //Sort object is a group. Error.
					throw boss_error("Cannot sort a plugin relative to a group.", BOSS_ERROR_INVALID_SYNTAX);
			} else if (InsertModOption->GetValue() && !Item(InsertModBox->GetValue().ToStdString()).IsGroup()) {  //Inserting into a mod. Error.
				if (InsertModBox->IsEmpty())
					throw boss_error("No group is specified to insert into.", BOSS_ERROR_INVALID_SYNTAX);
				else
					throw boss_error("Cannot insert into a plugin.", BOSS_ERROR_INVALID_SYNTAX);
			}
		}
		if (AddMessagesCheckBox->IsChecked() && NewModMessagesBox->IsEmpty())  //Can't add no messages. Error.
			throw boss_error("Cannot add messages when none are given.", BOSS_ERROR_INVALID_SYNTAX);
	} else {  //Rule object is a group, or empty.
		if (RuleModBox->IsEmpty())
			throw boss_error("No rule mod is specified.", BOSS_ERROR_INVALID_SYNTAX);
		if (SortModsCheckBox->IsChecked()) {
			if (SortModOption->GetValue()) {
				if (SortModBox->IsEmpty())  //No sort object specified. Error.
					throw boss_error("No mod is specified to sort relative to.", BOSS_ERROR_INVALID_SYNTAX);
				else if (Item(SortModBox->GetValue().ToStdString()).IsPlugin())  //Sort object is a plugin. Error.
					throw boss_error("Cannot sort a group relative to a plugin.", BOSS_ERROR_INVALID_SYNTAX);
			} else if (InsertModOption->GetValue())  //Can't insert groups. Error.
				throw boss_error("Cannot insert groups.", BOSS_ERROR_INVALID_SYNTAX);
		}
		if (AddMessagesCheckBox->IsChecked())  //Can't add messages to a group. Error.
			throw boss_error("Cannot add messages to groups.", BOSS_ERROR_INVALID_SYNTAX);
	}

	newRule.Enabled(true);
	newRule.Object(RuleModBox->GetValue().ToStdString());
	if (!SortModsCheckBox->IsChecked())
		newRule.Key(FOR);
	else {
		size_t pos = masterlist.FindItem(newRule.Object());
		if (pos != masterlist.Items().size())  //Mod in masterlist.
			newRule.Key(OVERRIDE);
		else
			newRule.Key(ADD);
		
		if (SortModOption->GetValue()) {
			RuleLine newLine;
			newLine.Object(SortModBox->GetValue().ToStdString());
			if (BeforeAfterChoiceBox->GetSelection() == 0)
				newLine.Key(BEFORE);
			else
				newLine.Key(AFTER);
			vector<RuleLine> lines = newRule.Lines();
			lines.push_back(newLine);
			newRule.Lines(lines);
		} else if (InsertModOption->GetValue()) {
			RuleLine newLine;
			newLine.Object(InsertModBox->GetValue().ToStdString());
			if (TopBottomChoiceBox->GetSelection() == 0)
				newLine.Key(TOP);
			else
				newLine.Key(BOTTOM);
			vector<RuleLine> lines = newRule.Lines();
			lines.push_back(newLine);
			newRule.Lines(lines);
		}
	}

	//Now add message lines.
	if (AddMessagesCheckBox->IsChecked()) {
		RuleLine newLine;
		string messages = NewModMessagesBox->GetValue();
		if (!messages.empty()) {
			//Split messages string by \n characters.
			size_t pos1 = 0, pos2 = string::npos;
			pos2 = messages.find("\n");
			if (pos2 == string::npos)  //No \n characters.
				pos2 = messages.length()-1;
			while (pos2 != string::npos) {
				newLine.Object(trim_copy(messages.substr(pos1,pos2-pos1)));
				if (pos1 == 0 && ReplaceMessagesCheckBox->IsChecked())
					newLine.Key(REPLACE);
				else
					newLine.Key(APPEND);
				
				if (!newLine.IsObjectMessage())  //Message is formatted incorrectly. Error.
					throw boss_error("The message \"" + newLine.Object() + "\" is formatted incorrectly.", BOSS_ERROR_INVALID_SYNTAX);
				
				vector<RuleLine> lines = newRule.Lines();
				lines.push_back(newLine);
				newRule.Lines(lines);

				if (pos2 >= messages.length()-1)
					break;
				pos1 = pos2 + 1;
				pos2 = messages.find("\n", pos1);
				if (pos2 == string::npos && pos1 < messages.length()-1)
					pos2 = messages.length()-1;
			}
		}
	}

	return newRule;
}

void UserRulesEditorFrame::OnRuleOrderChange(wxCommandEvent& event) {
	RulesList->MoveRule(event.GetId());
}

void UserRulesEditorFrame::OnDragStart(wxTreeEvent& event) {
	if (event.GetId() == LIST_Modlist) {
		dragData = new wxTextDataObject(InstalledModsList->GetItemText(event.GetItem()));
		dragSource = new wxDropSource(InstalledModsList);
	} else if (event.GetId() == LIST_Masterlist) {
		dragData = new wxTextDataObject(MasterlistModsList->GetItemText(event.GetItem()));
		dragSource = new wxDropSource(MasterlistModsList);
	}
	dragSource->SetData(*dragData);
	dragResult = dragSource->DoDragDrop();

/*	switch(dragResult) {  //Don't actually need to do anything with result.
	case wxDragCancel:
		wxMessageBox("Drag 'n' drop cancelled.");
		break;
	case wxDragNone:
		wxMessageBox("Drag 'n' drop error.");
		break;
	case wxDragCopy:
		wxMessageBox("Drag 'n' drop copied.");
		break;
	case wxDragMove:
		wxMessageBox("Drag 'n' drop moved.");
		break;
	case wxDragLink:
		wxMessageBox("Drag 'n' dropped link.");
		break;
	default:
		break;
	}*/
}

//////////////////////////////
// TextDropTarget functions
//////////////////////////////

TextDropTarget::TextDropTarget(wxTextCtrl *owner) {
	targetOwner = owner;
}

bool TextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString &data) {
	wxString originalValue = targetOwner->GetValue();
	targetOwner->SetValue(data);

	UserRulesEditorFrame *ureFrame = (UserRulesEditorFrame*)targetOwner->GetParent()->GetParent();  //Targets are owned by the static box created by the sizer they're in, which is in turn owned by the URE window.
	Item sortItem(ureFrame->SortModBox->GetValue().ToStdString());
	Item forItem(ureFrame->RuleModBox->GetValue().ToStdString());
	Item insertItem(ureFrame->InsertModBox->GetValue().ToStdString());
	bool isSorting = ureFrame->SortModsCheckBox->IsChecked();
	bool isInserting = ureFrame->InsertModOption->GetValue();

	if (isSorting && !forItem.Name().empty()) {
		if (forItem.IsPlugin()) {
			if (!isInserting && sortItem.IsGroup()) {  //Sort object is a group. Error.
				wxMessageBox(wxString::Format(
						wxT("Rule Syntax Error: Cannot sort a plugin relative to a group.")
					),
					wxT("BOSS: Error"),
					wxOK | wxICON_ERROR,
					NULL);
				targetOwner->SetValue(originalValue);
				return false;
			} else if (isInserting && insertItem.IsPlugin()) {  //Inserting into a mod. Error.
				wxMessageBox(wxString::Format(
						wxT("Rule Syntax Error: Cannot insert into a plugin.")
					),
					wxT("BOSS: Error"),
					wxOK | wxICON_ERROR,
					NULL);
				targetOwner->SetValue(originalValue);
				return false;
			}
		} else {  //Rule object is a group.
			if (!isInserting && sortItem.IsPlugin()) {  //Sort object is a plugin. Error.
				wxMessageBox(wxString::Format(
						wxT("Rule Syntax Error: Cannot sort a group relative to a plugin.")
					),
					wxT("BOSS: Error"),
					wxOK | wxICON_ERROR,
					NULL);
				targetOwner->SetValue(originalValue);
				return false;
			} else if (isInserting) {  //Can't insert groups. Error.
				wxMessageBox(wxString::Format(
						wxT("Rule Syntax Error: Cannot insert groups.")
					),
					wxT("BOSS: Error"),
					wxOK | wxICON_ERROR,
					NULL);
				targetOwner->SetValue(originalValue);
				return false;
			}
		}
	}
	return true;
}


////////////////////////////
// RuleBoxClass functions
////////////////////////////

RuleBoxClass::RuleBoxClass(wxScrolled<wxPanel> *parent, Rule currentRule, uint32_t index, bool isSelected) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize) {
	//First get text representation of rule.
	string text = Outputter(PLAINTEXT, currentRule).AsString();
	ruleIndex = index;
	
	//Now do GUI stuff.
	SetBackgroundColour(wxColour(255,255,255));

	wxFlexGridSizer *mainSizer = new wxFlexGridSizer(2,0,0);
	mainSizer->SetFlexibleDirection(wxHORIZONTAL);
	mainSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_NONE);
	mainSizer->AddGrowableCol(1,0);
	mainSizer->Add(ruleCheckbox = new wxCheckBox(this, wxID_ANY, ""),0,wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxTOP|wxBOTTOM,10);
	mainSizer->Add(ruleContent = new wxStaticText(this, wxID_ANY, text),0,wxEXPAND|wxRIGHT|wxTOP,10);

	ruleCheckbox->SetValue(currentRule.Enabled());
	ruleContent->Bind(wxEVT_LEFT_DOWN, &RuleBoxClass::OnSelect, this, wxID_ANY);
	if (!currentRule.Enabled())
		ruleContent->Enable(false);
	if (isSelected)
		SetBackgroundColour(wxColour(240,240,240));

	SetSizerAndFit(mainSizer);
	Show();
}

void RuleBoxClass::ToggleEnabled(wxCommandEvent& event) {
	if (event.IsChecked())
		ruleContent->Enable(true);
	else
		ruleContent->Enable(false);
	Refresh();
	event.SetId(ruleIndex);
	GetGrandParent()->ProcessWindowEvent(event);
}

void RuleBoxClass::OnSelect(wxMouseEvent& event) {
	event.SetId(ruleIndex);
	event.SetEventType(wxEVT_COMMAND_LISTBOX_SELECTED);
	GetGrandParent()->ProcessWindowEvent(event);
}

void RuleBoxClass::Highlight(bool highlight) {
	if (highlight)
		SetBackgroundColour(wxColour(240,240,240));
	else
		SetBackgroundColour(wxColour(255,255,255));
	Refresh();
}

//////////////////////////////////
// RuleListFrameClass functions
//////////////////////////////////

RuleListFrameClass::RuleListFrameClass(wxFrame *parent, wxWindowID id, ItemList &masterlist) : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize) {
	//Parse userlist.
	LOG_INFO("Starting to parse userlist.");
	try {
		userlist.Load(gl_current_game.Userlist());
	} catch (boss_error &e) {
		userlist.Clear();
		LOG_ERROR("Error: %s", e.getString().c_str());
		throw boss_error(BOSS_ERROR_GUI_WINDOW_INIT_FAIL, "User Rules Editor", e.getString());
	}

	//Now disable any ADD rules with rule mods that are in the masterlist.
	vector<Rule> rules = userlist.Rules();
	size_t size = rules.size();
	for (size_t i=0;i<size;i++) {
		if (rules[i].Key() == ADD) {
			size_t pos = masterlist.FindItem(rules[i].Object());
			if (pos != masterlist.Items().size())  //Mod in masterlist.
				rules[i].Enabled(false);
		}
	}
	userlist.Rules(rules);

	//Now set up GUI layout.
	SetBackgroundColour(*wxWHITE);

	wxStaticBoxSizer *staticListBox = new wxStaticBoxSizer(wxVERTICAL, this, "User Rules");
	staticListBox->Add(RuleListScroller = new wxScrolled<wxPanel>(this, wxID_ANY), 1, wxEXPAND);

	RuleListScroller->SetBackgroundColour(*wxWHITE);
	ReDrawRuleList();
	RuleListScroller->SetScrollRate(10, 10);
	RuleListScroller->SetAutoLayout(true);
	RuleListScroller->Show();

	SetSizerAndFit(staticListBox);
	Show();
	SetAutoLayout(true);
}

void RuleListFrameClass::SaveUserlist(const fs::path path) {
	try {
		userlist.Save(path);
	} catch (boss_error &e) {
		wxMessageBox(wxString::Format(
			wxT("Error: " + e.getString())
		),
		wxT("BOSS: Error"),
		wxOK | wxICON_ERROR,
		NULL);
	}
}

void RuleListFrameClass::ReDrawRuleList() {
	RuleListScroller->DestroyChildren();
	size_t size = userlist.Rules().size();
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	for (size_t i=0;i<size;i++) {
		if (i == selectedRuleIndex)
			sizer->Add(new RuleBoxClass(RuleListScroller, userlist.RuleAt(i), i, true), 0, wxEXPAND);
		else
			sizer->Add(new RuleBoxClass(RuleListScroller, userlist.RuleAt(i), i, false), 0, wxEXPAND);
	}
	RuleListScroller->SetSizer(sizer);
	RuleListScroller->FitInside();
	RuleListScroller->Layout();
}

void RuleListFrameClass::MoveRule(wxWindowID id) {
	if (selectedRuleIndex >= 0 && selectedRuleIndex < userlist.Rules().size()) {
		if (id == BUTTON_MoveRuleUp && selectedRuleIndex != 0) {
			Rule selectedRule = userlist.RuleAt(selectedRuleIndex);
			userlist.Erase(selectedRuleIndex);
			userlist.Insert(selectedRuleIndex - 1, selectedRule);
			selectedRuleIndex--;
			ReDrawRuleList();
		} else if (id == BUTTON_MoveRuleDown && selectedRuleIndex != userlist.Rules().size()-1) {
			Rule selectedRule = userlist.RuleAt(selectedRuleIndex);
			userlist.Erase(selectedRuleIndex);
			userlist.Insert(selectedRuleIndex + 1, selectedRule);
			selectedRuleIndex++;
			ReDrawRuleList();
		}
	}
}

void RuleListFrameClass::OnToggleRule(wxCommandEvent& event) {
	if (event.GetId() >= 0 && event.GetId() < userlist.Rules().size()) {
		uint32_t id = event.GetId();
		bool checked = event.IsChecked();
		UserRulesEditorFrame * parent = (UserRulesEditorFrame*)this->GetParent();

		Rule rule = userlist.RuleAt(id);

		rule.Enabled(checked);
		if (checked && rule.Key() == ADD && parent->masterlist.FindItem(rule.Object()) != parent->masterlist.Items().size())
			rule.Key(OVERRIDE);
		userlist.Replace(id, rule);
	}
}

Rule RuleListFrameClass::GetSelectedRule() {
	return userlist.RuleAt(selectedRuleIndex);  //If >= userlist.Rules().size() the function returns a Rule() anyway.
}

void RuleListFrameClass::AppendRule(Rule newRule) {
	//Add the rule to the end of the userlist.
	selectedRuleIndex = userlist.Rules().size();
	userlist.Insert(selectedRuleIndex, newRule);
	//Now refresh GUI.
	ReDrawRuleList();
	RuleListScroller->Scroll(RuleListScroller->GetChildren().back()->GetPosition());
}

void RuleListFrameClass::SaveEditedRule(Rule editedRule) {
	if (selectedRuleIndex >= 0 && selectedRuleIndex < userlist.Rules().size()) {
		userlist.Replace(selectedRuleIndex, editedRule);
		ReDrawRuleList();
	}
	RuleListScroller->Scroll(RuleListScroller->GetChildren()[selectedRuleIndex]->GetPosition());
}

void RuleListFrameClass::DeleteSelectedRule() {
	userlist.Erase(selectedRuleIndex);
	ReDrawRuleList();
	if (!userlist.Rules().empty()) {
		if (selectedRuleIndex == userlist.Rules().size())  //Just shortened rules by one. Make sure index isn't invalid.
			selectedRuleIndex--;
		RuleListScroller->Scroll(RuleListScroller->GetChildren()[selectedRuleIndex]->GetPosition());
	}
}

void RuleListFrameClass::OnRuleSelection(wxCommandEvent& event) {
	selectedRuleIndex = event.GetId();
	size_t size = userlist.Rules().size();
	wxWindowList list = RuleListScroller->GetChildren();
	for (size_t i=0; i<size; i++) {
		RuleBoxClass *temp = (RuleBoxClass*)list[i];
		if (i == selectedRuleIndex)
			temp->Highlight(true);
		else
			temp->Highlight(false);
	}
	GetParent()->ProcessWindowEvent(event);
}