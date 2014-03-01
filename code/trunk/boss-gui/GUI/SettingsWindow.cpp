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

#include "GUI/SettingsWindow.h"

BEGIN_EVENT_TABLE( SettingsFrame, wxFrame )
	EVT_BUTTON ( OPTION_OKExitSettings, SettingsFrame::OnOKQuit)
	EVT_BUTTON ( OPTION_CancelExitSettings, SettingsFrame::OnCancelQuit)
END_EVENT_TABLE()

using namespace boss;
using namespace std;

SettingsFrame::SettingsFrame(const wxString title, wxFrame *parent) : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxMINIMIZE_BOX|wxSYSTEM_MENU|wxCAPTION|wxCLOSE_BOX|wxCLIP_CHILDREN) {

	wxString DebugVerbosity[] = {
        translate("Standard (0)"),
        translate("Level 1"),
		translate("Level 2"),
		translate("Level 3")
    };

	wxString Game[] = {
		translate("Autodetect"),
		wxT("Oblivion"),
		wxT("Nehrim"),
		wxT("Skyrim"),
		wxT("Fallout 3"),
		wxT("Fallout: New Vegas"),
		wxT("Morrowind")
	};

	wxString Language[] = {
		wxT("English"),
		wxString("Español", wxConvUTF8),
		wxT("Deutsch"),
		wxString("Русский", wxConvUTF8),
		wxString("简体中文", wxConvUTF8)
	};

	//Set up stuff in the frame.
	SetBackgroundColour(wxColour(255,255,255));

	TabHolder = new wxNotebook(this,wxID_ANY);

	wxSizerFlags leftItem(1);
	leftItem.Border(wxALL, 10).Expand().Left();

	wxSizerFlags rightItem(1);
	rightItem.Border(wxALL, 10).Expand().Right();

	wxSizerFlags wholeItem(0);
	wholeItem.Border(wxALL, 10);

	//Create General Settings tab.
	GeneralTab = new wxPanel(TabHolder);
	wxBoxSizer *GeneralTabSizer = new wxBoxSizer(wxVERTICAL);

	wxGridSizer *GeneralGridSizer = new wxGridSizer(2, 5, 5);
	
	//wxBoxSizer *gameBox = new wxBoxSizer(wxHORIZONTAL);
	GeneralGridSizer->Add(new wxStaticText(GeneralTab, wxID_ANY, translate("Default Game:")), leftItem);
	GeneralGridSizer->Add(GameChoice = new wxChoice(GeneralTab, wxID_ANY, wxDefaultPosition, wxDefaultSize, 7, Game), rightItem);
	//GeneralTabSizer->Add(gameBox, wholeItem);
	//wxBoxSizer *languageBox = new wxBoxSizer(wxHORIZONTAL);
	GeneralGridSizer->Add(new wxStaticText(GeneralTab, wxID_ANY, translate("Language:")), leftItem);
	GeneralGridSizer->Add(LanguageChoice = new wxChoice(GeneralTab, wxID_ANY, wxDefaultPosition, wxDefaultSize, 5, Language), rightItem);
	GeneralTabSizer->Add(GeneralGridSizer);
	GeneralTabSizer->Add(StartupUpdateCheckBox = new wxCheckBox(GeneralTab,wxID_ANY,translate("Check for BOSS updates on startup")), wholeItem);
	GeneralTabSizer->Add(UseUserRuleManagerBox = new wxCheckBox(GeneralTab,wxID_ANY,translate("Use User Rules Manager")), wholeItem);
	GeneralTabSizer->Add(CloseGUIAfterRunningBox = new wxCheckBox(GeneralTab,wxID_ANY,translate("Close the GUI after running BOSS")), wholeItem);

	wxString text = translate("Language settings will be applied after the BOSS GUI is restarted.");
	GeneralTabSizer->Add(new wxStaticText(GeneralTab, wxID_ANY, text), 1, wxEXPAND|wxALL, 10);
	
	GeneralTab->SetSizer(GeneralTabSizer);

	//Create Internet Settings tab.
	InternetTab = new wxPanel(TabHolder);
	wxGridSizer *InternetTabSizer = new wxGridSizer(2, 5, 5);
	
	InternetTabSizer->Add(new wxStaticText(InternetTab, wxID_ANY, translate("Proxy Hostname:")), leftItem);
	InternetTabSizer->Add(ProxyHostBox = new wxTextCtrl(InternetTab,wxID_ANY), rightItem);

	InternetTabSizer->Add(new wxStaticText(InternetTab, wxID_ANY, translate("Proxy Port Number:")), leftItem);
	InternetTabSizer->Add(ProxyPortBox = new wxTextCtrl(InternetTab,wxID_ANY), rightItem);

	InternetTabSizer->Add(new wxStaticText(InternetTab, wxID_ANY, translate("Proxy Username:")), leftItem);
	InternetTabSizer->Add(ProxyUserBox = new wxTextCtrl(InternetTab,wxID_ANY), rightItem);

	InternetTabSizer->Add(new wxStaticText(InternetTab, wxID_ANY, translate("Proxy Password:")), leftItem);
	InternetTabSizer->Add(ProxyPasswdBox = new wxTextCtrl(InternetTab,wxID_ANY), rightItem);

	InternetTabSizer->AddStretchSpacer();

	InternetTab->SetSizerAndFit(InternetTabSizer);

	//Create Debugging Settings tab.
	DebugTab = new wxPanel(TabHolder);
	wxBoxSizer *DebugTabSizer = new wxBoxSizer(wxVERTICAL);

	wxGridSizer *DebugGridSizer = new wxGridSizer(2, 5, 5);
	DebugGridSizer->Add(new wxStaticText(DebugTab, wxID_ANY, translate("Debug Output Verbosity:")), leftItem);
	DebugGridSizer->Add(DebugVerbosityChoice = new wxChoice(DebugTab, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, DebugVerbosity), rightItem);
	DebugTabSizer->Add(DebugGridSizer);
	DebugTabSizer->Add(DebugSourceReferencesBox = new wxCheckBox(DebugTab,wxID_ANY, translate("Include Source Code References")), wholeItem);
	DebugTabSizer->Add(LogDebugOutputBox = new wxCheckBox(DebugTab,wxID_ANY, translate("Log Debug Output")), wholeItem);

	DebugTab->SetSizer(DebugTabSizer);

	//Attach all pages.
	TabHolder->AddPage(GeneralTab, translate("General"), true);
	TabHolder->AddPage(InternetTab, translate("Internet"));
	TabHolder->AddPage(DebugTab, translate("Debugging"));
	
	//Need to add 'OK' and 'Cancel' buttons.
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxButton(this, OPTION_OKExitSettings, translate("OK"), wxDefaultPosition, wxSize(70, 30)));
	hbox->Add(new wxButton(this, OPTION_CancelExitSettings, translate("Cancel"), wxDefaultPosition, wxSize(70, 30)), 0, wxLEFT, 20);

	//Now add TabHolder and OK button to window sizer.
	wxBoxSizer *bigBox = new wxBoxSizer(wxVERTICAL);
	bigBox->Add(TabHolder, 1, wxEXPAND);
	bigBox->Add(hbox, 0, wxCENTER|wxALL, 10);

	//Initialise options with values. For checkboxes, they are off by default.
	SetDefaultValues();

	//Tooltips.
	LogDebugOutputBox->SetToolTip(translate("The output is logged to the BOSSDebugLog.txt file"));
	DebugSourceReferencesBox->SetToolTip(translate("Adds source code references to command line output."));
	DebugVerbosityChoice->SetToolTip(translate("The higher the verbosity level, the more information is outputted to the command line."));
	
	//Now set the layout and sizes.
	SetSizerAndFit(bigBox);
}

