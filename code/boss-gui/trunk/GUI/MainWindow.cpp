/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/


	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/unordered_set.hpp>

#include <clocale>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>

#include <boost/regex.hpp>

#include <wx/aboutdlg.h>

#include "GUI/MainWindow.h"
#include "GUI/SettingsWindow.h"
#include "GUI/UserRuleEditor.h"
#include "ElementIDs.h"
#include "BOSS-Common.h"

BEGIN_EVENT_TABLE ( MainFrame, wxFrame )
	EVT_CLOSE (MainFrame::OnClose )
	EVT_MENU ( MENU_Quit, MainFrame::OnQuit )
	EVT_MENU ( OPTION_EditUserRules, MainFrame::OnEditUserRules )
	EVT_MENU ( OPTION_OpenBOSSlog, MainFrame::OnOpenFile )
	EVT_MENU ( OPTION_Run, MainFrame::OnRunBOSS )
	EVT_MENU ( MENU_OpenMainReadMe, MainFrame::OnOpenFile )
	EVT_MENU ( MENU_OpenUserRulesReadMe, MainFrame::OnOpenFile )
	EVT_MENU ( OPTION_CheckForUpdates, MainFrame::OnUpdateCheck )
	EVT_MENU ( MENU_ShowAbout, MainFrame::OnAbout )
	EVT_MENU ( MENU_ShowSettings, MainFrame::OnOpenSettings )
	EVT_BUTTON ( OPTION_Run, MainFrame::OnRunBOSS )
	EVT_BUTTON ( OPTION_EditUserRules, MainFrame::OnEditUserRules )
	EVT_BUTTON ( OPTION_OpenBOSSlog, MainFrame::OnOpenFile )
	EVT_BUTTON ( OPTION_CheckForUpdates, MainFrame::OnUpdateCheck )
	EVT_CHOICE ( DROPDOWN_LogFormat, MainFrame::OnFormatChange )
	EVT_CHOICE ( DROPDOWN_Game, MainFrame::OnGameChange )
	EVT_CHOICE ( DROPDOWN_Revert, MainFrame::OnRevertChange )
	EVT_CHECKBOX ( CHECKBOX_ShowBOSSlog, MainFrame::OnLogDisplayChange )
	EVT_CHECKBOX ( CHECKBOX_Update, MainFrame::OnUpdateChange )
	EVT_CHECKBOX ( CHECKBOX_EnableVersions, MainFrame::OnVersionDisplayChange )
	EVT_CHECKBOX ( CHECKBOX_EnableCRCs, MainFrame::OnCRCDisplayChange )
	EVT_CHECKBOX ( CHECKBOX_TrialRun, MainFrame::OnTrialRunChange )
	EVT_RADIOBUTTON ( RADIOBUTTON_SortOption, MainFrame::OnRunTypeChange )
	EVT_RADIOBUTTON ( RADIOBUTTON_UpdateOption, MainFrame::OnRunTypeChange )
	EVT_RADIOBUTTON ( RADIOBUTTON_UndoOption, MainFrame::OnRunTypeChange )
END_EVENT_TABLE()

IMPLEMENT_APP(BossGUI)

using namespace boss;
using namespace std;

