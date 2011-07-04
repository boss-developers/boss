/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/


	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/


#include "boost/exception/get_error_info.hpp"
#include <boost/algorithm/string.hpp>

#include "GUI/gui.h"
#include "Parsing/parser.h"
#include "GUI/updater.h"
#include "Helpers/helpers.h"
#include <string>

BEGIN_EVENT_TABLE ( MainFrame, wxFrame )
	EVT_IDLE( MainFrame::CheckForUpdate )
	EVT_CLOSE (MainFrame::OnClose )
	EVT_MENU ( MENU_Quit, MainFrame::OnQuit )
	EVT_MENU ( OPTION_OpenUserlist, MainFrame::OnOpenFile )
	EVT_MENU ( OPTION_OpenBOSSlog, MainFrame::OnOpenFile )
	EVT_MENU ( OPTION_Run, MainFrame::OnRunBOSS )
	EVT_MENU ( MENU_OpenMReadMe, MainFrame::OnOpenFile )
	EVT_MENU ( MENU_OpenURReadMe, MainFrame::OnOpenFile )
	EVT_MENU ( OPTION_CheckForUpdates, MainFrame::OnUpdateCheck )
	EVT_MENU ( MENU_ShowAbout, MainFrame::OnAbout )
	EVT_MENU ( MENU_ShowSettings, MainFrame::OnOpenSettings )
	EVT_BUTTON ( OPTION_Run, MainFrame::OnRunBOSS )
	EVT_BUTTON ( OPTION_OpenUserlist, MainFrame::OnOpenFile )
	EVT_BUTTON ( OPTION_OpenBOSSlog, MainFrame::OnOpenFile )
	EVT_BUTTON ( OPTION_CheckForUpdates, MainFrame::OnUpdateCheck )
	EVT_COMBOBOX ( DROPDOWN_LogFormat, MainFrame::OnFormatChange )
	EVT_COMBOBOX ( DROPDOWN_Verbosity, MainFrame::OnVerbosityChange )
	EVT_COMBOBOX ( DROPDOWN_Game, MainFrame::OnGameChange )
	EVT_COMBOBOX ( DROPDOWN_Revert, MainFrame::OnRevertChange )
	EVT_CHECKBOX ( CHECKBOX_ShowBOSSlog, MainFrame::OnLogDisplayChange )
	EVT_CHECKBOX ( CHECKBOX_EnableDebug, MainFrame::OnDebugChange )
	EVT_CHECKBOX ( CHECKBOX_Update, MainFrame::OnUpdateChange )
	EVT_CHECKBOX ( CHECKBOX_SortEnableVersions, MainFrame::OnVersionDisplayChange )
	EVT_CHECKBOX ( CHECKBOX_SortEnableCRCs, MainFrame::OnCRCDisplayChange )
	EVT_CHECKBOX ( CHECKBOX_RevertEnableVersions, MainFrame::OnVersionDisplayChange )
	EVT_CHECKBOX ( CHECKBOX_RevertEnableCRCs, MainFrame::OnCRCDisplayChange )
	EVT_CHECKBOX ( CHECKBOX_EnableLogging, MainFrame::OnLoggingChange )
	EVT_CHECKBOX ( CHECKBOX_TrialRun, MainFrame::OnTrialRunChange )
	EVT_RADIOBUTTON ( RADIOBUTTON_SortOption, MainFrame::OnRunTypeChange )
	EVT_RADIOBUTTON ( RADIOBUTTON_UpdateOption, MainFrame::OnRunTypeChange )
	EVT_RADIOBUTTON ( RADIOBUTTON_UndoOption, MainFrame::OnRunTypeChange )
END_EVENT_TABLE()

BEGIN_EVENT_TABLE( SettingsFrame, wxFrame )
	EVT_TEXT ( TEXT_ProxyHost, SettingsFrame::OnProxyHostChange )
	EVT_TEXT ( TEXT_ProxyPort, SettingsFrame::OnProxyPortChange )
	EVT_BUTTON ( OPTION_ExitSettings, OnQuit)
	EVT_COMBOBOX ( DROPDOWN_ProxyType, SettingsFrame::OnProxyTypeChange )
	EVT_CHECKBOX ( CHECKBOX_StartupUpdateCheck, SettingsFrame::OnStartupUpdateChange )
END_EVENT_TABLE()

IMPLEMENT_APP(BossGUI)

using namespace boss;
using namespace std;
namespace fs = boost::filesystem;

bool CheckedForUpdate = false;		//To prevent the update checker looping thanks to the OnIdle event handler.

//Draws the main window when program starts.
bool BossGUI::OnInit() {
	//Set up variable defaults.
	if (fs::exists("BOSS.ini")) {
		if (!parseIni("BOSS.ini"))
			wxMessageBox(wxString::Format(
				wxT("Error: BOSS.ini parsing failed. Some or all of the GUI's options may not have been set correctly. Run BOSS to see the details of the failure.")
			),
			wxT("Error"),
			wxOK | wxICON_ERROR,
			NULL);
	} else {
		if (!GenerateIni())
			wxMessageBox(wxString::Format(
				wxT("Error: BOSS.ini generation failed. Ensure your BOSS folder is not read-only. None of the GUI's options will be saved.")
			),
			wxT("Error"),
			wxOK | wxICON_ERROR,
			NULL);
	}

	MainFrame *frame = new MainFrame(
		wxT("Better Oblivion Sorting Software GUI - " + GetGame()), 100, 100, 510, 370);

	frame->SetIcon(wxIconLocation("BOSS GUI.exe"));
	frame->Show(TRUE);
	SetTopWindow(frame);
	return true;
}

