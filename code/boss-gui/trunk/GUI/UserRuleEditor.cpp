/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/


	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "GUI/UserRuleEditor.h"
#include "GUI/ElementIDs.h"

#include <boost/spirit/include/karma.hpp>
#include <boost/algorithm/string/case_conv.hpp>

BEGIN_EVENT_TABLE( UserRulesEditorFrame, wxFrame )
	EVT_BUTTON ( OPTION_OKExitSettings, UserRulesEditorFrame::OnOKQuit )
	EVT_BUTTON ( OPTION_CancelExitSettings, UserRulesEditorFrame::OnCancelQuit )
END_EVENT_TABLE()

using namespace boss;
using namespace std;
namespace karma = boost::spirit::karma;
namespace unicode = boost::spirit::unicode;

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

	wxArrayString *Rules = new wxArrayString();
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
		Rules->Add(text);
	}

	
	size = Modlist.size();
	wxString *ModlistMods;
	//for (size_t i=0;i<size;i++) {
	//	ModlistMods[i] = Modlist[i].name.string();
	//}

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
	//wxRearrangeCtrl *RulesList = new wxRearrangeCtrl(this, OPTION_RuleList, wxDefaultPosition, wxDefaultSize);
	////////Rule buttons
	wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
	buttons->Add(NewRuleButton = new wxButton(this, OPTION_NewRule, wxT("Create New Rule"), wxDefaultPosition, wxSize(70, 30)));
	buttons->Add(NewRuleButton = new wxButton(this, OPTION_EditRule, wxT("Save Edited Rule"), wxDefaultPosition, wxSize(70, 30)), 0, wxLEFT, 20);
	buttons->Add(NewRuleButton = new wxButton(this, OPTION_DeleteRule, wxT("Delete Rule"), wxDefaultPosition, wxSize(70, 30)), 0, wxLEFT, 20);
	rulesBox->Add(buttons);
	//Rule Creator/Editor
	
	wxStaticBoxSizer *ruleEditorBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Rule Creator/Editor"));  //Needs to go in an oulined box.
	wxBoxSizer *forBox = new wxBoxSizer(wxHORIZONTAL);
	forBox->Add(new wxStaticText(this, wxID_ANY, wxT("For")));
	forBox->Add(
	ruleEditorBox->Add(forBox);

	mainBox->Add(rulesBox);
	//////Second column.
	wxBoxSizer *listmessBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *listsBox = new wxBoxSizer(wxHORIZONTAL);
	////////Modlist column.
	wxBoxSizer *modlistBox = new wxBoxSizer(wxVERTICAL);
	modlistBox->Add(ModlistSearch = new wxTextCtrl(this,SEARCH_Modlist,wxT("Search Modlist"));
	modlistBox->Add(InstalledModsList = new wxListBox(this, LIST_Modlist, wxDefaultPosition, wxDefaultSize, ModlistMods->size(),ModlistMods));
	listsBox->Add(modlistBox);
	////////Masterlist column.
	wxBoxSizer *masterlistBox = new wxBoxSizer(wxVERTICAL);
	masterlistBox->Add(MasterlistSearch = new wxTextCtrl(this,SEARCH_Masterlist,wxT("Search Masterlist"));
	masterlistBox->Add(MasterlistModsList = new wxListBox(this, LIST_Modlist, wxDefaultPosition, wxDefaultSize, ModlistMods->size(),ModlistMods));
	listsBox->Add(masterlistBox);
	listmessBox->Add(listsBox);
	////////Mod Messages box
	wxBoxSizer *messageBox = new wxBoxSizer(wxVERTICAL);
	messageBox->Add(ModMessagesBox = new wxTextCtrl(this,TEXT_ModMessages,wxT(""),wxDefaultPosition,wxDefaultSize));
	
	listmessBox->Add(messageBox);
	mainBox->Add(listmessBox);
	bigBox->Add(mainBox);

	////Window buttons
	wxBoxSizer *mainButtonBox = new wxBoxSizer(wxHORIZONTAL);
	mainButtonBox->Add(new wxButton(this, OPTION_OKExitEditor, wxT("Save"), wxDefaultPosition, wxSize(70, 30)));
	mainButtonBox->Add(new wxButton(this, OPTION_CancelEditor, wxT("Cancel"), wxDefaultPosition, wxSize(70, 30)), 0, wxLEFT, 20);

	//Now add TabHolder and OK button to window sizer.
	bigBox->Add(mainButtonBox, 0, wxRIGHT|wxALL, 10);

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