/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/


	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "GUI/UserRuleEditor.h"
#include "GUI/ElementIDs.h"

#include <boost/algorithm/string/case_conv.hpp>

BEGIN_EVENT_TABLE( UserRulesEditorFrame, wxFrame )
	EVT_BUTTON ( OPTION_OKExitEditor, UserRulesEditorFrame::OnOKQuit )
	EVT_BUTTON ( OPTION_CancelExitEditor, UserRulesEditorFrame::OnCancelQuit )
	EVT_BUTTON ( BUTTON_MoveRuleUp, UserRulesEditorFrame::OnRuleOrderChange )
	EVT_BUTTON ( BUTTON_MoveRuleDown, UserRulesEditorFrame::OnRuleOrderChange )
	EVT_CHECKLISTBOX ( LIST_RuleList, UserRulesEditorFrame::OnToggleRuleCheckbox )
	EVT_LISTBOX ( LIST_Masterlist, UserRulesEditorFrame::OnSelectModInMasterlist )
	EVT_LISTBOX ( LIST_RuleList, UserRulesEditorFrame::OnRuleSelection )

END_EVENT_TABLE()

using namespace boss;
using namespace std;

using boost::algorithm::to_upper_copy;

UserRulesEditorFrame::UserRulesEditorFrame(const wxChar *title, wxFrame *parent) : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize) {

	//Need to parse userlist, masterlist and build modlist.
	BuildModlist(Modlist);

	//Parse masterlist/modlist backup into data structure.
	LOG_INFO("Starting to parse sorting file: %s", masterlist_path.string().c_str());
	try {
		parseMasterlist(masterlist_path,Masterlist);
	} catch (boss_error e) {
		wxMessageBox(wxString::Format(
				wxT("Error: "+e.getString()+" Scanning for plugins aborted. User Rules Editor cannot load.")
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
		return;
	}

	//Check if parsing failed. If so, exit with errors.
	if (!masterlistErrorBuffer.empty()) {
		size_t size = masterlistErrorBuffer.size();
		for (size_t i=0; i<size; i++)  //Print parser error messages.
			Output(masterlistErrorBuffer[i]);
		wxMessageBox(wxString::Format(
				wxT("Error: "+masterlistErrorBuffer[0]+" Masterlist parsing aborted. User Rules Editor cannot load.")
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
		return;
	}

	LOG_INFO("Starting to parse userlist.");
	try {
		bool parsed = parseUserlist(userlist_path,Userlist);
		if (!parsed)
			Userlist.clear();  //If userlist has parsing errors, empty it so no rules are applied.
	} catch (boss_error e) {
		LOG_ERROR("Error: %s", e.getString().c_str());
		wxMessageBox(wxString::Format(
				wxT("Error: "+e.getString()+" Userlist parsing aborted. User Rules Editor cannot load existing rules.")
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
	}

	//Trim down masterlist.
	size_t lastRec = BuildWorkingModlist(Modlist,Masterlist,Userlist);

	//Now disable any ADD rules with rule mods that are in the masterlist.
	size_t size = Userlist.size();
	for (size_t i=0;i<size;i++) {
		size_t pos = GetModPos(Masterlist,Userlist[i].ruleObject);
		if (pos < lastRec && pos != (size_t)-1)  //Mod in masterlist.
			Userlist[i].enabled = false;
	}

	///////////////////////
	// UI Stuff
	///////////////////////

	//Some variable setup.
	wxString BeforeAfter[] = {
        wxT("BEFORE"),
        wxT("AFTER")
    };

	wxString TopBottom[] = {
        wxT("TOP"),
        wxT("BOTTOM")
    };

	wxArrayString Rules;
	wxArrayInt RuleOrder;
	size = Userlist.size();
	for (size_t i=0;i<size;i++) {
		string text;
		bool hasAddedMessages = false;
		size_t linesSize = Userlist[i].lines.size();
		for (size_t j=0;j<linesSize;j++) {
			if (Userlist[i].lines[j].key == BEFORE)
				text = "Sort \"" + Userlist[i].ruleObject + "\" before \"" + Userlist[i].lines[j].object + "\"\n";
			else if (Userlist[i].lines[j].key == AFTER)
				text = "Sort \"" + Userlist[i].ruleObject + "\" after \"" + Userlist[i].lines[j].object + "\"\n";
			else if (Userlist[i].lines[j].key == TOP)
				text = "Insert \"" + Userlist[i].ruleObject + "\" at the top of \"" + Userlist[i].lines[j].object + "\"\n";
			else if (Userlist[i].lines[j].key == BOTTOM)
				text = "Insert \"" + Userlist[i].ruleObject + "\" at the bottom of \"" + Userlist[i].lines[j].object + "\"\n";
			else if (Userlist[i].lines[j].key == APPEND) {
				if (!hasAddedMessages)
					text += "Add the following messages to \"" + Userlist[i].ruleObject + "\":\n";
				text += "  " + Userlist[i].lines[j].object + "\n";
			} else if (Userlist[i].lines[j].key == REPLACE) {
				text += "Replace the messages attached to \"" + Userlist[i].ruleObject + "\" with:\n";
				text += "  " + Userlist[i].lines[j].object + "\n";
			}

		}
		RuleOrder.push_back(i);
		Rules.push_back(text);
	}

	
	size = Modlist.size();
	wxArrayString ModlistMods;
	for (size_t i=0;i<size;i++) {
		if (Modlist[i].type == MOD)
			ModlistMods.push_back(Modlist[i].name.string());
	}

	size = Masterlist.size();
	wxArrayString MasterlistMods;
	for (size_t i=0;i<size;i++) {
		if (Masterlist[i].type == MOD)
			MasterlistMods.push_back(Masterlist[i].name.string());
	}

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
	buttons->Add(NewRuleButton = new wxButton(this, OPTION_NewRule, wxT("Create New Rule"), wxDefaultPosition, wxDefaultSize));
	buttons->Add(NewRuleButton = new wxButton(this, OPTION_EditRule, wxT("Save Edited Rule"), wxDefaultPosition, wxDefaultSize), 0, wxLEFT, 20);
	buttons->Add(NewRuleButton = new wxButton(this, OPTION_DeleteRule, wxT("Delete Rule"), wxDefaultPosition, wxDefaultSize), 0, wxLEFT, 20);
	rulesBox->Add(buttons, 0, wxALIGN_RIGHT|wxALL, 10);
	//////Second column.
	wxBoxSizer *listmessBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *listsBox = new wxBoxSizer(wxHORIZONTAL);
	////////Modlist column.
	wxStaticBoxSizer *modlistBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Installed Mods"));
	modlistBox->Add(ModlistSearch = new wxSearchCtrl(this, SEARCH_Modlist));
	modlistBox->Add(InstalledModsList = new wxListBox(this, LIST_Modlist, wxDefaultPosition, wxDefaultSize, ModlistMods));
	listsBox->Add(modlistBox);
	////////Masterlist column.
	wxStaticBoxSizer *masterlistBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Masterlist"));
	masterlistBox->Add(MasterlistSearch = new wxSearchCtrl(this, SEARCH_Masterlist));
	masterlistBox->Add(MasterlistModsList = new wxListBox(this, LIST_Masterlist, wxDefaultPosition, wxDefaultSize, MasterlistMods));
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
	mainButtonBox->Add(new wxButton(this, OPTION_OKExitEditor, wxT("Save"), wxDefaultPosition, wxDefaultSize));
	mainButtonBox->Add(new wxButton(this, OPTION_CancelExitEditor, wxT("Cancel"), wxDefaultPosition, wxDefaultSize), 0, wxLEFT, 20);

	//Now add TabHolder and OK button to window sizer.
	bigBox->Add(mainButtonBox, 0, wxALIGN_RIGHT|wxALL, 10);

	//Now set the layout and sizes.
	SetSizerAndFit(bigBox);
}

void UserRulesEditorFrame::OnOKQuit(wxCommandEvent& event) {
	ofstream outFile(userlist_path.c_str(),ios_base::trunc);

	size_t size = Userlist.size();
	for (size_t i=0;i<size;i++) {
		if (!Userlist[i].enabled)
			outFile << "DISABLE ";
		outFile << to_upper_copy(KeyToString(Userlist[i].ruleKey)) << ": " << Userlist[i].ruleObject << endl;
		size_t linesSize = Userlist[i].lines.size();
		for (size_t j=0;j<linesSize;j++) {
			outFile << to_upper_copy(KeyToString(Userlist[i].lines[j].key)) << ": " << Userlist[i].lines[j].object << endl;
		}
		outFile << endl;
	}


	outFile.close();

	this->Close();
}

void UserRulesEditorFrame::OnCancelQuit(wxCommandEvent& event) {
	this->Close();
}

void UserRulesEditorFrame::OnSearchModlist(wxCommandEvent& event) {

}

void UserRulesEditorFrame::OnSearchMasterlist(wxCommandEvent& event) {

}

void UserRulesEditorFrame::OnSelectModInMasterlist(wxCommandEvent& event) {

}

void UserRulesEditorFrame::OnSortingCheckToggle(wxCommandEvent& event) {

}

void UserRulesEditorFrame::OnMessageAddToggle(wxCommandEvent& event) {

}

void UserRulesEditorFrame::OnSortSelection(wxCommandEvent& event) {

}

void UserRulesEditorFrame::OnInsertSelection(wxCommandEvent& event) {

}

void UserRulesEditorFrame::OnRuleCreate(wxCommandEvent& event) {

}

void UserRulesEditorFrame::OnRuleEdit(wxCommandEvent& event) {

}

void UserRulesEditorFrame::OnRuleDelete(wxCommandEvent& event) {

}

void UserRulesEditorFrame::OnToggleRuleCheckbox(wxCommandEvent& event) {

}

void UserRulesEditorFrame::OnRuleSelection(wxCommandEvent& event) {

}

void UserRulesEditorFrame::OnRuleOrderChange(wxCommandEvent& event) {

}