MainFrame::MainFrame(const wxChar *title, int x, int y, int width, int height) : wxFrame(NULL, -1, title, wxPoint(x, y), wxSize(width, height)) {

	//Some variable setup.
	wxString choices[] = {
        wxT("HTML"),
        wxT("Plain Text")
    };
	wxString choices2[] = {
        wxT("Standard (0)"),
        wxT("Level 1"),
		wxT("Level 2"),
		wxT("Level 3")
    };
	wxString choices3[] = {
		wxT("Autodetect"),
		wxT("Oblivion"),
		wxT("Fallout 3"),
		wxT("Nehrim"),
		wxT("Fallout: New Vegas")
	};
	wxString choices4[] = {
		wxT("No Undo"),
		wxT("Last Run"),
		wxT("2nd Last Run")
	};

	//Set up menu bar first.
    MenuBar = new wxMenuBar();
    // File Menu
    FileMenu = new wxMenu();
	FileMenu->Append(OPTION_OpenBOSSlog, _T("&Open BOSSlog"), _T("Opens your BOSSlog."));
    FileMenu->Append(OPTION_OpenUserlist, _T("&Edit Userlist"), _T("Opens your userlist in your default text editor."));
    FileMenu->Append(OPTION_Run, _T("&Run BOSS"), _T("Runs BOSS with the options you have chosen."));
    FileMenu->AppendSeparator();
    FileMenu->Append(MENU_Quit, _T("&Quit"), _T("Quit BOSS GUI."));
    MenuBar->Append(FileMenu, _T("&File"));
	//Edit Menu
	EditMenu = new wxMenu();
	EditMenu->Append(MENU_ShowSettings, _T("&Settings..."), _T("Opens the GUI settings window."));
	MenuBar->Append(EditMenu, _T("&Edit"));
    // About menu
    HelpMenu = new wxMenu();
	HelpMenu->Append(MENU_OpenMReadMe, _T("Open &Main ReadMe"), _T("Opens the main BOSS ReadMe in your default web browser."));
	HelpMenu->Append(MENU_OpenURReadMe, _T("Open &User Rules ReadMe"), _T("Opens the User Rules ReadMe in your default web browser."));
	HelpMenu->AppendSeparator();
	HelpMenu->Append(OPTION_CheckForUpdates, _T("&Check For Updates..."), _T("Checks for updates to BOSS."));
	HelpMenu->Append(MENU_ShowAbout, _T("&About BOSS GUI..."), _T("Shows information about BOSS GUI."));
    MenuBar->Append(HelpMenu, _T("&Help"));
    SetMenuBar(MenuBar);

	//Set up stuff in the frame.
	SetBackgroundColour(wxColour(255,255,255));

	//Contents in one big resizing box.
	wxBoxSizer *bigBox = new wxBoxSizer(wxHORIZONTAL);

	//Create first column box and add the output options to it.
	wxBoxSizer *columnBox = new wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer *outputOptionsBox = new wxStaticBoxSizer(wxVERTICAL, this, "Output Options");
	wxBoxSizer *formatBox = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *verbosityBox = new wxBoxSizer(wxHORIZONTAL);
	//Add stuff to output options sizer.
	outputOptionsBox->Add(ShowLogBox = new wxCheckBox(this,CHECKBOX_ShowBOSSlog, "Show BOSSlog On Completion"), 0, wxALL, 5);
	outputOptionsBox->Add(DebugBox = new wxCheckBox(this,CHECKBOX_EnableDebug, "Enable Debug Output"), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	outputOptionsBox->Add(LoggingBox = new wxCheckBox(this,CHECKBOX_EnableLogging, "Log Command Line Output"), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	formatBox->Add(new wxStaticText(this, wxID_ANY, "BOSSlog Format: "), 1, wxLEFT | wxBOTTOM, 5);
	formatBox->Add(FormatBox = new wxComboBox(this, DROPDOWN_LogFormat, choices[0], wxPoint(110,60), wxDefaultSize, 2, choices, wxCB_READONLY), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
	verbosityBox->Add(new wxStaticText(this, wxID_ANY, "Command Line Verbosity: "), 1, wxLEFT | wxBOTTOM, 5);
	verbosityBox->Add(VerbosityBox = new wxComboBox(this, DROPDOWN_Verbosity, choices2[0], wxDefaultPosition, wxDefaultSize, 4, choices2, wxCB_READONLY), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
	//Add the verbosityBox to its parent now to preserve layout.
	outputOptionsBox->Add(formatBox, 0, wxEXPAND, 0);
	outputOptionsBox->Add(verbosityBox, 0, wxEXPAND, 0);
	columnBox->Add(outputOptionsBox, 0, wxBOTTOM, 30);

	//Now add the main buttons to the first column.
	wxBoxSizer *buttonBox = new wxBoxSizer(wxVERTICAL);
	buttonBox->Add(OpenUserlistButton = new wxButton(this,OPTION_OpenUserlist, "Edit Userlist", wxDefaultPosition, wxSize(120,30)), 0, wxBOTTOM, 5);
	buttonBox->Add(RunBOSSButton = new wxButton(this,OPTION_Run, "Run BOSS", wxDefaultPosition, wxSize(120,30)));
	buttonBox->Add(OpenBOSSlogButton = new wxButton(this,OPTION_OpenBOSSlog, "View BOSSlog", wxDefaultPosition, wxSize(120,30)), 0, wxTOP, 5);
	columnBox->Add(buttonBox, 0, wxALIGN_CENTER, 20);

	//Add the first column to the big box.
	bigBox->Add(columnBox, 0, wxALL, 20);

	//The second column has a border.
	wxStaticBoxSizer *runOptionsBox = new wxStaticBoxSizer(wxVERTICAL, this, "Run Options");
	wxBoxSizer *sortBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *updateBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *undoBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *gameBox = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *revertBox = new wxBoxSizer(wxHORIZONTAL);

	//Run Options
	runOptionsBox->Add(SortOption = new wxRadioButton(this, RADIOBUTTON_SortOption, "Sort Mods"), 0, wxALL, 5);
	
	//Sort option stuff.
	sortBox->Add(UpdateBox = new wxCheckBox(this,CHECKBOX_Update, "Update Masterlist"), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	sortBox->Add(SortVersionBox = new wxCheckBox(this,CHECKBOX_SortEnableVersions, "Display Mod Versions"), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	sortBox->Add(SortCRCBox = new wxCheckBox(this,CHECKBOX_SortEnableCRCs, "Display File CRCs"), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	sortBox->Add(TrialRunBox = new wxCheckBox(this,CHECKBOX_TrialRun, "Perform Trial Run"), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	runOptionsBox->Add(sortBox, 0, wxLEFT | wxBOTTOM, 20);
	
	//Update only stuff.
	runOptionsBox->Add(UpdateOption = new wxRadioButton(this, RADIOBUTTON_UpdateOption, "Update Masterlist Only"), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	gameBox->Add(GameText = new wxStaticText(this, wxID_ANY, "Game: "), 1, wxLEFT | wxBOTTOM, 15);
	gameBox->Add(GameBox = new wxComboBox(this, DROPDOWN_Game, choices3[0], wxDefaultPosition, wxDefaultSize, 5, choices3, wxCB_READONLY), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
	updateBox->Add(gameBox, 0, wxEXPAND, 0);
	runOptionsBox->Add(updateBox, 0, wxEXPAND | wxLEFT | wxBOTTOM, 20);
	
	//Undo option stuff.
	runOptionsBox->Add(UndoOption = new wxRadioButton(this, RADIOBUTTON_UndoOption, "Undo Changes"), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	revertBox->Add(RevertText = new wxStaticText(this, wxID_ANY, "Undo Level: "), 1, wxLEFT | wxBOTTOM, 5);
	revertBox->Add(RevertBox = new wxComboBox(this, DROPDOWN_Revert, choices4[0], wxDefaultPosition, wxDefaultSize, 3, choices4, wxCB_READONLY), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
	undoBox->Add(revertBox, 0, wxEXPAND, 0);
	undoBox->Add(UndoVersionBox = new wxCheckBox(this,CHECKBOX_RevertEnableVersions, "Display Mod Versions"), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	undoBox->Add(UndoCRCBox = new wxCheckBox(this,CHECKBOX_RevertEnableCRCs, "Display File CRCs"), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	runOptionsBox->Add(undoBox, 0, wxEXPAND | wxLEFT | wxBOTTOM, 20);

	bigBox->Add(runOptionsBox, 0, wxTOP | wxRIGHT | wxBOTTOM, 20);


	//Tooltips
	LoggingBox->SetToolTip("The output is logged to the BOSSCommandLineLog.txt file");
	FormatBox->SetToolTip("This decides both the format of BOSSlog generated when you click the \"Run BOSS\" button and the BOSSlog format opened when you click the \"View BOSSlog\" button.");
	OpenBOSSlogButton->SetToolTip("The format of BOSSlog this opens is decided by the setting of the \"BOSSlog Format\" Output Option above.");
	DebugBox->SetToolTip("Adds source code references to command line output.");
	VerbosityBox->SetToolTip("The higher the verbosity level, the more information is outputted to the command line.");
	TrialRunBox->SetToolTip("Runs BOSS, simulating its changes to your load order, but doesn't actually reorder your mods.");

	//Set option values based on initialised variable values.
	RunBOSSButton->SetDefault();
	if (update)
		UpdateBox->SetValue(true);
	else
		UpdateBox->SetValue(false);
	if (silent)
		ShowLogBox->SetValue(false);
	else
		ShowLogBox->SetValue(true);
	if (debug)
		DebugBox->SetValue(true);
	else
		DebugBox->SetValue(false);
	if (trial_run)
		TrialRunBox->SetValue(true);
	else
		TrialRunBox->SetValue(false);
	if (logCL)
		LoggingBox->SetValue(true);
	else
		LoggingBox->SetValue(false);
	if (sort_show_CRCs)
		SortCRCBox->SetValue(true);
	else
		SortCRCBox->SetValue(false);
	if (sort_skip_version_parse)
		SortVersionBox->SetValue(false);
	else
		SortVersionBox->SetValue(true);
	if (revert_show_CRCs)
		UndoCRCBox->SetValue(true);
	else
		UndoCRCBox->SetValue(false);
	if (revert_skip_version_parse)
		UndoVersionBox->SetValue(false);
	else
		UndoVersionBox->SetValue(true);
	if (game == 0)
		GameBox->SetValue(choices3[0]);
	else if (game == 1)
		GameBox->SetValue(choices3[1]);
	else if (game == 2)
		GameBox->SetValue(choices3[2]);
	else if (game == 3)
		GameBox->SetValue(choices3[3]);
	else if (game == 4)
		GameBox->SetValue(choices3[4]);
	if (revert == 0)
		RevertBox->SetValue(choices4[0]);
	else if (revert == 1)
		RevertBox->SetValue(choices4[1]);
	else if (revert == 2)
		RevertBox->SetValue(choices4[2]);
	if (verbosity == 0)
		VerbosityBox->SetValue(choices2[0]);
	else if (verbosity == 1)
		VerbosityBox->SetValue(choices2[1]);
	else if (verbosity == 2)
		VerbosityBox->SetValue(choices2[2]);
	else if (verbosity == 3)
		VerbosityBox->SetValue(choices2[3]);
	if (log_format == "html")
		FormatBox->SetValue(choices[0]);
	else if (log_format == "text")
		FormatBox->SetValue(choices[1]);

	if (run_type == 1) {
		SortOption->SetValue(true);
		UpdateBox->Enable(true);
		SortVersionBox->Enable(true);
		SortCRCBox->Enable(true);
		TrialRunBox->Enable(true);
		GameText->Enable(false);
		GameBox->Enable(false);
		RevertText->Enable(false);
		RevertBox->Enable(false);
		UndoVersionBox->Enable(false);
		UndoCRCBox->Enable(false);
	} else if (run_type == 2) {
		UpdateOption->SetValue(true);
		UpdateBox->Enable(false);
		SortVersionBox->Enable(false);
		SortCRCBox->Enable(false);
		TrialRunBox->Enable(false);
		GameText->Enable(true);
		GameBox->Enable(true);
		RevertText->Enable(false);
		RevertBox->Enable(false);
		UndoVersionBox->Enable(false);
		UndoCRCBox->Enable(false);
	} else {
		UndoOption->SetValue(true);
		UpdateBox->Enable(false);
		SortVersionBox->Enable(false);
		SortCRCBox->Enable(false);
		TrialRunBox->Enable(false);
		GameText->Enable(false);
		GameBox->Enable(false);
		RevertText->Enable(true);
		RevertBox->Enable(true);
		UndoVersionBox->Enable(true);
		UndoCRCBox->Enable(true);
	}

	//Now set up the status bar.
	CreateStatusBar(1);
    SetStatusText("Ready");

	//Now set the layout and sizes.
	SetSizerAndFit(bigBox);
}

//Called when program exits.
void MainFrame::OnQuit( wxCommandEvent& event ) {
	Close(TRUE); // Tells the OS to quit running this process
}

void MainFrame::OnClose(wxCloseEvent& event) {
       //Save settings to BOSS.ini before quitting.
	//Read ini file into string buffer, then search for setting strings and replace their values.
	string buffer;
	if (fs::exists("BOSS.ini")) {
		fileToBuffer("BOSS.ini",buffer);
		size_t pos1,pos2;
		pos1 = buffer.find("[BOSS.InternetSettings]");
		for (int i=0;i<3;i++) {
			pos1 = buffer.find("=",pos1+1);
			if (i==0) {  //Replace ProxyType setting
				//Need to find out how long the current setting string is.
				//Look for the end of the line.
				pos2 = buffer.find("\r\n",pos1);  //Windows EOL.
				if (pos2 == string::npos)
					pos2 = buffer.find("\n",pos1);  //Unix EOL.
				//Length of setting string is pos2-pos1-2.
				buffer.replace(pos1+2,pos2-pos1-2,proxy_type);
			} else if (i==1) {  //Replace ProxyHost setting
				pos2 = buffer.find("\r\n",pos1);  //Windows EOL.
				if (pos2 == string::npos)
					pos2 = buffer.find("\n",pos1);  //Unix EOL.
				buffer.replace(pos1+2,pos2-pos1-2,proxy_host);
			} else if (i==2) {  //Replace ProxyPort setting
				pos2 = buffer.find("\r\n",pos1);  //Windows EOL.
				if (pos2 == string::npos)
					pos2 = buffer.find("\n",pos1);  //Unix EOL.
				buffer.replace(pos1+2,pos2-pos1-2,proxy_port);
			}

		}
		pos1 = buffer.find("[GUI.Settings]", pos1+1);
			pos1 = buffer.find("=",pos1+1);
				buffer.replace(pos1+2,1,BoolToString(do_startup_update_check));  //Replace DoStartupUpdateCheck setting.
		pos1 = buffer.find("[GUI.LastOptions]", pos1+1);
		for (int i=0;i<14;i++) {
			pos1 = buffer.find("=",pos1+1);
			if (i==0)
				buffer.replace(pos1+2,1,IntToString(run_type));  //Replace RunType setting.
			else if (i==1)
				buffer.replace(pos1+2,1,BoolToString(silent));  //Replace SilentRun setting.
			else if (i==2)
				buffer.replace(pos1+2,1,BoolToString(debug));  //Replace Debug setting.
			else if (i==3)
				buffer.replace(pos1+2,1,BoolToString(logCL));  //Replace LogCLOutput setting.
			else if (i==4)
				buffer.replace(pos1+2,4,log_format);  //Replace BOSSlogFormat setting.
			else if (i==5)
				buffer.replace(pos1+2,1,IntToString(verbosity));  //Replace CLVerbosity setting.
			else if (i==6)
				buffer.replace(pos1+2,1,BoolToString(update));  //Replace UpdateMasterlist setting.
			else if (i==7)
				buffer.replace(pos1+2,1,BoolToString(sort_skip_version_parse));  //Replace SortNoVersionParse setting.
			else if (i==8)
				buffer.replace(pos1+2,1,BoolToString(sort_show_CRCs));  //Replace SortDisplayCRCs setting.
			else if (i==9)
				buffer.replace(pos1+2,1,BoolToString(trial_run));  //Replace DoTrialRun setting.
			else if (i==10) {
				//Need to find out how long the current setting string is.
				//Look for the end of the line.
				pos2 = buffer.find("\r\n",pos1);  //Windows EOL.
				if (pos2 == string::npos)
					pos2 = buffer.find("\n",pos1);  //Unix EOL.
				//Length of setting string is pos2-pos1-2.
				if (game == 0)
					buffer.replace(pos1+2,pos2-pos1-2,"auto");  //Replace Game setting.
				else if (game == 1)
					buffer.replace(pos1+2,pos2-pos1-2,"Oblivion");  //Replace Game setting.
				else if (game == 2)
					buffer.replace(pos1+2,pos2-pos1-2,"Fallout3");  //Replace Game setting.
				else if (game == 3)
					buffer.replace(pos1+2,pos2-pos1-2,"Nehrim");  //Replace Game setting.
				else
					buffer.replace(pos1+2,pos2-pos1-2,"FalloutNV");  //Replace Game setting.
			} else if (i==11)
				buffer.replace(pos1+2,1,IntToString(revert));  //Replace RevertLevel setting.
			else if (i==12)
				buffer.replace(pos1+2,1,BoolToString(revert_skip_version_parse));  //Replace RevertNoVersionParse setting.
			else if (i==13)
				buffer.replace(pos1+2,1,BoolToString(revert_show_CRCs));  //Replace RevertDisplayCRCs setting.
		}
		ofstream out("BOSS.ini");
		if (!out.fail()) {
			out.unsetf(ios::skipws);
			out << buffer;
			out.close();
		} else
			wxMessageBox(wxString::Format(
				wxT("Error: BOSS.ini could not be saved. Ensure your BOSS folder is not read-only.")
			),
			wxT("Error"),
			wxOK | wxICON_ERROR,
			this);
	}

    Destroy();  // you may also do:  event.Skip();
                // since the default event handler does call Destroy(), too
}

//Called when program exits.
void MainFrame::OnRunBOSS( wxCommandEvent& event ) {
	if (fs::exists("BOSS.exe"))
		RunBOSS();
	else
		wxMessageBox(wxString::Format(
				wxT("Error: BOSS.exe not found. Reinstall BOSS correctly, so that both BOSS.exe and BOSS GUI.exe are in the BOSS folder in your game's installation directory.")
			),
			wxT("Error"),
			wxOK | wxICON_ERROR,
			this);
}

//Call when a file is opened. Either readmes, BOSSlogs or userlist.
void MainFrame::OnOpenFile( wxCommandEvent& event ) {
	string file;
	if (event.GetId() == OPTION_OpenUserlist)
		file = "userlist.txt";
	else if (event.GetId() == OPTION_OpenBOSSlog)
		file = "bosslog";
	else if (event.GetId() == MENU_OpenMReadMe)
		file = "BOSS ReadMe";
	else
		file = "BOSS User Rules ReadMe";
	//Need to choose file based on what fired the event.
	if (file == "userlist.txt") {
		if (!fs::exists(file)) {  //Create the userlist.
			ofstream ofile(file.c_str());
			ofile.close();
		}
		//Now open it.
		OpenInSysDefault(fs::path("userlist.txt"));
	} else if (file == "bosslog") {
		if (log_format == "html") {  //Open HTML BOSSlog.
			if (fs::exists("BOSSlog.html"))
				OpenInSysDefault(fs::path("BOSSlog.html"));
			else {
				wxMessageBox(wxString::Format(
					wxT("Error: No BOSSlog.html found. Make sure you have run BOSS from BOSS.exe, or run it with the HTML output format selected, at least once before attempting to open the BOSSlog in the HTML format.")
				),
				wxT("Error"),
				wxOK | wxICON_ERROR,
				this);
			}
		} else {  //Open text BOSSlog.
			if (fs::exists("BOSSlog.txt"))
				OpenInSysDefault(fs::path("BOSSlog.txt"));
			else {
				wxMessageBox(wxString::Format(
					wxT("Error: No BOSSlog.txt found. Make sure you have run BOSS at least once with the text output format selected before attempting to open the BOSSlog in the plain text format.")
				),
				wxT("Error"),
				wxOK | wxICON_ERROR,
				this);
			}
		}
	} else {  //Readme files. They could be anywhere - this could be complicated.
		//Simplify by looking for either the files themselves or shortcuts to them in the BOSS folder.
		//If neither, show a pop-up message saying they can't be found.
		if (fs::exists(file + ".html")) {
			file += ".html";
			OpenInSysDefault(fs::path(file));
		} else if (fs::exists(file + ".lnk")) {
			file += ".lnk";
			OpenInSysDefault(fs::path(file));
		} else {
			//No ReadMe exists, show a pop-up message saying so.
			wxMessageBox(wxString::Format(
				wxT("Error: No %s found. Make sure you have the ReadMe or a shortcut to the ReadMe in your BOSS folder."),
				file
			),
			wxT("Error"),
			wxOK | wxICON_ERROR,
			this);
		}
	}
}

void MainFrame::OnAbout(wxCommandEvent& event) {

	wxDialog *frame = new wxDialog(this,-1,"About Better Oblivion Sorting Software GUI");

	frame->SetBackgroundColour(wxColour(255,255,255));

	wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);

	box->Add(new wxStaticText(frame,-1,
		"Better Oblivion Sorting Software GUI\nv"+boss_version+" ("+boss_releaseDate+")\n\n"
		"Provides a graphical front end for running "), 0, wxTOP | wxLEFT | wxRIGHT, 20);

	wxHyperlinkCtrl *link = new wxHyperlinkCtrl(frame, -1, "Better Oblivion Sorting Software","http://code.google.com/p/better-oblivion-sorting-software/");
	link->SetBackgroundColour(wxColour(255,255,255));
	box->Add(link, 0, wxBOTTOM | wxLEFT | wxRIGHT, 20);
	box->Add(new wxStaticText(frame, -1, "© WrinklyNinja and the BOSS development team, 2011.\nSome rights reserved. Copyright license:"), 0, wxLEFT | wxRIGHT, 20);
	link = new wxHyperlinkCtrl(frame, -1, "CC Attribution-Noncommercial-No Derivative Works 3.0","http://creativecommons.org/licenses/by-nc-nd/3.0/");
	link->SetBackgroundColour(wxColour(255,255,255));
	box->Add(link, 0, wxBOTTOM | wxLEFT | wxRIGHT, 20);
	
	//Need to add an 'OK' button.
	wxButton *okButton = new wxButton(frame, OPTION_ExitAbout, wxT("OK"), wxDefaultPosition, wxSize(70, 30));
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(okButton, 1);
	
	box->Add(hbox, 0, wxALIGN_CENTER | wxBOTTOM, 10);
	frame->SetAffirmativeId(OPTION_ExitAbout);

	//Now set the layout and sizes.
	frame->SetSizerAndFit(box);
	
	frame->ShowModal();
}

void MainFrame::OnFormatChange(wxCommandEvent& event) {
	if (event.GetInt() == 0)
		log_format = "html";
	else
		log_format = "text";
}

void MainFrame::OnVerbosityChange(wxCommandEvent& event) {
	verbosity = event.GetInt();
}
void MainFrame::OnGameChange(wxCommandEvent& event) {
	game = event.GetInt();
}

void MainFrame::OnRevertChange(wxCommandEvent& event) {
	revert = event.GetInt();
}

void MainFrame::OnLogDisplayChange(wxCommandEvent& event) {
	silent = (!event.IsChecked());
}

void MainFrame::OnDebugChange(wxCommandEvent& event) {
	debug = event.IsChecked();
}

void MainFrame::OnUpdateChange(wxCommandEvent& event) {
	update = event.IsChecked();
}

void MainFrame::OnVersionDisplayChange(wxCommandEvent& event) {
	if (event.GetId() == CHECKBOX_SortEnableVersions)
		sort_skip_version_parse = (!event.IsChecked());
	else
		revert_skip_version_parse = (!event.IsChecked());
}

void MainFrame::OnCRCDisplayChange(wxCommandEvent& event) {
	if (event.GetId() == CHECKBOX_SortEnableCRCs)
		sort_show_CRCs = event.IsChecked();
	else
		revert_show_CRCs = event.IsChecked();
}

void MainFrame::OnLoggingChange(wxCommandEvent& event) {
	logCL = event.IsChecked();
}

void MainFrame::OnTrialRunChange(wxCommandEvent& event) {
	trial_run = event.IsChecked();
}

void MainFrame::OnRunTypeChange(wxCommandEvent& event) {
	if (event.GetId() == RADIOBUTTON_SortOption) {
		run_type = 1;
		UpdateBox->Enable(true);
		SortVersionBox->Enable(true);
		SortCRCBox->Enable(true);
		TrialRunBox->Enable(true);
		GameText->Enable(false);
		GameBox->Enable(false);
		RevertText->Enable(false);
		RevertBox->Enable(false);
		UndoVersionBox->Enable(false);
		UndoCRCBox->Enable(false);
	}else if (event.GetId() == RADIOBUTTON_UpdateOption) {
		run_type = 2;
		UpdateBox->Enable(false);
		SortVersionBox->Enable(false);
		SortCRCBox->Enable(false);
		TrialRunBox->Enable(false);
		GameText->Enable(true);
		GameBox->Enable(true);
		RevertText->Enable(false);
		RevertBox->Enable(false);
		UndoVersionBox->Enable(false);
		UndoCRCBox->Enable(false);
	}else {
		run_type = 3;
		UpdateBox->Enable(false);
		SortVersionBox->Enable(false);
		SortCRCBox->Enable(false);
		TrialRunBox->Enable(false);
		GameText->Enable(false);
		GameBox->Enable(false);
		RevertText->Enable(true);
		RevertBox->Enable(true);
		UndoVersionBox->Enable(true);
		UndoCRCBox->Enable(true);
	}
}

//This is called when the menu "Check For Updates" option is selected.
void MainFrame::OnUpdateCheck(wxCommandEvent& event) {
	string updateText;
	bool connection = false;
	try {
		connection = CheckConnection();
	} catch (update_error & e) {
		string const * detail = boost::get_error_info<err_detail>(e);
		updateText = "Update check failed. Details: " + *detail;
		wxMessageBox(updateText, wxT("BOSS GUI: Check For Updates"), wxOK | wxICON_ERROR, this);
		return;
	}
	if (connection) {
		try {
			updateText = IsUpdateAvailable();
			if (updateText.length() == 0) {
				wxMessageBox(wxT("You are already using the latest version of BOSS."), wxT("BOSS GUI: Check For Updates"), wxOK | wxICON_INFORMATION, this);
				return;
			} else
				updateText = "Update available! New version: " + updateText + "\nDo you want to download and install the update?";
		} catch (update_error & e) {
			string const * detail = boost::get_error_info<err_detail>(e);
			updateText = "Update check failed. Details: " + *detail;
			wxMessageBox(updateText, wxT("BOSS GUI: Check For Updates"), wxOK | wxICON_ERROR, this);
			return;
		}
	} else {
		wxMessageBox(wxT("Update check failed. No Internet connection detected."), wxT("BOSS GUI: Check For Updates"), wxOK | wxICON_ERROR, this);
		return;
	}

	//Display dialog BOSS telling user about update available.
	wxMessageDialog *dlg = new wxMessageDialog(this,updateText, "BOSS GUI: Check For Updates", wxYES_NO);

	if (dlg->ShowModal() != wxID_YES) {  //User has chosen not to update. Quit now.
		//Display a message saying no update was installed.
		wxMessageBox(wxT("No update has been downloaded or installed."), wxT("BOSS GUI: Check For Updates"), wxOK | wxICON_INFORMATION, this);
		return;
	} else  //User has chosen to update. On with the show!
		Update();
}

//This is called after the GUI has finished launching (actually called every time nothing is happening, but only does something the first time).
void MainFrame::CheckForUpdate(wxIdleEvent& event) {
	if (CheckedForUpdate || do_startup_update_check == false)
		return;
	CheckedForUpdate = true;
	string updateText;
	bool connection = false;
	try {
		connection = CheckConnection();
	} catch (update_error & e) {
		string const * detail = boost::get_error_info<err_detail>(e);
		updateText = "Update check failed. Details: " + *detail;
		wxMessageBox(updateText, wxT("BOSS GUI: Check For Updates"), wxOK | wxICON_ERROR, this);
		return;
	}
	if (connection) {
		try {
			updateText = IsUpdateAvailable();
			if (updateText.length() != 0)
				updateText = "Update available! New version: " + updateText + "\nDo you want to download and install the update?";
			else
				return;
		} catch (update_error& e) {
			string const * detail = boost::get_error_info<err_detail>(e);
			updateText = "Update check failed. Details: " + *detail;
			wxMessageBox(updateText, wxT("BOSS GUI: Check For Updates"), wxOK | wxICON_ERROR, this);
			return;
		}
	} else
		return;

	//Display dialog BOSS telling user about update available.
	wxMessageDialog *dlg = new wxMessageDialog(this,updateText, "BOSS GUI: Check For Updates", wxYES_NO);

	if (dlg->ShowModal() != wxID_YES) {  //User has chosen not to update. Quit now.
		//Display a message saying no update was installed.
		wxMessageBox(wxT("No update has been downloaded or installed."), wxT("BOSS GUI: Check For Updates"), wxOK | wxICON_INFORMATION, this);
		return;
	} else  //User has chosen to update. On with the show!
		Update();
}

void MainFrame::Update() {
	//First detect type of current install: manual or installer.
	if (fs::exists("BOSS ReadMe.lnk")) {  //Installer
		string message = "Your current install has been determined as having been installed via the BOSS installer.\n\n";
		message += "The automatic updater will download the installer for the new version to this BOSS folder.\n\n";
		message += "It will then launch the installer before exiting the GUI. Complete the installer to complete the update.";
		
		wxMessageDialog *dlg = new wxMessageDialog(this,message, wxT("BOSS GUI: Automatic Updater"), wxOK | wxCANCEL);
		if (dlg->ShowModal() != wxID_OK) {  //User has chosen to cancel. Quit now.
			wxMessageBox(wxT("Automatic updater cancelled."), wxT("BOSS GUI: Automatic Updater"), wxOK | wxICON_EXCLAMATION, this);
			return;
		}

		wxProgressDialog *progDia = new wxProgressDialog("BOSS GUI: Automatic Updater","Initialising download...", 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_ELAPSED_TIME|wxPD_CAN_ABORT);
		try {
			DownloadUpdateFiles(INSTALLER, progDia);
			InstallUpdateFiles();
		} catch (update_error & e) {
			progDia->Destroy();
			CleanUp();
			string const * detail = boost::get_error_info<err_detail>(e);
			if (*detail == "Cancelled by user.")
				wxMessageBox(wxT("Update cancelled."), wxT("BOSS GUI: Automatic Updater"), wxOK | wxICON_INFORMATION, this);
			else
				wxMessageBox(wxT("Update failed. Details: " + *detail + "\n\nUpdate cancelled."), wxT("BOSS GUI: Automatic Updater"), wxOK | wxICON_ERROR, this);
			return;
		} catch (fs::filesystem_error e) {
			progDia->Destroy();
			CleanUp();
			string detail = e.what();
			wxMessageBox(wxT("Update failed. Details: " + detail + "\n\nUpdate cancelled."), wxT("BOSS GUI: Automatic Updater"), wxOK | wxICON_ERROR, this);
			return;
		}
		//Remind the user to run the uninstaller and installer.
		wxMessageBox(wxT("New installer successfully downloaded!\n\nWhen you click 'OK', the GUI will launch the downloaded installer and exit. Complete the installer to complete the update."), wxT("BOSS GUI: Automatic Updater"), wxOK | wxICON_INFORMATION, this);

		//Now run downloaded installer then exit.
		//Although there should only be one installer file, to be safe iterate through the files vector.
		for (size_t i=0;i<updatedFiles.size();i++) {
			if (updatedFiles[i].name.empty())  //Just in case.
				continue;
			OpenInSysDefault(fs::path(updatedFiles[i].name));
		}
	} else {  //Manual.
		string message = "Your current install has been determined as having been installed manually.\n\n";
		message += "The automatic updater will download the updated files and replace your existing files with them.";
		if (fs::exists("BOSS.ini"))
			message += " Your current BOSS.ini will be renamed to BOSS.ini.old. It may still be opened in your chosen text editor, allowing you to migrate your settings.";
		if (fs::exists("userlist.txt"))
			message += " Your current userlist.txt will not be replaced.";
		
		wxMessageDialog *dlg = new wxMessageDialog(this,message, wxT("BOSS GUI: Automatic Updater"), wxOK | wxCANCEL);
		if (dlg->ShowModal() != wxID_OK) {  //User has chosen to cancel. Quit now.
			wxMessageBox(wxT("Automatic updater cancelled."), wxT("BOSS GUI: Automatic Updater"), wxOK | wxICON_EXCLAMATION, this);
			return;
		}
		wxProgressDialog *progDia = new wxProgressDialog("BOSS GUI: Automatic Updater","Initialising download...", 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_ELAPSED_TIME|wxPD_CAN_ABORT);
		try {
			DownloadUpdateFiles(MANUAL, progDia);
			InstallUpdateFiles();
		} catch (update_error & e) {
			progDia->Destroy();
			CleanUp();
			string const * detail = boost::get_error_info<err_detail>(e);
			if (*detail == "Cancelled by user.")
				wxMessageBox(wxT("Update cancelled."), wxT("BOSS GUI: Automatic Updater"), wxOK | wxICON_INFORMATION, this);
			else
				wxMessageBox(wxT("Update failed. Details: " + *detail + "\n\nUpdate cancelled."), wxT("BOSS GUI: Automatic Updater"), wxOK | wxICON_ERROR, this);
			return;
		} catch (fs::filesystem_error e) {
			progDia->Destroy();
			CleanUp();
			string detail = e.what();
			wxMessageBox(wxT("Update failed. Details: " + detail + "\n\nUpdate cancelled."), wxT("BOSS GUI: Automatic Updater"), wxOK | wxICON_ERROR, this);
			return;
		}
		//Remind the user to update BOSS GUI.exe
		wxMessageBox(wxT("Files successfully updated!\n\nWhen you click 'OK' the GUI will exit. Once it has closed, you must manually delete your current \"BOSS GUI.exe\" and rename the downloaded \"BOSS GUI.exe.new\" to \"BOSS GUI.exe\" to complete the update."), wxT("BOSS GUI: Automatic Updater"), wxOK | wxICON_INFORMATION, this);
	}
	this->Close();
}

void MainFrame::OnOpenSettings(wxCommandEvent& event) {
	SettingsFrame *settings = new SettingsFrame(wxT("BOSS GUI: Settings"),this,wxDefaultPosition.x,wxDefaultPosition.y,wxDefaultSize.x,wxDefaultSize.y);
	settings->Show();
}

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

	//Set up stuff in the frame.
	SetBackgroundColour(wxColour(255,255,255));

	//Contents in one big resizing box.
	wxBoxSizer *bigBox = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer *generalOptionsBox = new wxStaticBoxSizer(wxVERTICAL, this, "GUI Settings");
	generalOptionsBox->Add(StartupUpdateCheckBox = new wxCheckBox(this,CHECKBOX_StartupUpdateCheck,"Check for BOSS updates on GUI startup"), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	bigBox->Add(generalOptionsBox, 0, wxALL, 20);

	wxStaticBoxSizer *proxyOptionsBox = new wxStaticBoxSizer(wxVERTICAL,this,"Internet Settings");
	proxyOptionsBox->Add(new wxStaticText(this, wxID_ANY, "These settings control BOSS's proxy support and are \napplied for both the GUI and BOSS.exe.\n"), 0, wxALL, 5);
	
	wxBoxSizer *proxyTypeSizer = new wxBoxSizer(wxHORIZONTAL);
	proxyTypeSizer->Add(ProxyTypeText = new wxStaticText(this, wxID_ANY, "Proxy Type: "), 1, wxLEFT | wxTOP | wxBOTTOM, 5);
	proxyTypeSizer->Add(ProxyTypeBox = new wxComboBox(this,DROPDOWN_ProxyType,ProxyTypes[0],wxDefaultPosition,wxDefaultSize,7,ProxyTypes,wxCB_READONLY), 0, wxALIGN_RIGHT | wxRIGHT | wxTOP | wxBOTTOM, 5);
	proxyOptionsBox->Add(proxyTypeSizer, 0, wxEXPAND, 0);
	
	wxBoxSizer *proxyHostSizer = new wxBoxSizer(wxHORIZONTAL);
	proxyHostSizer->Add(ProxyTypeText = new wxStaticText(this, wxID_ANY, "Proxy Hostname:   "), 1, wxLEFT | wxBOTTOM, 5);
	proxyHostSizer->Add(ProxyHostBox = new wxTextCtrl(this,TEXT_ProxyHost), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
	proxyOptionsBox->Add(proxyHostSizer, 0, wxEXPAND, 0);

	wxBoxSizer *proxyPortSizer = new wxBoxSizer(wxHORIZONTAL);
	proxyPortSizer->Add(ProxyTypeText = new wxStaticText(this, wxID_ANY, "Proxy Port Number: "), 1, wxLEFT | wxBOTTOM, 5);
	proxyPortSizer->Add(ProxyPortBox = new wxTextCtrl(this,TEXT_ProxyPort), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
	proxyOptionsBox->Add(proxyPortSizer, 0, wxEXPAND, 0);

	bigBox->Add(proxyOptionsBox, 0, wxLEFT | wxRIGHT | wxBOTTOM, 20);

	//Need to add an 'OK' button.
	wxButton *okButton = new wxButton(this, OPTION_ExitSettings, wxT("OK"), wxDefaultPosition, wxSize(70, 30));
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(okButton, 1);
	
	bigBox->Add(hbox, 0, wxALIGN_CENTER | wxBOTTOM, 10);

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