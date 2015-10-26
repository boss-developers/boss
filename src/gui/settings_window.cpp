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

#include "gui/settings_window.h"

#include <string>

#include "common/globals.h"
#include "gui/element_ids.h"
#include "support/logger.h"

using boss::SettingsFrame;

wxBEGIN_EVENT_TABLE(SettingsFrame, wxFrame)
	EVT_BUTTON(OPTION_OKExitSettings, SettingsFrame::OnOKQuit)
	EVT_BUTTON(OPTION_CancelExitSettings, SettingsFrame::OnCancelQuit)
wxEND_EVENT_TABLE()

namespace boss {

SettingsFrame::SettingsFrame(const wxString title, wxFrame *parent)
    : wxFrame(parent, wxID_ANY, title) {

	wxString DebugVerbosity[] = {translate("Standard (0)"),
	                             translate("Level 1"),
	                             translate("Level 2"),
	                             translate("Level 3")};

	wxString Game[] = {translate("Autodetect"), wxT("Oblivion"),
	                   wxT("Nehrim"), wxT("Skyrim"), wxT("Fallout 3"),
	                   wxT("Fallout: New Vegas")};

	wxString Language[] = {wxT("English"),
	                       wxString("Español", wxConvUTF8),
	                       wxT("Deutsch"),
	                       wxString("Русский", wxConvUTF8),
	                       wxString("简体中文", wxConvUTF8)};

	// Set up stuff in the frame.
	SetBackgroundColour(wxColour(255, 255, 255));

	GameChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition,
	                          wxDefaultSize, 6, Game);
	LanguageChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition,
	                              wxDefaultSize, 5, Language);
	DebugVerbosityChoice = new wxChoice(this, wxID_ANY,
	                                    wxDefaultPosition,
	                                    wxDefaultSize,
	                                    4, DebugVerbosity);
	UseUserRuleManagerBox = new wxCheckBox(this, wxID_ANY,
	                                       translate("Use User Rules Manager"));
	CloseGUIAfterRunningBox = new wxCheckBox(this, wxID_ANY,
	                                         translate("Close the GUI after running BOSS"));
	OblivionRepoURLTxt = new wxTextCtrl(this, wxID_ANY);
	NehrimRepoURLTxt = new wxTextCtrl(this, wxID_ANY);
	SkyrimRepoURLTxt = new wxTextCtrl(this, wxID_ANY);
	Fallout3RepoURLTxt = new wxTextCtrl(this, wxID_ANY);
	FalloutNVRepoURLTxt = new wxTextCtrl(this, wxID_ANY);

	wxSizerFlags leftItem(1);
	leftItem.Border(wxTOP | wxBOTTOM | wxRIGHT, 5).Expand().Left();

	wxSizerFlags rightItem(1);
	rightItem.Border(wxTOP | wxBOTTOM | wxLEFT, 5).Expand().Right();

	wxSizerFlags wholeItem(0);
	wholeItem.Border(wxALL, 10).Expand();

	// Set up layout.
	wxBoxSizer *bigBox = new wxBoxSizer(wxVERTICAL);

	wxGridSizer *GridSizer = new wxGridSizer(2, 0, 0);
	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Default Game:")), leftItem);
	GridSizer->Add(GameChoice, rightItem);
	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Language:")), leftItem);
	GridSizer->Add(LanguageChoice, rightItem);
	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Debug Output Verbosity:")), leftItem);
	GridSizer->Add(DebugVerbosityChoice, rightItem);
	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Oblivion Masterlist URL:")), leftItem);
	GridSizer->Add(OblivionRepoURLTxt, rightItem);
	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Nehrim Masterlist URL:")), leftItem);
	GridSizer->Add(NehrimRepoURLTxt, rightItem);
	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Skyrim Masterlist URL:")), leftItem);
	GridSizer->Add(SkyrimRepoURLTxt, rightItem);
	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Fallout 3 Masterlist URL:")), leftItem);
	GridSizer->Add(Fallout3RepoURLTxt, rightItem);
	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Falllout: New Vegas Masterlist URL:")), leftItem);
	GridSizer->Add(FalloutNVRepoURLTxt, rightItem);

	bigBox->Add(GridSizer, wholeItem);

	bigBox->Add(UseUserRuleManagerBox, wholeItem);
	bigBox->Add(CloseGUIAfterRunningBox, wholeItem);
	wxString text = translate("Language settings will be applied after the BOSS GUI is restarted.");
	bigBox->Add(new wxStaticText(this, wxID_ANY, text), 0, wxEXPAND | wxALL, 10);

	// Need to add 'OK' and 'Cancel' buttons.
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxButton(this, OPTION_OKExitSettings, translate("OK"), wxDefaultPosition, wxSize(70, 30)));
	hbox->Add(new wxButton(this, OPTION_CancelExitSettings, translate("Cancel"), wxDefaultPosition, wxSize(70, 30)), 0, wxLEFT, 20);
	bigBox->Add(hbox, 0, wxCENTER|wxALL, 10);

	// Initialise options with values. For checkboxes, they are off by default.
	SetDefaultValues();

	// Tooltips.
	DebugVerbosityChoice->SetToolTip(translate("The higher the verbosity level, the more information is outputted to BOSSDebugLog.txt."));

	// Now set the layout and sizes.
	SetSizerAndFit(bigBox);
}

