/*	Better Oblivion Sorting Software

	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011  WrinklyNinja & the BOSS development team. 
	Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "GUI/UserRuleEditor.h"
#include "GUI/ElementIDs.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim_all.hpp>

BEGIN_EVENT_TABLE( UserRulesEditorFrame, wxFrame )
	EVT_BUTTON ( BUTTON_OKExitEditor, UserRulesEditorFrame::OnOKQuit )
	EVT_BUTTON ( BUTTON_CancelExitEditor, UserRulesEditorFrame::OnCancelQuit )
	EVT_BUTTON ( BUTTON_MoveRuleUp, UserRulesEditorFrame::OnRuleOrderChange )
	EVT_BUTTON ( BUTTON_MoveRuleDown, UserRulesEditorFrame::OnRuleOrderChange )
	EVT_BUTTON ( BUTTON_NewRule, UserRulesEditorFrame::OnRuleCreate )
	EVT_BUTTON ( BUTTON_EditRule, UserRulesEditorFrame::OnRuleEdit )
	EVT_BUTTON ( BUTTON_DeleteRule, UserRulesEditorFrame::OnRuleDelete )
	EVT_CHECKBOX ( CHECKBOX_SortMods, UserRulesEditorFrame::OnSortingCheckToggle )
	EVT_CHECKBOX ( CHECKBOX_AddMessages, UserRulesEditorFrame::OnMessageAddToggle )
	EVT_CHECKLISTBOX ( LIST_RuleList, UserRulesEditorFrame::OnToggleRuleCheckbox )  //This doesn't work for some reason.
	EVT_LISTBOX ( LIST_Masterlist, UserRulesEditorFrame::OnSelectModInMasterlist )
	EVT_LISTBOX ( LIST_RuleList, UserRulesEditorFrame::OnRuleSelection )
	EVT_RADIOBUTTON ( RADIO_SortMod, UserRulesEditorFrame::OnSortInsertChange )
	EVT_RADIOBUTTON ( RADIO_InsertMod, UserRulesEditorFrame::OnSortInsertChange )
	EVT_SEARCHCTRL_SEARCH_BTN ( SEARCH_Masterlist, UserRulesEditorFrame::OnSearchList )
	EVT_SEARCHCTRL_SEARCH_BTN ( SEARCH_Modlist, UserRulesEditorFrame::OnSearchList )
	EVT_SEARCHCTRL_CANCEL_BTN ( SEARCH_Masterlist, UserRulesEditorFrame::OnCancelSearch )
	EVT_SEARCHCTRL_CANCEL_BTN ( SEARCH_Modlist, UserRulesEditorFrame::OnCancelSearch )
	EVT_TEXT_ENTER ( SEARCH_Masterlist, UserRulesEditorFrame::OnSearchList )
	EVT_TEXT_ENTER ( SEARCH_Modlist, UserRulesEditorFrame::OnSearchList )
END_EVENT_TABLE()

using namespace boss;
using namespace std;

using boost::algorithm::to_upper_copy;
using boost::algorithm::trim_copy;

UserRulesEditorFrame::UserRulesEditorFrame(const wxChar *title, wxFrame *parent) : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize) {

	try{
		LoadLists();
	} catch(boss_error e) {
		this->Close();
		wxMessageBox(wxString::Format(
				wxT("Error: "+e.getString())
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
		return;
	}

	//Some variable setup.
	wxString BeforeAfter[] = {
        wxT("BEFORE"),
        wxT("AFTER")
    };

	wxString TopBottom[] = {
        wxT("TOP"),
        wxT("BOTTOM")
    };

	//Set up stuff in the frame.
	SetBackgroundColour(wxColour(255,255,255));

	//Sizer flags.
	wxSizerFlags ContentSizerFlags(1);
	ContentSizerFlags.Expand().Border(wxTOP|wxBOTTOM, 5);

	wxSizerFlags ItemSizerFlags(1);
	ItemSizerFlags.Border(wxLEFT|wxRIGHT, 10);

	wxSizerFlags BorderSizerFlags(0);
	BorderSizerFlags.Border(wxALL, 10);

	////////////////////////
	// Layout
	////////////////////////

	//Window
	wxBoxSizer *bigBox = new wxBoxSizer(wxVERTICAL);

	////Main content
	wxBoxSizer *mainBox = new wxBoxSizer(wxHORIZONTAL);

	//////First column
	wxBoxSizer *rulesBox = new wxBoxSizer(wxVERTICAL);
	rulesBox->Add(RulesList = new wxRearrangeList(this, LIST_RuleList, wxDefaultPosition, wxDefaultSize, RuleOrder, Rules));
	////Window buttons
	wxBoxSizer *listButtonBox = new wxBoxSizer(wxHORIZONTAL);
	listButtonBox->Add(new wxButton(this, BUTTON_MoveRuleUp, wxT("Move Up"), wxDefaultPosition, wxDefaultSize));
	listButtonBox->Add(new wxButton(this, BUTTON_MoveRuleDown, wxT("Move Down"), wxDefaultPosition, wxDefaultSize), 0, wxLEFT, 20);
	rulesBox->Add(listButtonBox, 0, wxALIGN_RIGHT|wxALL, 10);

	////////Rule Creator/Editor
	wxStaticBoxSizer *ruleEditorBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Rule Creator/Editor"));  //Needs to go in an oulined box.
	wxBoxSizer *forBox = new wxBoxSizer(wxHORIZONTAL);
	forBox->Add(new wxStaticText(this, wxID_ANY, wxT("For")));
	forBox->Add(RuleModBox = new wxTextCtrl(this,TEXT_RuleMod));
	ruleEditorBox->Add(forBox);
	ruleEditorBox->Add(SortModsCheckBox = new wxCheckBox(this, CHECKBOX_SortMods, wxT("Sort Item")));
	wxBoxSizer *sortModOptionBox = new wxBoxSizer(wxHORIZONTAL);
	sortModOptionBox->Add(SortModOption = new wxRadioButton(this, RADIO_SortMod, wxT("Sort"), wxDefaultPosition, wxDefaultSize));
	sortModOptionBox->Add(BeforeAfterChoiceBox = new wxChoice(this, CHOICE_BeforeAfter, wxDefaultPosition, wxDefaultSize, 2, BeforeAfter));
	sortModOptionBox->Add(SortModBox = new wxTextCtrl(this,TEXT_SortMod));
	ruleEditorBox->Add(sortModOptionBox);
	wxBoxSizer *InsertOptionBox = new wxBoxSizer(wxHORIZONTAL);
	InsertOptionBox->Add(InsertModOption = new wxRadioButton(this, RADIO_InsertMod, wxT("Insert at the"), wxDefaultPosition, wxDefaultSize));
	InsertOptionBox->Add(TopBottomChoiceBox = new wxChoice(this, CHOICE_TopBottom, wxDefaultPosition, wxDefaultSize, 2, TopBottom));
	InsertOptionBox->Add(new wxStaticText(this, wxID_ANY, wxT("of")));
	InsertOptionBox->Add(InsertModBox = new wxTextCtrl(this,TEXT_InsertMod));
	ruleEditorBox->Add(InsertOptionBox);
	ruleEditorBox->Add(AddMessagesCheckBox = new wxCheckBox(this, CHECKBOX_AddMessages, wxT("Add the following messages:")));
	ruleEditorBox->Add(NewModMessagesBox = new wxTextCtrl(this,TEXT_NewMessages, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE), 0, wxEXPAND);
	ruleEditorBox->Add(ReplaceMessagesCheckBox = new wxCheckBox(this, CHECKBOX_RemoveMessages, wxT("Replace Existing Messages")));
	rulesBox->Add(ruleEditorBox);
	mainBox->Add(rulesBox);
	////////Rule buttons
	wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
	buttons->Add(NewRuleButton = new wxButton(this, BUTTON_NewRule, wxT("Create New Rule"), wxDefaultPosition, wxDefaultSize));
	buttons->Add(NewRuleButton = new wxButton(this, BUTTON_EditRule, wxT("Save Edited Rule"), wxDefaultPosition, wxDefaultSize), 0, wxLEFT, 20);
	buttons->Add(NewRuleButton = new wxButton(this, BUTTON_DeleteRule, wxT("Delete Rule"), wxDefaultPosition, wxDefaultSize), 0, wxLEFT, 20);
	rulesBox->Add(buttons, 0, wxALIGN_RIGHT|wxALL, 10);
	//////Second column.
	wxBoxSizer *listmessBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *listsBox = new wxBoxSizer(wxHORIZONTAL);
	////////Modlist column.
	wxStaticBoxSizer *modlistBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Installed Mods"));
	modlistBox->Add(ModlistSearch = new wxSearchCtrl(this, SEARCH_Modlist, wxEmptyString, wxDefaultPosition, wxDefaultSize,wxTE_PROCESS_ENTER));
	modlistBox->Add(InstalledModsList = new wxTreeCtrl(this, LIST_Modlist, wxDefaultPosition, wxDefaultSize, wxTR_TWIST_BUTTONS|wxTR_NO_LINES|wxTR_FULL_ROW_HIGHLIGHT|wxTR_HIDE_ROOT));
	listsBox->Add(modlistBox);
	////////Masterlist column.
	wxStaticBoxSizer *masterlistBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Masterlist"));
	masterlistBox->Add(MasterlistSearch = new wxSearchCtrl(this, SEARCH_Masterlist, wxEmptyString, wxDefaultPosition, wxDefaultSize,wxTE_PROCESS_ENTER));
	masterlistBox->Add(MasterlistModsList = new wxTreeCtrl(this, LIST_Masterlist, wxDefaultPosition, wxDefaultSize, wxTR_TWIST_BUTTONS|wxTR_HIDE_ROOT));
	listsBox->Add(masterlistBox);
	listmessBox->Add(listsBox);
	////////Mod Messages box
	wxStaticBoxSizer *messageBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Mod Messages"));
	messageBox->Add(ModMessagesBox = new wxTextCtrl(this,TEXT_ModMessages,wxT(""),wxDefaultPosition,wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY), 0, wxEXPAND);
	
	listmessBox->Add(messageBox, 0, wxEXPAND);
	mainBox->Add(listmessBox);
	bigBox->Add(mainBox);

	////Window buttons
	wxBoxSizer *mainButtonBox = new wxBoxSizer(wxHORIZONTAL);
	mainButtonBox->Add(new wxButton(this, BUTTON_OKExitEditor, wxT("Save"), wxDefaultPosition, wxDefaultSize));
	mainButtonBox->Add(new wxButton(this, BUTTON_CancelExitEditor, wxT("Cancel"), wxDefaultPosition, wxDefaultSize), 0, wxLEFT, 20);

	//Now add TabHolder and OK button to window sizer.
	bigBox->Add(mainButtonBox, 0, wxALIGN_RIGHT|wxALL, 10);

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
	ModMessagesBox->Enable(false);

	//Fill modlist and masterlist.
	wxTreeItemId root = InstalledModsList->AddRoot("Installed Mods");
	for (size_t i=0;i<ModlistMods.size();i++) {
		InstalledModsList->AppendItem(root, ModlistMods[i]);
	}

	vector<wxTreeItemId> opengroups;
	opengroups.push_back(MasterlistModsList->AddRoot("Masterlist"));
	for (size_t i=0;i<masterlist.items.size();i++) {
		wxTreeItemId item = MasterlistModsList->AppendItem(opengroups.back(), masterlist.items[i].name.string());
		if (masterlist.items[i].type == BEGINGROUP)
			opengroups.push_back(item);
		else if (masterlist.items[i].type == ENDGROUP)
			opengroups.pop_back();
	}

	//Now set the layout and sizes.
	SetSizerAndFit(bigBox);
}

void UserRulesEditorFrame::OnOKQuit(wxCommandEvent& event) {
	try {
		userlist.Save(userlist_path);
	} catch (boss_error e) {
		wxMessageBox(wxString::Format(
			wxT("Error: "+e.getString()+" Unable to save changes.")
		),
		wxT("BOSS: Error"),
		wxOK | wxICON_ERROR,
		NULL);
	}

	this->Close();
}

void UserRulesEditorFrame::OnCancelQuit(wxCommandEvent& event) {
	this->Close();
}

void UserRulesEditorFrame::OnSearchList(wxCommandEvent& event) {
	if (event.GetId() == SEARCH_Modlist) {
		ModlistSearch->ShowCancelButton(true);
		string searchStr = ModlistSearch->GetValue();
		searchStr = Tidy(searchStr);
		size_t length = searchStr.length();
		if (length == 0) {
			OnCancelSearch(event);
			return;
		}
		InstalledModsList->DeleteAllItems();
		wxTreeItemId root = InstalledModsList->AddRoot("Installed Mods");
		for (size_t i=0;i<ModlistMods.size();i++) {
			if (Tidy(string(ModlistMods[i].substr(0,length))) == searchStr)
				InstalledModsList->AppendItem(root, ModlistMods[i]);
		}
	} else {
		MasterlistSearch->ShowCancelButton(true);
		string searchStr = MasterlistSearch->GetValue();
		searchStr = Tidy(searchStr);
		size_t length = searchStr.length();
		if (length == 0) {
			OnCancelSearch(event);
			return;
		}
		MasterlistModsList->DeleteAllItems();
		vector<wxTreeItemId> opengroups;
		opengroups.push_back(MasterlistModsList->AddRoot("Masterlist"));
		for (size_t i=0;i<masterlist.items.size();i++) {
			if (Tidy(string(masterlist.items[i].name.string().substr(0,length))) == searchStr) {
					wxTreeItemId item = MasterlistModsList->AppendItem(opengroups.back(), masterlist.items[i].name.string());
				if (masterlist.items[i].type == BEGINGROUP)
					opengroups.push_back(item);
				else if (masterlist.items[i].type == ENDGROUP)
					opengroups.pop_back();
			}
		}
	}
}

void UserRulesEditorFrame::OnCancelSearch(wxCommandEvent& event) {
	if (event.GetId() == SEARCH_Modlist) {
		ModlistSearchResultMods.Clear();
		ModlistSearch->ShowCancelButton(false);
		ModlistSearch->SetValue("");
		InstalledModsList->DeleteAllItems();
		wxTreeItemId root = InstalledModsList->AddRoot("Installed Mods");
		for (size_t i=0;i<ModlistMods.size();i++) {
			InstalledModsList->AppendItem(root, ModlistMods[i]);
		}
	} else {
		MasterlistSearchResultMods.Clear();
		MasterlistSearch->ShowCancelButton(false);
		MasterlistSearch->SetValue("");
		MasterlistModsList->DeleteAllItems();
		vector<wxTreeItemId> opengroups;
		opengroups.push_back(MasterlistModsList->AddRoot("Masterlist"));
		for (size_t i=0;i<masterlist.items.size();i++) {
			wxTreeItemId item = MasterlistModsList->AppendItem(opengroups.back(), masterlist.items[i].name.string());
			if (masterlist.items[i].type == BEGINGROUP)
				opengroups.push_back(item);
			else if (masterlist.items[i].type == ENDGROUP)
				opengroups.pop_back();
		}
	}
}

void UserRulesEditorFrame::OnSelectModInMasterlist(wxCommandEvent& event) {
	int i = event.GetSelection();
	size_t size = masterlist.items[i].messages.size();
	string messages = "";
	for (size_t j=0;j<size;j++)
		messages += masterlist.items[i].messages[j].KeyToString() + ": " + masterlist.items[i].messages[j].data + "\n";
	ModMessagesBox->SetValue(messages);
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
	Rule newRule = GetRuleFromForm();
	if (newRule.enabled == false)
		wxMessageBox(wxString::Format(
				wxT("Rule Syntax Error: " + newRule.ruleObject + " Please correct the mistake before continuing.")
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
	//Update userlist object.
	userlist.rules.push_back(newRule);
	//Now update RuleList text.
	Rules.push_back(GetRuleText(userlist.rules.size()-1));
	if (userlist.rules.back().enabled)
		RuleOrder.push_back(userlist.rules.size()-1);
	else
		RuleOrder.push_back(~(userlist.rules.size()-1));
	RulesList->Update();  //This doesn't work.
}

void UserRulesEditorFrame::OnRuleEdit(wxCommandEvent& event) {
	wxMessageDialog *dlg = new wxMessageDialog(this,
			wxT("Are you sure you want to save your changes to the selected rule?")
			, wxT("BOSS: User Rules Editor"), wxYES_NO);

	if (dlg->ShowModal() != wxID_YES)  //User has chosen not to save.
		return;
	else {  //User has chosen to save.
		int i = RulesList->GetSelection();
		Rule newRule = GetRuleFromForm();
		if (newRule.enabled == false)
			wxMessageBox(wxString::Format(
					wxT("Rule Syntax Error: " + newRule.ruleObject + " Please correct the mistake before continuing.")
				),
				wxT("BOSS: Error"),
				wxOK | wxICON_ERROR,
				NULL);
		//Update userlist object.
		userlist.rules.push_back(newRule);
		//Now update RuleList text.
		string text = GetRuleText(i);
		RulesList->SetString(i,text);
	}
}

void UserRulesEditorFrame::OnRuleDelete(wxCommandEvent& event) {
	wxMessageDialog *dlg = new wxMessageDialog(this,
			wxT("Are you sure you want to delete the selected rule?")
			, wxT("BOSS: User Rules Editor"), wxYES_NO);

	if (dlg->ShowModal() != wxID_YES)  //User has chosen not to delete.
		return;
	else {  //User has chosen to delete.
		int i = RulesList->GetSelection();
		userlist.rules.erase(userlist.rules.begin()+i);
		Rules.erase(Rules.begin()+i);
		RuleOrder.erase(RuleOrder.begin()+i);
		RulesList->Update();  //This doesn't work.
	}
}

void UserRulesEditorFrame::OnToggleRuleCheckbox(wxCommandEvent& event) {
	unsigned int i = event.GetInt();
	userlist.rules[i].enabled = RulesList->IsChecked(i);
	userlist.rules[i].enabled = false;
	wxMessageBox(wxString::Format(
			wxT("Error: "+IntToString(i)+" Scanning for plugins aborted. User Rules Editor cannot load.")
		),
		wxT("BOSS: Error"),
		wxOK | wxICON_ERROR,
		NULL);
}

void UserRulesEditorFrame::OnRuleSelection(wxCommandEvent& event) {
	int i = RulesList->GetSelection();
	string str = RulesList->GetString(i);
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
	RuleModBox->SetValue(userlist.rules[i].ruleObject);
	SortModBox->SetValue("");
	InsertModBox->SetValue("");
	size_t size = userlist.rules[i].lines.size();
	for (size_t j=0;j<size;j++) {
		if (userlist.rules[i].lines[j].key == BEFORE || userlist.rules[i].lines[j].key == AFTER) {
			SortModsCheckBox->SetValue(true);
			SortModOption->SetValue(true);
			SortModBox->Enable(true);
			BeforeAfterChoiceBox->Enable(true);
			SortModBox->SetValue(userlist.rules[i].lines[j].object);
			if (userlist.rules[i].lines[j].key == BEFORE)
				BeforeAfterChoiceBox->SetSelection(0);
			else
				BeforeAfterChoiceBox->SetSelection(1);
		} else if (userlist.rules[i].lines[j].key == TOP || userlist.rules[i].lines[j].key == BOTTOM) {
			SortModsCheckBox->SetValue(true);
			InsertModOption->SetValue(true);
			TopBottomChoiceBox->Enable(true);
			InsertModBox->Enable(true);
			InsertModBox->SetValue(userlist.rules[i].lines[j].object);
			if (userlist.rules[i].lines[j].key == TOP)
				TopBottomChoiceBox->SetSelection(0);
			else
				TopBottomChoiceBox->SetSelection(1);
		} else if (userlist.rules[i].lines[j].key == APPEND || userlist.rules[i].lines[j].key == REPLACE) {
			AddMessagesCheckBox->SetValue(true);
			NewModMessagesBox->Enable(true);
			ReplaceMessagesCheckBox->Enable(true);
			if (userlist.rules[i].lines[j].key == REPLACE)
				ReplaceMessagesCheckBox->SetValue(true);
			messages += userlist.rules[i].lines[j].object + "\n";
		}
	}
	NewModMessagesBox->SetValue(messages);
}

void UserRulesEditorFrame::OnRuleOrderChange(wxCommandEvent& event) {
	int i = RulesList->GetSelection();
	if (event.GetId() == BUTTON_MoveRuleUp && i != 0) {
		if (!RulesList->MoveCurrentUp())
			LOG_ERROR("Could not move rule %i up.", i);
		Rule selectedRule = userlist.rules[i];
		userlist.rules.erase(userlist.rules.begin()+i);
		userlist.rules.insert(userlist.rules.begin()+i-1,selectedRule);
	} else if (event.GetId() == BUTTON_MoveRuleDown && i != userlist.rules.size()-1) {
		if (!RulesList->MoveCurrentDown())
			LOG_ERROR("Could not move rule %i down.", i);
		Rule selectedRule = userlist.rules[i];
		userlist.rules.erase(userlist.rules.begin()+i);
		userlist.rules.insert(userlist.rules.begin()+i+1,selectedRule);
	}
}

void UserRulesEditorFrame::LoadLists() {
	ItemList modlist;
	size_t size;

	///////////////
	// Modlist
	///////////////

	//Need to parse userlist, masterlist and build modlist.
	try {
		modlist.Load(data_path);
	} catch (boss_error e) {
		throw boss_error(BOSS_ERROR_GUI_WINDOW_INIT_FAIL, "User Rules Editor", e.getString());
	}

	size = modlist.items.size();
	for (size_t i=0;i<size;i++)
		ModlistMods.push_back(modlist.items[i].name.string());

	////////////////
	// Masterlist
	////////////////

	//Parse masterlist/modlist backup into data structure.
	LOG_INFO("Starting to parse sorting file: %s", masterlist_path.string().c_str());
	try {
		masterlist.Load(masterlist_path);
	} catch (boss_error e) {
		throw boss_error(BOSS_ERROR_GUI_WINDOW_INIT_FAIL, "User Rules Editor", e.getString());
	}



	////////////////
	// Userlist
	////////////////

	LOG_INFO("Starting to parse userlist.");
	try {
		userlist.Load(userlist_path);
	} catch (boss_error e) {
		userlist.rules.clear();
		LOG_ERROR("Error: %s", e.getString().c_str());
		throw boss_error(BOSS_ERROR_GUI_WINDOW_INIT_FAIL, "User Rules Editor", e.getString());
	}

	//Now disable any ADD rules with rule mods that are in the masterlist.
	size = userlist.rules.size();
	for (size_t i=0;i<size;i++) {
		vector<Item>::iterator pos = masterlist.FindItem(userlist.rules[i].ruleObject);
		if (pos < masterlist.lastRecognisedPos && pos != masterlist.items.end())  //Mod in masterlist.
			userlist.rules[i].enabled = false;
	}

	size = userlist.rules.size();
	for (size_t i=0;i<size;i++) {
		string text = GetRuleText(i);
		if (userlist.rules[i].enabled)
			RuleOrder.push_back(i);
		else
			RuleOrder.push_back(~i);
		Rules.push_back(text);
	}
}

string UserRulesEditorFrame::GetRuleText(int i) {
	string text = "";
	bool hasAddedMessages = false;
	size_t linesSize = userlist.rules[i].lines.size();
	for (size_t j=0;j<linesSize;j++) {
		if (userlist.rules[i].lines[j].key == BEFORE)
			text = "Sort \"" + userlist.rules[i].ruleObject + "\" before \"" + userlist.rules[i].lines[j].object + "\"\n";
		else if (userlist.rules[i].lines[j].key == AFTER)
			text = "Sort \"" + userlist.rules[i].ruleObject + "\" after \"" + userlist.rules[i].lines[j].object + "\"\n";
		else if (userlist.rules[i].lines[j].key == TOP)
			text = "Insert \"" + userlist.rules[i].ruleObject + "\" at the top of \"" + userlist.rules[i].lines[j].object + "\"\n";
		else if (userlist.rules[i].lines[j].key == BOTTOM)
			text = "Insert \"" + userlist.rules[i].ruleObject + "\" at the bottom of \"" + userlist.rules[i].lines[j].object + "\"\n";
		else if (userlist.rules[i].lines[j].key == APPEND) {
			if (!hasAddedMessages)
				text += "Add the following messages to \"" + userlist.rules[i].ruleObject + "\":\n";
			text += "  " + userlist.rules[i].lines[j].object + "\n";
			hasAddedMessages = true;
		} else if (userlist.rules[i].lines[j].key == REPLACE) {
			text += "Replace the messages attached to \"" + userlist.rules[i].ruleObject + "\" with:\n";
			text += "  " + userlist.rules[i].lines[j].object + "\n";
		}
	}
	return text;
}

Rule UserRulesEditorFrame::GetRuleFromForm() {
	Rule newRule;
	//First validate.
	//Calling functions need to check for an enabled = false; rule as a failure.
	//Failure description is given in ruleObject.
	if (Item(string(RuleModBox->GetValue())).IsPlugin()) {
		if (SortModOption->GetValue() && !Item(string(SortModBox->GetValue())).IsPlugin()) {  //Sort object is a group. Error.
				newRule.enabled = false;
				newRule.ruleObject = "Cannot sort a plugin relative to a group.";
				return newRule;
		} else if (Item(string(InsertModBox->GetValue())).IsPlugin()) {  //Inserting into a mod. Error.
			newRule.enabled = false;
			newRule.ruleObject = "Cannot insert into a plugin.";
			return newRule;
		}
		if (AddMessagesCheckBox->IsChecked() && NewModMessagesBox->IsEmpty()) {  //Can't add no messages. Error.
			newRule.enabled = false;
			newRule.ruleObject = "Cannot add messages when none are given.";
			return newRule;
		}
	} else {  //Rule object is a group.
		if (SortModOption->GetValue() && Item(string(SortModBox->GetValue())).IsPlugin()) {  //Sort object is a plugin. Error.
				newRule.enabled = false;
				newRule.ruleObject = "Cannot sort a group relative to a plugin.";
				return newRule;
		} else {  //Can't insert groups. Error.
			newRule.enabled = false;
			newRule.ruleObject = "Cannot insert groups.";
			return newRule;
		}
		if (AddMessagesCheckBox->IsChecked()) {  //Can't add messages to a group. Error.
			newRule.enabled = false;
			newRule.ruleObject = "Cannot add messages to groups.";
			return newRule;
		}
	}

	newRule.enabled = true;
	newRule.ruleObject = RuleModBox->GetValue();
	if (!SortModsCheckBox->IsChecked())
		newRule.ruleKey = FOR;
	else {
		vector<Item>::iterator pos = masterlist.FindItem(newRule.ruleObject);
		if (pos < masterlist.lastRecognisedPos && pos != masterlist.items.end())  //Mod in masterlist.
			newRule.ruleKey = OVERRIDE;
		else
			newRule.ruleKey = ADD;
		
		if (SortModOption->GetValue()) {
			RuleLine newLine;
			newLine.object = SortModBox->GetValue();
			if (BeforeAfterChoiceBox->GetSelection() == 0)
				newLine.key = BEFORE;
			else
				newLine.key = AFTER;
			newRule.lines.push_back(newLine);
		} else if (InsertModOption->GetValue()) {
			RuleLine newLine;
			newLine.object = InsertModBox->GetValue();
			if (TopBottomChoiceBox->GetSelection() == 0)
				newLine.key = TOP;
			else
				newLine.key = BOTTOM;
			newRule.lines.push_back(newLine);
		}
	}

	//Now add message lines.
	if (AddMessagesCheckBox->IsChecked()) {
		RuleLine newLine;
		string messages = NewModMessagesBox->GetValue();
		size_t pos1 = 0, pos2 = string::npos;
		pos2 = messages.find("\n",pos1);
		while (pos2 != string::npos) {
			newLine.object = trim_copy(messages.substr(pos1,pos2-pos1));
			if (pos1 == 0 && ReplaceMessagesCheckBox->IsChecked())
				newLine.key = REPLACE;
			else
				newLine.key = APPEND;
			newRule.lines.push_back(newLine);

			if (pos2 == messages.length()-1)
				break;
			pos1 = pos2 + 1;
			pos2 = messages.find("\n",pos1);
		}
	}

	return newRule;
}