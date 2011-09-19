/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "GUI/SettingsWindow.h"
#include "GUI/MainWindow.h"
#include "BOSS-Common.h"

BEGIN_EVENT_TABLE( SettingsFrame, wxFrame )
	EVT_TEXT ( TEXT_ProxyHost, SettingsFrame::OnProxyHostChange )
	EVT_TEXT ( TEXT_ProxyPort, SettingsFrame::OnProxyPortChange )
	EVT_BUTTON ( OPTION_ExitSettings, OnQuit)
	EVT_COMBOBOX ( DROPDOWN_ProxyType, SettingsFrame::OnProxyTypeChange )
	EVT_COMBOBOX ( DROPDOWN_Verbosity, SettingsFrame::OnVerbosityChange )
	EVT_CHECKBOX ( CHECKBOX_StartupUpdateCheck, SettingsFrame::OnStartupUpdateChange )
	EVT_CHECKBOX ( CHECKBOX_EnableDebug, SettingsFrame::OnDebugChange )
	EVT_CHECKBOX ( CHECKBOX_EnableLogging, SettingsFrame::OnLoggingChange )
END_EVENT_TABLE()

using namespace boss;
using namespace std;

SettingsFrame::SettingsFrame(const wxChar *title, wxFrame *parent, int x, int y, int width, int height) : wxFrame(parent, -1, title, wxPoint(x, y), wxSize(width, height)) {

	wxString ProxyTypes[] = {
        wxT("Direct (No Proxy)"),
        wxT("HTTP"),
		wxT("HTTP 1.0"),
		wxT("SOCKS 4"),
		wxT("SOCKS 4a"),
		wxT("SOCKS 5"),
		wxT("SOCKS 5h (proxy resolves hostname)")
    };

	wxString DebugVerbosity[] = {
        wxT("Standard (0)"),
        wxT("Level 1"),
		wxT("Level 2"),
		wxT("Level 3")
    };

	//Set up stuff in the frame.
	SetBackgroundColour(wxColour(255,255,255));

	//Contents in one big resizing box.
	wxBoxSizer *bigBox = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer *generalOptionsBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("General"));
	generalOptionsBox->Add(StartupUpdateCheckBox = new wxCheckBox(this,CHECKBOX_StartupUpdateCheck,wxT("Check for BOSS updates on GUI startup")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	bigBox->Add(generalOptionsBox, 0, wxALL, 20);

	wxStaticBoxSizer *debugOptionsBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Debugging"));
	wxBoxSizer *verbosityBox = new wxBoxSizer(wxHORIZONTAL);
	verbosityBox->Add(new wxStaticText(this, wxID_ANY, wxT("Debug Output Verbosity: ")), 1, wxLEFT | wxBOTTOM, 5);
	verbosityBox->Add(DebugVerbosityBox = new wxComboBox(this, DROPDOWN_Verbosity, DebugVerbosity[0], wxDefaultPosition, wxDefaultSize, 4, DebugVerbosity, wxCB_READONLY), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
	debugOptionsBox->Add(verbosityBox, 0, wxEXPAND, 0);
	debugOptionsBox->Add(DebugSourceReferencesBox = new wxCheckBox(this,CHECKBOX_EnableDebug, wxT("Include Source Code References")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	debugOptionsBox->Add(RecordDebugOutput = new wxCheckBox(this,CHECKBOX_EnableLogging, wxT("Record Debug Output")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	bigBox->Add(debugOptionsBox, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);

	wxStaticBoxSizer *proxyOptionsBox = new wxStaticBoxSizer(wxVERTICAL,this,wxT("Internet"));
	proxyOptionsBox->Add(new wxStaticText(this, wxID_ANY, wxT("These settings control BOSS's proxy support.\n")), 0, wxALL, 5);
	
	wxBoxSizer *proxyTypeSizer = new wxBoxSizer(wxHORIZONTAL);
	proxyTypeSizer->Add(ProxyTypeText = new wxStaticText(this, wxID_ANY, wxT("Proxy Type: ")), 1, wxLEFT | wxTOP | wxBOTTOM, 5);
	proxyTypeSizer->Add(ProxyTypeBox = new wxComboBox(this,DROPDOWN_ProxyType,ProxyTypes[0],wxDefaultPosition,wxDefaultSize,7,ProxyTypes,wxCB_READONLY), 0, wxALIGN_RIGHT | wxRIGHT | wxTOP | wxBOTTOM, 5);
	proxyOptionsBox->Add(proxyTypeSizer, 0, wxEXPAND, 0);
	
	wxBoxSizer *proxyHostSizer = new wxBoxSizer(wxHORIZONTAL);
	proxyHostSizer->Add(ProxyTypeText = new wxStaticText(this, wxID_ANY, wxT("Proxy Hostname:   ")), 1, wxLEFT | wxBOTTOM, 5);
	proxyHostSizer->Add(ProxyHostBox = new wxTextCtrl(this,TEXT_ProxyHost), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
	proxyOptionsBox->Add(proxyHostSizer, 0, wxEXPAND, 0);

	wxBoxSizer *proxyPortSizer = new wxBoxSizer(wxHORIZONTAL);
	proxyPortSizer->Add(ProxyTypeText = new wxStaticText(this, wxID_ANY, wxT("Proxy Port Number: ")), 1, wxLEFT | wxBOTTOM, 5);
	proxyPortSizer->Add(ProxyPortBox = new wxTextCtrl(this,TEXT_ProxyPort), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
	proxyOptionsBox->Add(proxyPortSizer, 0, wxEXPAND, 0);

	bigBox->Add(proxyOptionsBox, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);

	//Need to add an 'OK' button.
	wxButton *okButton = new wxButton(this, OPTION_ExitSettings, wxT("OK"), wxDefaultPosition, wxSize(70, 30));
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(okButton, 1);
	
	bigBox->Add(hbox, 0, wxALIGN_CENTER | wxBOTTOM, 10);

	RecordDebugOutput->SetToolTip(wxT("The output is logged to the BOSSDebugLog.txt file"));
	DebugSourceReferencesBox->SetToolTip(wxT("Adds source code references to command line output."));
	DebugVerbosityBox->SetToolTip(wxT("The higher the verbosity level, the more information is outputted to the command line."));

	//Initialise options with values.
	if (do_startup_update_check)
		StartupUpdateCheckBox->SetValue(true);
	else
		StartupUpdateCheckBox->SetValue(false);

	ProxyHostBox->SetValue(proxy_host);

	ProxyPortBox->SetValue(proxy_port);

	if (proxy_type == "direct")
		ProxyTypeBox->SetValue(ProxyTypes[0]);
	else if (proxy_type == "http")
		ProxyTypeBox->SetValue(ProxyTypes[1]);
	else if (proxy_type == "http1_0")
		ProxyTypeBox->SetValue(ProxyTypes[2]);
	else if (proxy_type == "socks4")
		ProxyTypeBox->SetValue(ProxyTypes[3]);
	else if (proxy_type == "socks4a")
		ProxyTypeBox->SetValue(ProxyTypes[4]);
	else if (proxy_type == "socks5")
		ProxyTypeBox->SetValue(ProxyTypes[5]);
	else if (proxy_type == "socks5h")
		ProxyTypeBox->SetValue(ProxyTypes[6]);

	if (debug)
		DebugSourceReferencesBox->SetValue(true);
	else
		DebugSourceReferencesBox->SetValue(false);

	if (record_debug_output)
		RecordDebugOutput->SetValue(true);
	else
		RecordDebugOutput->SetValue(false);

	if (verbosity == 0)
		DebugVerbosityBox->SetValue(DebugVerbosity[0]);
	else if (verbosity == 1)
		DebugVerbosityBox->SetValue(DebugVerbosity[1]);
	else if (verbosity == 2)
		DebugVerbosityBox->SetValue(DebugVerbosity[2]);
	else if (verbosity == 3)
		DebugVerbosityBox->SetValue(DebugVerbosity[3]);
	
	//Now set the layout and sizes.
	SetSizerAndFit(bigBox);
}

void SettingsFrame::OnQuit(wxCommandEvent& event) {
	this->Close();
}

void SettingsFrame::OnStartupUpdateChange(wxCommandEvent& event) {
	do_startup_update_check = event.IsChecked();
}

void SettingsFrame::OnProxyTypeChange(wxCommandEvent& event) {
	int i = event.GetInt();
	if (i == 0)
		proxy_type = "direct";
	else if (i == 1)
		proxy_type = "http";
	else if (i == 2)
		proxy_type = "http1_0";
	else if (i == 3)
		proxy_type = "socks4";
	else if (i == 4)
		proxy_type = "socks4a";
	else if (i == 5)
		proxy_type = "socks5";
	else if (i == 6)
		proxy_type = "socks5h";
	if (i == 0) {
		ProxyHostBox->Enable(false);
		ProxyPortBox->Enable(false);
	} else {
		ProxyHostBox->Enable(true);
		ProxyPortBox->Enable(true);
	}
}

void SettingsFrame::OnProxyHostChange(wxCommandEvent& event) {
	proxy_host = ProxyHostBox->GetValue();
}

void SettingsFrame::OnProxyPortChange(wxCommandEvent& event) {
	proxy_port = ProxyPortBox->GetValue();
}


void SettingsFrame::OnVerbosityChange(wxCommandEvent& event) {
	verbosity = event.GetInt();
}

void SettingsFrame::OnDebugChange(wxCommandEvent& event) {
	debug = event.IsChecked();
}

void SettingsFrame::OnLoggingChange(wxCommandEvent& event) {
	record_debug_output = event.IsChecked();
}