void SettingsFrame::OnOKQuit(wxCommandEvent &event) {
	// Make sure the settings are saved.

	// General
	gl_use_user_rules_manager = UseUserRuleManagerBox->IsChecked();
	gl_close_gui_after_sorting = CloseGUIAfterRunningBox->IsChecked();
	switch (GameChoice->GetSelection()) {
		case 0:
			gl_game = AUTODETECT;
			break;
		case 1:
			gl_game = OBLIVION;
			break;
		case 2:
			gl_game = NEHRIM;
			break;
		case 3:
			gl_game = SKYRIM;
			break;
		case 4:
			gl_game = FALLOUT3;
			break;
		case 5:
			gl_game = FALLOUTNV;
			break;
	}
	switch (LanguageChoice->GetSelection()) {
		case 0:
			gl_language = ENGLISH;
			break;
		case 1:
			gl_language = SPANISH;
			break;
		case 2:
			gl_language = GERMAN;
			break;
		case 3:
			gl_language = RUSSIAN;
			break;
		case 4:
			gl_language = SIMPCHINESE;
			break;
	}

	// Debugging
	gl_debug_verbosity = DebugVerbosityChoice->GetSelection();

	// Also set the logger settings now.
	g_logger.setVerbosity(static_cast<LogVerbosity>(LV_WARN + gl_debug_verbosity));
	if (gl_debug_verbosity > 0)
		g_logger.setStream(debug_log_path.string().c_str());

	// Masterlist repository URLs
	gl_oblivion_repo_url = std::string(OblivionRepoURLTxt->GetValue().ToUTF8());
	gl_nehrim_repo_url = std::string(NehrimRepoURLTxt->GetValue().ToUTF8());
	gl_skyrim_repo_url = std::string(SkyrimRepoURLTxt->GetValue().ToUTF8());
	gl_fallout3_repo_url = std::string(Fallout3RepoURLTxt->GetValue().ToUTF8());
	gl_falloutnv_repo_url = std::string(FalloutNVRepoURLTxt->GetValue().ToUTF8());

	this->Close();
}

void SettingsFrame::OnCancelQuit(wxCommandEvent &event) {
	this->Close();
}

void SettingsFrame::SetDefaultValues() {
	// General Settings
	if (gl_use_user_rules_manager)
		UseUserRuleManagerBox->SetValue(true);
	if (gl_close_gui_after_sorting)
		CloseGUIAfterRunningBox->SetValue(true);

	// TODO(MCP): Look at converting this to a switch-statement
	if (gl_game == AUTODETECT)
		GameChoice->SetSelection(0);
	else if (gl_game == OBLIVION)
		GameChoice->SetSelection(1);
	else if (gl_game == NEHRIM)
		GameChoice->SetSelection(2);
	else if (gl_game == SKYRIM)
		GameChoice->SetSelection(3);
	else if (gl_game == FALLOUT3)
		GameChoice->SetSelection(4);
	else if (gl_game == FALLOUTNV)
		GameChoice->SetSelection(5);

	// TODO(MCP): Look at converting this to a switch-statement
	if (gl_language == ENGLISH)
		LanguageChoice->SetSelection(0);
	else if (gl_language == SPANISH)
		LanguageChoice->SetSelection(1);
	else if (gl_language == GERMAN)
		LanguageChoice->SetSelection(2);
	else if (gl_language == RUSSIAN)
		LanguageChoice->SetSelection(3);
	else if (gl_language == SIMPCHINESE)
		LanguageChoice->SetSelection(4);

	// MCP Note: Look at converting this to a switch-statement?
	// Debugging Settings
	if (gl_debug_verbosity == 0)
		DebugVerbosityChoice->SetSelection(0);
	else if (gl_debug_verbosity == 1)
		DebugVerbosityChoice->SetSelection(1);
	else if (gl_debug_verbosity == 2)
		DebugVerbosityChoice->SetSelection(2);
	else if (gl_debug_verbosity == 3)
		DebugVerbosityChoice->SetSelection(3);

	// Masterlist repository URL settings
	OblivionRepoURLTxt->SetValue(FromUTF8(gl_oblivion_repo_url));
	NehrimRepoURLTxt->SetValue(FromUTF8(gl_nehrim_repo_url));
	SkyrimRepoURLTxt->SetValue(FromUTF8(gl_skyrim_repo_url));
	Fallout3RepoURLTxt->SetValue(FromUTF8(gl_fallout3_repo_url));
	FalloutNVRepoURLTxt->SetValue(FromUTF8(gl_falloutnv_repo_url));
}

}  // namespace boss