void SettingsFrame::SetDefaultValues() {
	//General Settings
	if (gl_do_startup_update_check)
		StartupUpdateCheckBox->SetValue(true);
	if (gl_use_user_rules_manager)
		UseUserRuleManagerBox->SetValue(true);
	if (gl_close_gui_after_sorting)
		CloseGUIAfterRunningBox->SetValue(true);

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
	else if (gl_game == MORROWIND)
		GameChoice->SetSelection(6);
	
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

	//Internet Settings
	ProxyHostBox->SetValue(wxString(gl_proxy_host.c_str(), wxConvUTF8));

	ProxyPortBox->SetValue(wxString::Format(wxT("%i"),gl_proxy_port));

	ProxyUserBox->SetValue(wxString(gl_proxy_user.c_str(), wxConvUTF8));

	ProxyPasswdBox->SetValue(wxString(gl_proxy_passwd.c_str(), wxConvUTF8));

	//Debugging Settings
	if (gl_debug_with_source)
		DebugSourceReferencesBox->SetValue(true);

	if (gl_log_debug_output)
		LogDebugOutputBox->SetValue(true);

	if (gl_debug_verbosity == 0)
		DebugVerbosityChoice->SetSelection(0);
	else if (gl_debug_verbosity == 1)
		DebugVerbosityChoice->SetSelection(1);
	else if (gl_debug_verbosity == 2)
		DebugVerbosityChoice->SetSelection(2);
	else if (gl_debug_verbosity == 3)
		DebugVerbosityChoice->SetSelection(3);
}

void SettingsFrame::OnOKQuit(wxCommandEvent& event) {
	//Make sure the settings are saved.

	//General
	gl_do_startup_update_check = StartupUpdateCheckBox->IsChecked();
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
	case 6:
		gl_game = MORROWIND;
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

	//Network
	gl_proxy_host = ProxyHostBox->GetValue().ToUTF8();
	gl_proxy_port = wxAtoi(ProxyPortBox->GetValue().ToUTF8());
	gl_proxy_user = ProxyUserBox->GetValue().ToUTF8();
	gl_proxy_passwd = ProxyPasswdBox->GetValue().ToUTF8();

	//Debugging
	gl_debug_verbosity = DebugVerbosityChoice->GetSelection();
	gl_debug_with_source = DebugSourceReferencesBox->IsChecked();
	gl_log_debug_output = LogDebugOutputBox->IsChecked();

	//Also set the logger settings now.
	g_logger.setOriginTracking(gl_debug_with_source);
	g_logger.setVerbosity(static_cast<LogVerbosity>(LV_WARN + gl_debug_verbosity));
	if (gl_log_debug_output)
		g_logger.setStream(debug_log_path.string().c_str());

	this->Close();
}

void SettingsFrame::OnCancelQuit(wxCommandEvent& event) {
	this->Close();
}