//Draws the main window when program starts.
bool BossGUI::OnInit() {
	//Set up variable defaults.
	if (fs::exists(ini_path)) {
		parseIni(ini_path);
		if (!iniErrorBuffer.empty())
			wxMessageBox(wxString::Format(
				wxT("Error: BOSS.ini parsing failed. Some or all of BOSS's options may not have been set correctly. Run BOSS to see the details of the failure.")
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
	} else {
		if (!GenerateIni())
			wxMessageBox(wxString::Format(
				wxT("Error: BOSS.ini generation failed. Ensure your BOSS folder is not read-only. None of BOSS's options will be saved.")
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
	}

	// set alternative output stream for logger and whether to track log statement origins
	if (log_debug_output)
		g_logger.setStream(debug_log_path.string().c_str());
	g_logger.setOriginTracking(debug_with_source);

	if (game == 0) {
		try {
			GetGame();
		} catch (boss_error e) {
			wxMessageBox(wxString::Format(
					wxT("Error: Game autodetection failed! " + e.getString())
				),
				wxT("BOSS: Error"),
				wxOK | wxICON_ERROR,
				NULL);
		}
	}

	MainFrame *frame = new MainFrame(
		wxT("Better Oblivion Sorting Software - " + GetGameString()), 100, 100, 510, 370);

	frame->SetIcon(wxIconLocation("BOSS GUI.exe"));
	frame->Show(TRUE);
	SetTopWindow(frame);

	//Now check for updates.
	CheckForUpdate(frame);
	return true;
}

//This is called after the GUI has finished launching.
void BossGUI::CheckForUpdate(MainFrame *frame) {
	if (do_startup_update_check == false)
		return;
	string updateText, updateVersion;
	bool connection = false;
	try {
		connection = CheckConnection();
	} catch (boss_error e) {
		updateText = "Update check failed. Details: " + e.getString();
		wxMessageBox(updateText, wxT("BOSS: Check For Updates"), wxOK | wxICON_ERROR, frame);
		return;
	}
	if (connection) {
		try {
			updateVersion = IsBOSSUpdateAvailable();
			if (!updateVersion.empty())
				updateText = "Update available! New version: " + updateVersion + "\nDo you want to download and install the update?";
			else
				return;
		} catch (boss_error e) {
			updateText = "Update check failed. Details: " + e.getString();
			wxMessageBox(updateText, wxT("BOSS: Check For Updates"), wxOK | wxICON_ERROR, frame);
			return;
		}
	} else
		return;

	//Display dialog BOSS telling user about update available.
	wxMessageDialog *dlg = new wxMessageDialog(frame,updateText, "BOSS: Check For Updates", wxYES_NO);

	if (dlg->ShowModal() != wxID_YES) {  //User has chosen not to update. Quit now.
		//Display a message saying no update was installed.
		wxMessageBox(wxT("No update has been downloaded or installed."), wxT("BOSS: Check For Updates"), wxOK | wxICON_INFORMATION, frame);
		return;
	} else  //User has chosen to update. On with the show!
		frame->Update(updateVersion);
}

MainFrame::MainFrame(const wxChar *title, int x, int y, int width, int height) : wxFrame(NULL, wxID_ANY, title, wxPoint(x, y), wxSize(width, height)) {

	//Some variable setup.
	wxString BOSSlogFormat[] = {
        wxT("HTML"),
        wxT("Plain Text")
    };
	
	wxString Game[] = {
		wxT("Autodetect"),
		wxT("Oblivion"),
		wxT("Fallout 3"),
		wxT("Nehrim"),
		wxT("Fallout: New Vegas"),
		wxT("Skyrim")
	};
	wxString UndoLevel[] = {
		wxT("No Undo"),
		wxT("Last Run"),
		wxT("2nd Last Run")
	};

	//Set up menu bar first.
    MenuBar = new wxMenuBar();
    // File Menu
    FileMenu = new wxMenu();
	FileMenu->Append(OPTION_OpenBOSSlog, wxT("&View BOSS Log"), wxT("Opens your BOSSlog."));
    FileMenu->Append(OPTION_Run, wxT("&Run BOSS"), wxT("Runs BOSS with the options you have chosen."));
    FileMenu->AppendSeparator();
    FileMenu->Append(MENU_Quit, wxT("&Quit"), wxT("Quit BOSS."));
    MenuBar->Append(FileMenu, wxT("&File"));
	//Edit Menu
	EditMenu = new wxMenu();
	EditMenu->Append(OPTION_EditUserRules, wxT("&User Rules..."), wxT("Opens your userlist in your default text editor."));
	EditMenu->Append(MENU_ShowSettings, wxT("&Settings..."), wxT("Opens the Settings window."));
	MenuBar->Append(EditMenu, wxT("&Edit"));
    // About menu
    HelpMenu = new wxMenu();
	HelpMenu->Append(MENU_OpenMainReadMe, wxT("Open &Main ReadMe"), wxT("Opens the main BOSS ReadMe in your default web browser."));
	HelpMenu->Append(MENU_OpenUserRulesReadMe, wxT("Open &User Rules ReadMe"), wxT("Opens the User Rules ReadMe in your default web browser."));
	HelpMenu->Append(MENU_OpenMasterlistReadMe, wxT("Open &Masterlist &ReadMe"), wxT("Opens the BOSS Masterlist Syntax ReadMe in your default web browser."));
	HelpMenu->Append(MENU_OpenAPIReadMe, wxT("&Open API ReadMe"), wxT("Opens the BOSS API ReadMe in your default web browser."));
	HelpMenu->AppendSeparator();
	HelpMenu->Append(OPTION_CheckForUpdates, wxT("&Check For Updates..."), wxT("Checks for updates to BOSS."));
	HelpMenu->Append(MENU_ShowAbout, wxT("&About BOSS..."), wxT("Shows information about BOSS."));
    MenuBar->Append(HelpMenu, wxT("&Help"));
    SetMenuBar(MenuBar);

	//Set up stuff in the frame.
	SetBackgroundColour(wxColour(255,255,255));

	//Contents in one big resizing box.
	wxBoxSizer *bigBox = new wxBoxSizer(wxHORIZONTAL);

	//Create first column box and add the output options to it.
	wxBoxSizer *columnBox = new wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer *outputOptionsBox = new wxStaticBoxSizer(wxVERTICAL, this, "Output Options");
	wxBoxSizer *formatBox = new wxBoxSizer(wxHORIZONTAL);
	
	//Add stuff to output options sizer.
	outputOptionsBox->Add(ShowLogBox = new wxCheckBox(this,CHECKBOX_ShowBOSSlog, wxT("Show BOSS Log On Completion")), 0, wxALL, 5);
	outputOptionsBox->Add(VersionBox = new wxCheckBox(this,CHECKBOX_EnableVersions, wxT("Display Plugin Versions")), 0, wxLEFT | wxBOTTOM, 5);
	outputOptionsBox->Add(CRCBox = new wxCheckBox(this,CHECKBOX_EnableCRCs, wxT("Display File CRCs")), 0, wxLEFT | wxBOTTOM, 5);
	formatBox->Add(new wxStaticText(this, wxID_ANY, wxT("BOSS Log Format: ")), 1, wxLEFT | wxBOTTOM, 5);
	formatBox->Add(FormatChoice = new wxChoice(this, DROPDOWN_LogFormat, wxPoint(110,60), wxDefaultSize, 2, BOSSlogFormat, wxCB_READONLY), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
	//Add the verbosityBox to its parent now to preserve layout.
	outputOptionsBox->Add(formatBox, 0, wxEXPAND, 0);
	columnBox->Add(outputOptionsBox, 0, wxBOTTOM, 30);

	//Now add the main buttons to the first column.
	wxBoxSizer *buttonBox = new wxBoxSizer(wxVERTICAL);
	buttonBox->Add(EditUserRulesButton = new wxButton(this,OPTION_EditUserRules, wxT("Edit User Rules"), wxDefaultPosition, wxSize(120,30)), 0, wxBOTTOM, 5);
	buttonBox->Add(RunBOSSButton = new wxButton(this,OPTION_Run, wxT("Run BOSS"), wxDefaultPosition, wxSize(120,30)));
	buttonBox->Add(OpenBOSSlogButton = new wxButton(this,OPTION_OpenBOSSlog, wxT("View BOSS Log"), wxDefaultPosition, wxSize(120,30)), 0, wxTOP, 5);
	columnBox->Add(buttonBox, 0, wxALIGN_CENTER, 20);

	//Add the first column to the big box.
	bigBox->Add(columnBox, 0, wxALL, 20);

	//The second column has a border.
	wxStaticBoxSizer *runOptionsBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Run Options"));
	wxBoxSizer *sortBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *updateBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *undoBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *gameBox = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *revertBox = new wxBoxSizer(wxHORIZONTAL);

	//Run Options
	runOptionsBox->Add(SortOption = new wxRadioButton(this, RADIOBUTTON_SortOption, wxT("Sort Plugins")), 0, wxALL, 5);
	
	//Sort option stuff.
	sortBox->Add(UpdateBox = new wxCheckBox(this,CHECKBOX_Update, wxT("Update Masterlist")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	sortBox->Add(TrialRunBox = new wxCheckBox(this,CHECKBOX_TrialRun, wxT("Perform Trial Run")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	runOptionsBox->Add(sortBox, 0, wxLEFT | wxBOTTOM, 20);
	
	//Update only stuff.
	runOptionsBox->Add(UpdateOption = new wxRadioButton(this, RADIOBUTTON_UpdateOption, wxT("Update Masterlist Only")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	gameBox->Add(GameText = new wxStaticText(this, wxID_ANY, wxT("Game: ")), 1, wxLEFT | wxBOTTOM, 15);
	gameBox->Add(GameChoice = new wxChoice(this, DROPDOWN_Game, wxDefaultPosition, wxDefaultSize, 6, Game), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
	updateBox->Add(gameBox, 0, wxEXPAND, 0);
	runOptionsBox->Add(updateBox, 0, wxEXPAND | wxLEFT | wxBOTTOM, 20);
	
	//Undo option stuff.
	runOptionsBox->Add(UndoOption = new wxRadioButton(this, RADIOBUTTON_UndoOption, wxT("Undo Changes")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	revertBox->Add(RevertText = new wxStaticText(this, wxID_ANY, wxT("Undo Level: ")), 1, wxLEFT | wxBOTTOM, 5);
	revertBox->Add(RevertChoice = new wxChoice(this, DROPDOWN_Revert, wxDefaultPosition, wxDefaultSize, 3, UndoLevel), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);
	undoBox->Add(revertBox, 0, wxEXPAND, 0);
	runOptionsBox->Add(undoBox, 0, wxEXPAND | wxLEFT | wxBOTTOM, 20);

	bigBox->Add(runOptionsBox, 0, wxTOP | wxRIGHT | wxBOTTOM, 20);


	//Tooltips
	FormatChoice->SetToolTip(wxT("This decides both the format of BOSSlog generated when you click the \"Run BOSS\" button and the BOSSlog format opened when you click the \"View BOSSlog\" button."));
	OpenBOSSlogButton->SetToolTip(wxT("The format of BOSSlog this opens is decided by the setting of the \"BOSSlog Format\" Output Option above."));
	TrialRunBox->SetToolTip(wxT("Runs BOSS, simulating its changes to your load order, but doesn't actually reorder your mods."));

	//Set option values based on initialised variable values.
	RunBOSSButton->SetDefault();

	if (!silent)
		ShowLogBox->SetValue(true);

	if (log_format == "html")
		FormatChoice->SetSelection(0);
	else if (log_format == "text")
		FormatChoice->SetSelection(1);

	if (show_CRCs)
		CRCBox->SetValue(true);

	if (!skip_version_parse)
		VersionBox->SetValue(true);

	if (update)
		UpdateBox->SetValue(true);

	if (trial_run)
		TrialRunBox->SetValue(true);

	if (game == 0)
		GameChoice->SetSelection(0);
	else if (game == 1)
		GameChoice->SetSelection(1);
	else if (game == 2)
		GameChoice->SetSelection(2);
	else if (game == 3)
		GameChoice->SetSelection(3);
	else if (game == 4)
		GameChoice->SetSelection(4);
	else if (game == 5)
		GameChoice->SetSelection(5);

	if (revert == 0)
		RevertChoice->SetSelection(0);
	else if (revert == 1)
		RevertChoice->SetSelection(1);
	else if (revert == 2)
		RevertChoice->SetSelection(2);

	if (run_type == 1) {
		SortOption->SetValue(true);
		
		UpdateBox->Enable(true);
		TrialRunBox->Enable(true);

		GameText->Enable(false);
		GameChoice->Enable(false);
		RevertText->Enable(false);
		RevertChoice->Enable(false);
	} else if (run_type == 2) {
		UpdateOption->SetValue(true);

		GameText->Enable(true);
		GameChoice->Enable(true);

		UpdateBox->Enable(false);
		TrialRunBox->Enable(false);
		RevertText->Enable(false);
		RevertChoice->Enable(false);
	} else {
		UndoOption->SetValue(true);

		RevertText->Enable(true);
		RevertChoice->Enable(true);

		UpdateBox->Enable(false);
		TrialRunBox->Enable(false);
		GameText->Enable(false);
		GameChoice->Enable(false);
	}

	//Now set up the status bar.
	CreateStatusBar(1);
    SetStatusText(wxT("Ready"));

	//Now set the layout and sizes.
	SetSizerAndFit(bigBox);
}

//Called when program exits.
void MainFrame::OnQuit( wxCommandEvent& event ) {
	Close(TRUE); // Tells the OS to quit running this process
}

void MainFrame::OnClose(wxCloseEvent& event) {
	//Save settings to BOSS.ini before quitting.
	if (!GenerateIni())
			wxMessageBox(wxString::Format(
				wxT("Error: BOSS.ini update failed. Ensure your BOSS folder is not read-only. None of the BOSS's options will be saved.")
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);

    Destroy();  // you may also do:  event.Skip();
                // since the default event handler does call Destroy(), too
}

//Called when program exits.
void MainFrame::OnRunBOSS( wxCommandEvent& event ) {

	size_t lastRecognisedPos = 0;			//position of last recognised mod.
	string scriptExtender;					//What script extender is present.
	time_t esmtime = 0;						//File modification times.
	vector<item> Modlist, Masterlist;		//Modlist and masterlist data structures.
	vector<rule> Userlist;					//Userlist data structure.
	summaryCounters counters;				//Summary counters.
	bosslogContents contents;				//BOSSlog contents.
	string gameStr;							// allow for autodetection override
	fs::path bosslog_path;					//Path to BOSSlog being used.
	fs::path sortfile;						//Modlist/masterlist to sort plugins using.

	if (log_debug_output)
		g_logger.setStream(debug_log_path.string().c_str());
	g_logger.setOriginTracking(debug_with_source);

	//Tell the user that stuff is happenining.
	wxProgressDialog *progDia = new wxProgressDialog(wxT("BOSS: Working..."),wxT("Better Oblivion Sorting Software working..."), 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_ELAPSED_TIME|wxPD_CAN_ABORT);

	//Set the locale to get encoding conversions working correctly.
	setlocale(LC_CTYPE, "");
	locale global_loc = locale();
	locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	LOG_INFO("BOSS starting...");

	//Set BOSSlog path to be used.
	if (log_format == "html")
		bosslog_path = bosslog_html_path;
	else
		bosslog_path = bosslog_text_path;

	//Set masterlist path to be used.
	if (revert==1)
		sortfile = curr_modlist_path;	
	else if (revert==2) 
		sortfile = prev_modlist_path;
	else 
		sortfile = masterlist_path;
	LOG_INFO("Using sorting file: %s", sortfile.string().c_str());

	progDia->Pulse();
	if (progDia->WasCancelled()) {
		progDia->Destroy();
		return;
	}

	////////////////////////////////////////////////
	// Record last BOSSlog's recognised mod list
	////////////////////////////////////////////////

	//Back up old recognised mod list for diff later. Only works for HTML bosslog due to formatting conversion.
	if (fs::exists(bosslog_html_path))
		contents.oldRecognisedPlugins = GetOldRecognisedList(bosslog_html_path);

	progDia->Pulse();
	if (progDia->WasCancelled()) {
		progDia->Destroy();
		return;
	}

	/////////////////////////////////////////
	// Check for critical error conditions
	/////////////////////////////////////////

	//BOSSlog check.
	LOG_DEBUG("opening '%s'", bosslog_path.string().c_str());
	bosslog.open(bosslog_path.c_str());  //Might as well keep it open, just don't write anything unless an error till the end.
	if (bosslog.fail()) {
		LOG_ERROR("file '%s' could not be accessed for writing. Check the"
				  " Troubleshooting section of the ReadMe for more"
				  " information and possible solutions.", bosslog_path.string().c_str());
		progDia->Destroy();
		return;
	}

	//Game checks.
	if (0 == game) {
		LOG_DEBUG("Detecting game...");
		try {
			GetGame();
		} catch (boss_error e) {
			LOG_ERROR("Critical Error: %s", e.getString().c_str());
			OutputHeader();
			Output("<p class='error'>Critical Error: " + e.getString() + "<br />");
			Output("Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />");
			Output("Utility will end now.");
			OutputFooter();
			bosslog.close();
			LOG_ERROR("Installation error found: check BOSSLOG.");
			if ( !silent ) 
				wxLaunchDefaultApplication(bosslog_path.string());	//Displays the BOSSlog.txt.
			progDia->Destroy();
			return; //fail in screaming heap.
		}
	}
	LOG_INFO("Game detected: %d", game);

	progDia->Pulse();
	if (progDia->WasCancelled()) {
		progDia->Destroy();
		return;
	}

	/////////////////////////////////////////////////////////
	// Error Condition Check Interlude - Update Masterlist
	/////////////////////////////////////////////////////////
	
	if (revert<1 && (update || update_only)) {
		//First check for internet connection, then update masterlist if connection present.
		bool connection = false;
		try {
			connection = CheckConnection();
		} catch (boss_error e) {
			contents.updaterErrors += "<li class='warn'>Error: Masterlist update failed.<br />";
			contents.updaterErrors += "Details: " + EscapeHTMLSpecial(e.getString()) + "<br />";
			contents.updaterErrors += "Check the Troubleshooting section of the ReadMe for more information and possible solutions.";
			LOG_ERROR("Error: Masterlist update failed. Details: %s", e.getString().c_str());
		}
		if (connection) {
			progDia->Update(0,wxT("Updating to the latest masterlist from the Google Code repository..."));
			LOG_DEBUG("Updating masterlist...");
			try {
				string localDate, remoteDate;
				unsigned int localRevision, remoteRevision;
				uiStruct ui(progDia);
				UpdateMasterlist(ui, localRevision, localDate, remoteRevision, remoteDate);
				if (localRevision == remoteRevision) {
					contents.summary += "<p>Your masterlist is already at the latest revision (r" + IntToString(localRevision) + "; " + EscapeHTMLSpecial(localDate) + "). No update necessary.";
					progDia->Pulse(wxT("Masterlist update unnecessary."));
					LOG_DEBUG("Masterlist update unnecessary.");
				} else {
					contents.summary += "<p>Your masterlist has been updated to revision " + IntToString(remoteRevision) + " (" + EscapeHTMLSpecial(remoteDate) + ").";
					progDia->Pulse(wxT("Masterlist updated successfully."));
					LOG_DEBUG("Masterlist updated successfully.");
				}
			} catch (boss_error e) {
				contents.updaterErrors += "<li class='warn'>Error: Masterlist update failed.<br />";
				contents.updaterErrors += "Details: " + EscapeHTMLSpecial(e.getString()) + "<br />";
				contents.updaterErrors += "Check the Troubleshooting section of the ReadMe for more information and possible solutions.";
				LOG_ERROR("Error: Masterlist update failed. Details: %s", e.getString().c_str());
			}
		} else
			contents.summary += "<p>No internet connection detected. Masterlist auto-updater aborted.";
	}

	//If true, exit BOSS now. Flush earlyBOSSlogBuffer to the bosslog and exit.
	if (update_only == true) {
		OutputHeader();
		if (contents.updaterErrors.empty()) {
			Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>Summary</h3><ul>");
			Output(contents.summary);
			Output("</ul>");
		} else {
			Output("<h3 onclick='toggleSectionDisplay(this)'><span>&#x2212;</span>General Messages</h3><ul>");
			Output(contents.updaterErrors);
			Output("</ul>");
		}
		Output("<h3 id='end'>BOSS Execution Complete</h3>");
		OutputFooter();
		bosslog.close();
		if ( !silent ) 
			wxLaunchDefaultApplication(bosslog_path.string());	//Displays the BOSSlog.
		progDia->Destroy();
		return;
	}

	progDia->Pulse(wxT("Better Oblivion Sorting Software working..."));
	if (progDia->WasCancelled()) {
		progDia->Destroy();
		return;
	}

	///////////////////////////////////
	// Resume Error Condition Checks
	///////////////////////////////////

	//Get the master esm's modification date. 
	try {
		esmtime = GetMasterTime();
	} catch (boss_error e) {
		OutputHeader();
		Output("<p class='error'>Critical Error: " + e.getString() + "<br />");
		Output("Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />");
		Output("Utility will end now.");
		OutputFooter();
		bosslog.close();
		LOG_ERROR("Failed to set modification time of game master file, error was: %s", e.getString().c_str());
		if ( !silent ) 
			wxLaunchDefaultApplication(bosslog_path.string());	//Displays the BOSSlog.txt.
		progDia->Destroy();
		return; //fail in screaming heap.
	}

	//Build and save modlist.
	BuildModlist(Modlist);
	if (revert<1) {
		try {
			SaveModlist(Modlist, curr_modlist_path);
		} catch (boss_error e) {
			OutputHeader();
			Output("<p class='error'>Critical Error: Modlist backup failed!<br />");
			Output("Details: " + EscapeHTMLSpecial(e.getString()) + ".<br />");
			Output("Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />");
			Output("Utility will end now.");
			OutputFooter();
			bosslog.close();
			if ( !silent ) 
				wxLaunchDefaultApplication(bosslog_path.string());	//Displays the BOSSlog.txt.
			progDia->Destroy();
			return; //fail in screaming heap.
		}
	}

	progDia->Pulse();
	if (progDia->WasCancelled()) {
		progDia->Destroy();
		return;
	}

	/////////////////////////////////
	// Parse Master- and Userlists
	/////////////////////////////////
	//Masterlist parse errors are critical, ini and userlist parse errors are not.
	
	//Parse masterlist/modlist backup into data structure.
	LOG_INFO("Starting to parse sorting file: %s", sortfile.string().c_str());
	try {
		parseMasterlist(sortfile,Masterlist);
	} catch (boss_error e) {
		OutputHeader();
		Output("<p class='error'>Critical Error: " +EscapeHTMLSpecial(e.getString()) +"<br />");
		Output("Check the Troubleshooting section of the ReadMe for more information and possible solutions.<br />");
		Output("Utility will end now.");
		OutputFooter();
        bosslog.close();
        LOG_ERROR("Couldn't open sorting file: %s", sortfile.filename().string().c_str());
        if ( !silent ) 
			wxLaunchDefaultApplication(bosslog_path.string());  //Displays the BOSSlog.txt.
		progDia->Destroy();
        return; //fail in screaming heap.
	}

	//Check if parsing failed. If so, exit with errors.
	if (!masterlistErrorBuffer.empty()) {
		OutputHeader();
		size_t size = masterlistErrorBuffer.size();
		for (size_t i=0; i<size; i++)  //Print parser error messages.
			Output(masterlistErrorBuffer[i]);
		OutputFooter();
		bosslog.close();
		if ( !silent ) 
			wxLaunchDefaultApplication(bosslog_path.string());  //Displays the BOSSlog.txt.
		progDia->Destroy();
        return; //fail in screaming heap.
	}

	LOG_INFO("Starting to parse userlist.");
	try {
		bool parsed = parseUserlist(userlist_path,Userlist);
		if (!parsed)
			Userlist.clear();  //If userlist has parsing errors, empty it so no rules are applied.
	} catch (boss_error e) {
		userlistErrorBuffer.push_back("<p class='error'>Error: "+e.getString()+" Userlist parsing aborted. No rules will be applied.");
		LOG_ERROR("Error: %s", e.getString().c_str());
	}

	progDia->Pulse();
	if (progDia->WasCancelled()) {
		progDia->Destroy();
		return;
	}

	/////////////////////////////////////////////////
	// Compare Masterlist against Modlist, Userlist
	/////////////////////////////////////////////////

	lastRecognisedPos = BuildWorkingModlist(Modlist,Masterlist,Userlist);
	LOG_INFO("Modlist now filled with ordered mods and unknowns.");

	progDia->Pulse();
	if (progDia->WasCancelled()) {
		progDia->Destroy();
		return;
	}

	//////////////////////////
	// Apply Userlist Rules
	//////////////////////////

	//Apply userlist rules to modlist.
	if (revert<1 && fs::exists(userlist_path)) {
		ApplyUserRules(Modlist, Userlist, contents.userlistMessages, lastRecognisedPos);
		LOG_INFO("Userlist sorting process finished.");
	}

	progDia->Pulse();
	if (progDia->WasCancelled()) {
		progDia->Destroy();
		return;
	}

	//////////////////////////////////////////////////////
	// Print version & checksum info for OBSE & plugins
	//////////////////////////////////////////////////////

	if (show_CRCs)
		scriptExtender = GetSEPluginInfo(contents.seInfo);

	progDia->Pulse();
	if (progDia->WasCancelled()) {
		progDia->Destroy();
		return;
	}

	////////////////////////////////
	// Re-date Files & Output Info
	////////////////////////////////

	//Re-date .esp/.esm files according to order in modlist and output messages
	SortRecognisedMods(Modlist, lastRecognisedPos, contents.recognisedPlugins, esmtime, counters);


	//Find and show found mods not recognised. These are the mods that are found at and after index lastRecognisedPos in the mods vector.
	//Order their dates to be i days after the master esm to ensure they load last.
	ListUnrecognisedMods(Modlist, lastRecognisedPos, contents.unrecognisedPlugins, esmtime, counters);

	progDia->Pulse();
	if (progDia->WasCancelled()) {
		progDia->Destroy();
		return;
	}

	/////////////////////////////
	// Print Output to BOSSlog
	/////////////////////////////

	PrintBOSSlog(contents, counters, scriptExtender);

	progDia->Destroy();

	bosslog.close();
	LOG_INFO("Launching boss log in browser.");
	if ( !silent ) 
		wxLaunchDefaultApplication(bosslog_path.string());	//Displays the BOSSlog.txt.
	LOG_INFO("BOSS finished.");
	return;
}

void MainFrame::OnEditUserRules( wxCommandEvent& event ) {
	if (use_user_rules_editor) {
		UserRulesEditorFrame *editor = new UserRulesEditorFrame(wxT("Better Oblivion Sorting Software: User Rule Editor"),this);
		editor->SetIcon(wxIconLocation("BOSS GUI.exe"));
		editor->Show();
		return;
	} else {
		if (fs::exists(userlist_path))
			wxLaunchDefaultApplication(userlist_path.string());
		else {
			ofstream userlist_file(userlist_path.c_str(),ios_base::binary);
			if (!userlist_file.fail())
				userlist_file << '\xEF' << '\xBB' << '\xBF';  //Write UTF-8 BOM to ensure the file is recognised as having the UTF-8 encoding.
			else
				wxMessageBox(wxString::Format(
					wxT("Error: Userlist.txt could not be created. Ensure your BOSS folder is not read-only.")
				),
				wxT("BOSS: Error"),
				wxOK | wxICON_ERROR,
				this);
			userlist_file.close();
			wxLaunchDefaultApplication(userlist_path.string());
		}
	}
}

//Call when a file is opened. Either readmes, BOSSlogs or userlist.
void MainFrame::OnOpenFile( wxCommandEvent& event ) {
	string file;
	if (event.GetId() == OPTION_OpenBOSSlog) {
		if (log_format == "html") {  //Open HTML BOSSlog.
			if (fs::exists(bosslog_html_path))
				wxLaunchDefaultApplication(bosslog_html_path.string());
			else {
				wxMessageBox(wxString::Format(
					wxT("Error: No BOSSlog.html found. Make sure you have run BOSS with the HTML output format selected at least once before attempting to open the BOSSlog in the HTML format.")
				),
				wxT("BOSS: Error"),
				wxOK | wxICON_ERROR,
				this);
			}
		} else {  //Open text BOSSlog.
			if (fs::exists("BOSSlog.txt"))
				wxLaunchDefaultApplication("BOSSlog.txt");
			else {
				wxMessageBox(wxString::Format(
					wxT("Error: No BOSSlog.txt found. Make sure you have run BOSS at least once with the plain text output format selected before attempting to open the BOSSlog in the plain text format.")
				),
				wxT("BOSS: Error"),
				wxOK | wxICON_ERROR,
				this);
			}
		}
	} else {
		//Readme files. They could be anywhere - this could be complicated.
		if (event.GetId() == MENU_OpenMainReadMe)
			file = "BOSS ReadMe";
		else if (event.GetId() == MENU_OpenUserRulesReadMe)
			file = "BOSS User Rules ReadMe";
		else if (event.GetId() == MENU_OpenMasterlistReadMe)
			file = "BOSS Masterlist Syntax";
		else if (event.GetId() == MENU_OpenAPIReadMe)
			file = "BOSS API ReadMe";
		//Simplify by looking for either the files themselves or shortcuts to them in the BOSS folder.
		//If neither, show a pop-up message saying they can't be found.
		if (fs::exists(file + ".html")) {
			file += ".html";
			wxLaunchDefaultApplication(file);
		} else if (fs::exists(file + ".lnk")) {
			file += ".lnk";
			wxLaunchDefaultApplication(file);
		} else {
			//No ReadMe exists, show a pop-up message saying so.
			wxMessageBox(wxString::Format(
				wxT("Error: No %s found. Make sure you have the ReadMe or a shortcut to the ReadMe in your BOSS folder."),
				file
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			this);
		}
	}
}

void MainFrame::OnAbout(wxCommandEvent& event) {
	wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("Better Oblivion Sorting Software");
    aboutInfo.SetVersion(IntToString(BOSS_VERSION_MAJOR)+"."+IntToString(BOSS_VERSION_MINOR)+"."+IntToString(BOSS_VERSION_PATCH));
    aboutInfo.SetDescription(wxT("A \"one-click\" program for users that quickly optimises and avoids detrimental conflicts in their\nTES IV: Oblivion, Nehrim - At Fate's Edge, TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders."));
    aboutInfo.SetCopyright("(C) 2011 Random007, WrinklyNinja and the BOSS Development Team.");
    aboutInfo.SetWebSite("http://code.google.com/p/better-oblivion-sorting-software/");
	aboutInfo.SetLicence("CC Attribution-Noncommercial-No Derivative Works 3.0 (http://creativecommons.org/licenses/by-nc-nd/3.0/)");
	aboutInfo.SetIcon(wxIconLocation("BOSS GUI.exe"));
    wxAboutBox(aboutInfo);
}

void MainFrame::OnLogDisplayChange(wxCommandEvent& event) {
	silent = (!event.IsChecked());
}

void MainFrame::OnFormatChange(wxCommandEvent& event) {
	if (event.GetInt() == 0)
		log_format = "html";
	else
		log_format = "text";
}

void MainFrame::OnVersionDisplayChange(wxCommandEvent& event) {
	skip_version_parse = (!event.IsChecked());
}

void MainFrame::OnCRCDisplayChange(wxCommandEvent& event) {
	show_CRCs = event.IsChecked();
}

void MainFrame::OnUpdateChange(wxCommandEvent& event) {
	update = event.IsChecked();
}

void MainFrame::OnTrialRunChange(wxCommandEvent& event) {
	trial_run = event.IsChecked();
}

void MainFrame::OnGameChange(wxCommandEvent& event) {
	game = event.GetInt();
}

void MainFrame::OnRevertChange(wxCommandEvent& event) {
	revert = event.GetInt();
}

void MainFrame::OnRunTypeChange(wxCommandEvent& event) {
	if (event.GetId() == RADIOBUTTON_SortOption) {
		run_type = 1;

		UpdateBox->Enable(true);
		TrialRunBox->Enable(true);

		GameText->Enable(false);
		GameChoice->Enable(false);
		RevertText->Enable(false);
		RevertChoice->Enable(false);
	}else if (event.GetId() == RADIOBUTTON_UpdateOption) {
		run_type = 2;

		GameText->Enable(true);
		GameChoice->Enable(true);

		UpdateBox->Enable(false);
		TrialRunBox->Enable(false);
		RevertText->Enable(false);
		RevertChoice->Enable(false);
	}else {
		run_type = 3;

		RevertText->Enable(true);
		RevertChoice->Enable(true);

		UpdateBox->Enable(false);
		TrialRunBox->Enable(false);
		GameText->Enable(false);
		GameChoice->Enable(false);
	}
}

//This is called when the menu "Check For Updates" option is selected.
void MainFrame::OnUpdateCheck(wxCommandEvent& event) {
	string updateText, updateVersion;
	bool connection = false;
	try {
		connection = CheckConnection();
	} catch (boss_error e) {
		updateText = "Update check failed. Details: " + e.getString();
		wxMessageBox(updateText, wxT("BOSS: Check For Updates"), wxOK | wxICON_ERROR, this);
		return;
	}
	if (connection) {
		try {
			updateVersion = IsBOSSUpdateAvailable();
			if (updateVersion.empty()) {
				wxMessageBox(wxT("You are already using the latest version of BOSS."), wxT("BOSS: Check For Updates"), wxOK | wxICON_INFORMATION, this);
				return;
			} else
				updateText = "Update available! New version: " + updateVersion + "\nDo you want to download and install the update?";
		} catch (boss_error e) {
			updateText = "Update check failed. Details: " + e.getString();
			wxMessageBox(updateText, wxT("BOSS: Check For Updates"), wxOK | wxICON_ERROR, this);
			return;
		}
	} else {
		wxMessageBox(wxT("Update check failed. No Internet connection detected."), wxT("BOSS: Check For Updates"), wxOK | wxICON_ERROR, this);
		return;
	}

	//Display dialog BOSS telling user about update available.
	wxMessageDialog *dlg = new wxMessageDialog(this,updateText, wxT("BOSS: Check For Updates"), wxYES_NO);

	if (dlg->ShowModal() != wxID_YES) {  //User has chosen not to update. Quit now.
		//Display a message saying no update was installed.
		wxMessageBox(wxT("No update has been downloaded or installed."), wxT("BOSS: Check For Updates"), wxOK | wxICON_INFORMATION, this);
		return;
	} else  //User has chosen to update. On with the show!
		Update(updateVersion);
}

void MainFrame::Update(string updateVersion) {
	//First detect type of current install: manual or installer.
	if (fs::exists("BOSS ReadMe.lnk")) {  //Installer
		wxString message = wxT("Your current install has been determined as having been installed via the BOSS installer.\n\n");
		message += wxT("The automatic updater will download the installer for the new version to this BOSS folder.\n\n");
		message += wxT("It will then launch the installer before exiting. Complete the installer to complete the update.");
		
		wxMessageDialog *dlg = new wxMessageDialog(this,message, wxT("BOSS: Automatic Updater"), wxOK | wxCANCEL);
		if (dlg->ShowModal() != wxID_OK) {  //User has chosen to cancel. Quit now.
			wxMessageBox(wxT("Automatic updater cancelled."), wxT("BOSS: Automatic Updater"), wxOK | wxICON_EXCLAMATION, this);
			return;
		}

		wxProgressDialog *progDia = new wxProgressDialog(wxT("BOSS: Automatic Updater"),wxT("Initialising download..."), 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_ELAPSED_TIME|wxPD_CAN_ABORT);
		vector<string> fails;
		try {
			fails = DownloadInstallBOSSUpdate(progDia, INSTALLER, updateVersion);
		} catch (boss_error e) {
			progDia->Destroy();
			try {
				CleanUp();
			} catch (boss_error ee) {
				LOG_ERROR("Update clean up failed. Details: '%s'", ee.getString().c_str());
				wxMessageBox("Update failed. Details: " + ee.getString() + "\n\nUpdate cancelled.", wxT("BOSS: Automatic Updater"), wxOK | wxICON_ERROR, this);
				return;
			}
			if (e.getCode() == BOSS_ERROR_CURL_USER_CANCEL)
				wxMessageBox(wxT("Update cancelled."), wxT("BOSS: Automatic Updater"), wxOK | wxICON_INFORMATION, this);
			else
				wxMessageBox("Update failed. Details: " + e.getString() + "\n\nUpdate cancelled.", wxT("BOSS: Automatic Updater"), wxOK | wxICON_ERROR, this);
			return;
		} catch (fs::filesystem_error e) {
			progDia->Destroy();
			try {
				CleanUp();
			} catch (boss_error ee) {
				LOG_ERROR("Update clean up failed. Details: '%s'", ee.getString().c_str());
				wxMessageBox("Update failed. Details: " + ee.getString() + "\n\nUpdate cancelled.", wxT("BOSS: Automatic Updater"), wxOK | wxICON_ERROR, this);
				return;
			}
			string detail = e.what();
			wxMessageBox("Update failed. Details: " + detail + "\n\nUpdate cancelled.", wxT("BOSS: Automatic Updater"), wxOK | wxICON_ERROR, this);
			return;
		}

		if (!fails.empty()) {
			message = "There were errors renaming the downloaded files. When you click 'OK', BOSS will exit. Once BOSS has exit, remove the \".new\" extension from the following file(s), deleting any existing files with the same names, then run the downloaded installer to complete the update:\n\n";
			size_t size=fails.size();
			for (size_t i=0;i<size;i++)
				message += fails[i] +".new\n";
			wxMessageBox(wxT("New installer successfully downloaded!\n\n"+message), wxT("BOSS: Automatic Updater"), wxOK | wxICON_INFORMATION, this);
		} else {
			//Remind the user to run the uninstaller and installer.
			wxMessageBox(wxT("New installer successfully downloaded!\n\nWhen you click 'OK', BOSS will launch the downloaded installer and exit. Complete the installer to complete the update."), wxT("BOSS: Automatic Updater"), wxOK | wxICON_INFORMATION, this);

			//Now run downloaded installer then exit.
			//Although there should only be one installer file, to be safe iterate through the files vector.
			for (size_t i=0;i<updatedFiles.size();i++) {
				if (updatedFiles[i].name.empty())  //Just in case.
					continue;
				wxLaunchDefaultApplication(updatedFiles[i].name);
			}
		}
	} else {  //Manual.
		wxString message = wxT("Your current install has been determined as having been installed manually.\n\n");
		message += wxT("The automatic updater will download the updated files and replace your existing files with them.");
		message += wxT(" Your current BOSS.ini will be renamed to BOSS.ini.old. It may still be opened in your chosen text editor, allowing you to migrate your settings.");
		message += wxT(" Your current userlist.txt will not be replaced.");
		
		wxMessageDialog *dlg = new wxMessageDialog(this,message, wxT("BOSS: Automatic Updater"), wxOK | wxCANCEL);
		if (dlg->ShowModal() != wxID_OK) {  //User has chosen to cancel. Quit now.
			wxMessageBox(wxT("Automatic updater cancelled."), wxT("BOSS: Automatic Updater"), wxOK | wxICON_EXCLAMATION, this);
			return;
		}
		wxProgressDialog *progDia = new wxProgressDialog(wxT("BOSS: Automatic Updater"),wxT("Initialising download..."), 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_ELAPSED_TIME|wxPD_CAN_ABORT);
		vector<string> fails;
		try {
			fails = DownloadInstallBOSSUpdate(progDia, MANUAL, updateVersion);
		} catch (boss_error e) {
			progDia->Destroy();
			try {
				CleanUp();
			} catch (boss_error ee) {
				LOG_ERROR("Update clean up failed. Details: '%s'", ee.getString().c_str());
				wxMessageBox("Update failed. Details: " + ee.getString() + "\n\nUpdate cancelled.", wxT("BOSS: Automatic Updater"), wxOK | wxICON_ERROR, this);
				return;
			}
			if (e.getCode() == BOSS_ERROR_CURL_USER_CANCEL)
				wxMessageBox(wxT("Update cancelled."), wxT("BOSS: Automatic Updater"), wxOK | wxICON_INFORMATION, this);
			else
				wxMessageBox("Update failed. Details: " + e.getString() + "\n\nUpdate cancelled.", wxT("BOSS: Automatic Updater"), wxOK | wxICON_ERROR, this);
			return;
		} catch (fs::filesystem_error e) {
			progDia->Destroy();
			try {
				CleanUp();
			} catch (boss_error ee) {
				LOG_ERROR("Update clean up failed. Details: '%s'", ee.getString().c_str());
				wxMessageBox("Update failed. Details: " + ee.getString() + "\n\nUpdate cancelled.", wxT("BOSS: Automatic Updater"), wxOK | wxICON_ERROR, this);
				return;
			}
			string detail = e.what();
			wxMessageBox("Update failed. Details: " + detail + "\n\nUpdate cancelled.", wxT("BOSS: Automatic Updater"), wxOK | wxICON_ERROR, this);
			return;
		}

		if (!fails.empty()) {
			message = "However, the following files could not be automatically installed. When you click 'OK', BOSS will exit. After BOSS exits, remove the \".new\" extension from the following file(s), deleting any existing files with the same names to complete the update:\n\n";
			size_t size=fails.size();
			for (size_t i=0;i<size;i++) {
				message += fails[i] + ".new\n";
			}
			wxMessageBox(wxT("Files successfully downloaded!\n\n"+message), wxT("BOSS: Automatic Updater"), wxOK | wxICON_INFORMATION, this);
		} else
			wxMessageBox(wxT("Files successfully updated!\n\nWhen you click 'OK', BOSS will exit."), wxT("BOSS: Automatic Updater"), wxOK | wxICON_INFORMATION, this);
	}
	this->Close();
}

void MainFrame::OnOpenSettings(wxCommandEvent& event) {
	SettingsFrame *settings = new SettingsFrame(wxT("Better Oblivion Sorting Software: Settings"),this);
	settings->SetIcon(wxIconLocation("BOSS GUI.exe"));
	settings->Show();
}