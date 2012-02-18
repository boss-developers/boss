;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !include "LogicLib.nsh"
  !include "nsDialogs.nsh"

;--------------------------------
;General

  ;Name, file and version info for installer.
  Name "BOSS v1.9.2"
  OutFile "BOSS Installer.exe"
  VIProductVersion 1.9.1.0
  VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "BOSS"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "BOSS Development Team"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© 2009-2012 BOSS Development Team"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Installer for BOSS 1.9.2"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "1.9.2"

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
;Interface Settings

  ; This causes an "are you sure?" message to be displayed if you try to quit the installer.
  !define MUI_ABORTWARNING
  
;--------------------------------
;Variables


Var OB_Path
Var NE_Path
Var SK_Path
Var FO_Path
Var NV_Path
Var GameNum
Var Empty ;An empty string.
Var InstallPath ;Path to existing BOSS install.

Var Dialog
Var Check_Run
Var CheckState_Run
Var Check_Readme

Var Check_RemoveUserFiles
Var CheckState_RemoveUserFiles
  
;--------------------------------
;Initialise Install Path
Function .onInit
; First check to see if BOSS is already installed via installer, and launch the existing uninstaller if so.

ReadRegStr $InstallPath HKLM "Software\BOSS" "Installed Path"
${If} $InstallPath != ""
	IfFileExists "$InstallPath\Uninstall.exe" 0 +8
		MessageBox MB_OKCANCEL|MB_ICONQUESTION "BOSS is already installed, and must be uninstalled before continuing. $\n$\nClick `OK` to remove the previous version or `Cancel` to cancel this upgrade." IDOK cont IDCANCEL cancel
		cancel:
			Quit
		cont:
			ExecWait '$InstallPath\Uninstall.exe _?=$InstallPath' ;Run the uninstaller in its folder and wait until it's done.
		Delete "$InstallPath\Uninstall.exe"
		RMDir "$InstallPath"
${EndIf}


; Look for games, counting them.
StrCpy $Empty ""
ReadRegStr $OB_Path HKLM "Software\Bethesda Softworks\Oblivion" "Installed Path"
${If} $OB_Path == $Empty ;Try 64 bit path.
	ReadRegStr $OB_Path HKLM "Software\Wow6432Node\Bethesda Softworks\Oblivion" "Installed Path"
${EndIf}
${If} $OB_Path != $Empty
	StrCpy $INSTDIR $OB_Path
	IntOp $GameNum $GameNum + 1
${EndIf}
ReadRegStr $NE_Path HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Nehrim - At Fate's Edge_is1" "InstallLocation" ;No 64 bit path.
${If} $NE_Path != $Empty
	StrCpy $INSTDIR $NE_Path
	IntOp $GameNum $GameNum + 1
${EndIf}
ReadRegStr $SK_Path HKLM "Software\Bethesda Softworks\Skyrim" "Installed Path"
${If} $SK_Path == $Empty ;Try 64 bit path.
	ReadRegStr $SK_Path HKLM "Software\Wow6432Node\Bethesda Softworks\Skyrim" "Installed Path"
${EndIf}
${If} $SK_Path != $Empty
	StrCpy $INSTDIR $SK_Path
	IntOp $GameNum $GameNum + 1
${EndIf}
ReadRegStr $FO_Path HKLM "Software\Bethesda Softworks\Fallout3" "Installed Path"
${If} $FO_Path == $Empty ;Try 64 bit path.
	ReadRegStr $FO_Path HKLM "Software\Wow6432Node\Bethesda Softworks\Fallout3" "Installed Path"
${EndIf}
${If} $FO_Path != $Empty
	StrCpy $INSTDIR $FO_Path
	IntOp $GameNum $GameNum + 1
${EndIf}
ReadRegStr $NV_Path HKLM "Software\Bethesda Softworks\FalloutNV" "Installed Path"
${If} $NV_Path == $Empty ;Try 64 bit path.
	ReadRegStr $NV_Path HKLM "Software\Wow6432Node\Bethesda Softworks\FalloutNV" "Installed Path"
${EndIf}
${If} $NV_Path != $Empty
	StrCpy $INSTDIR $NV_Path
	IntOp $GameNum $GameNum + 1
${EndIf}
;All games looked for. If only one game exists, then INSTDIR will be set to its location. 
;If > 1 game exists, INSTDIR will be set to the last game detected's location. Set this instead to C:\.
${If} $GameNum != 1
	StrCpy $INSTDIR "C:\"
${EndIf}
StrCpy $INSTDIR "$INSTDIR\BOSS"
FunctionEnd

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "data\boss-common\Licenses.txt"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  Page custom PAGE_FINISH PAGE_FINISH_Leave
  
  !insertmacro MUI_UNPAGE_WELCOME
  UninstPage custom un.PAGE_SELECT_OPTIONS un.PAGE_SELECT_OPTIONS_Leave
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  LangString unPAGE_SELECT_GAMES_TITLE ${LANG_ENGLISH} "Choose Options"
  LangString unPAGE_SELECT_GAMES_SUBTITLE ${LANG_ENGLISH} "Please select from the following uninstall options."
  LangString PAGE_FINISH_TITLE ${LANG_ENGLISH} "Finished installing BOSS v1.9.2."
  LangString PAGE_FINISH_SUBTITLE ${LANG_ENGLISH} "Please select post-install tasks."
  

;-------------------------------- Custom Installation Pages and their Functions:
    Function PAGE_FINISH
        !insertmacro MUI_HEADER_TEXT $(PAGE_FINISH_TITLE) $(PAGE_FINISH_SUBTITLE)
        
        nsDialogs::Create 1018
            Pop $Dialog

        ${If} $Dialog == error
            Abort
        ${EndIf}
        
        IntOp $0 0 + 0
		${NSD_CreateCheckBox} 0 $0u 100% 8u "Run BOSS"
			Pop $Check_Run
			${NSD_AddStyle} $Check_Run ${WS_GROUP}
			${NSD_SetState} $Check_Run $CheckState_Run
		IntOp $0 $0 + 15
        IntOp $1 0 + 0
        IfFileExists "$INSTDIR\Docs\BOSS ReadMe.html" 0 +6
            ${NSD_CreateCheckBox} $1% $0u 25% 8u "View Readme"
                Pop $Check_Readme
                ${NSD_AddStyle} $Check_Readme ${WS_GROUP}
                ${NSD_SetState} $Check_Readme ${BST_CHECKED}
                IntOp $1 $1 + 25
		nsDialogs::Show		
    FunctionEnd
    Function PAGE_FINISH_Leave
		${NSD_GetState} $Check_Run $CheckState_Run
        ${If} $CheckState_Run == ${BST_CHECKED}
			SetOutPath "$INSTDIR"
            Exec '"$INSTDIR\BOSS.exe"'
        ${EndIf}
        ${NSD_GetState} $Check_Readme $0
        ${If} $0 == ${BST_CHECKED}
            SetOutPath "$INSTDIR"
            ExecShell "open" '"$INSTDIR\Docs\BOSS ReadMe.html"'
        ${EndIf}
		
    FunctionEnd

;-------------------------------- Custom Uninstallation Pages and their Functions:
    Function un.PAGE_SELECT_OPTIONS
        !insertmacro MUI_HEADER_TEXT $(unPAGE_SELECT_GAMES_TITLE) $(unPAGE_SELECT_GAMES_SUBTITLE)
     ;   GetFunctionAddress $unFunction_Browse un.OnClick_Browse
        
        nsDialogs::Create 1018
            Pop $Dialog

        ${If} $Dialog == error
            Abort
        ${EndIf}
        
        
        ${NSD_CreateCheckBox} 0 $0u 100% 13u "Uninstall userlist and ini file if they exist."
            Pop $Check_RemoveUserFiles
        nsDialogs::Show
    FunctionEnd
    Function un.PAGE_SELECT_OPTIONS_Leave
        ${NSD_GetState} $Check_RemoveUserFiles $CheckState_RemoveUserFiles
    FunctionEnd

;--------------------------------
;Installer Sections

Section "Installer Section"

	;Install main executables first.
	SetOutPath "$INSTDIR"
	File "code\boss-cli\trunk\bin\Release-32\BOSS.exe"
	File "code\boss-gui\trunk\bin\Release-32\BOSS GUI.exe"
	
	;Silently move files from past BOSS installs.
	IfFileExists "$OB_Path\Data\BOSS\userlist.txt" 0 +3
		CreateDirectory "$INSTDIR\Oblivion"
		Rename "$OB_Path\Data\BOSS\userlist.txt" "$INSTDIR\Oblivion\userlist.txt"
	IfFileExists "$NE_Path\Data\BOSS\userlist.txt" 0 +3
		CreateDirectory "$INSTDIR\Nehrim"
		Rename "$NE_Path\Data\BOSS\userlist.txt" "$INSTDIR\Nehrim\userlist.txt"
	IfFileExists "$FO_Path\Data\BOSS\userlist.txt" 0 +3
		CreateDirectory "$INSTDIR\Fallout 3"
		Rename "$FO_Path\Data\BOSS\userlist.txt" "$INSTDIR\Fallout 3\userlist.txt"
	IfFileExists "$NV_Path\Data\BOSS\userlist.txt" 0 +3
		CreateDirectory "$INSTDIR\Fallout New Vegas"
		Rename "$NV_Path\Data\BOSS\userlist.txt" "$INSTDIR\Fallout New Vegas\userlist.txt"
	IfFileExists "$OB_Path\BOSS\userlist.txt" 0 +3
		CreateDirectory "$INSTDIR\Oblivion"
		Rename "$OB_Path\BOSS\userlist.txt" "$INSTDIR\Oblivion\userlist.txt"
	IfFileExists "$NE_Path\BOSS\userlist.txt" 0 +3
		CreateDirectory "$INSTDIR\Nehrim"
		Rename "$NE_Path\BOSS\userlist.txt" "$INSTDIR\Nehrim\userlist.txt"
	IfFileExists "$SK_Path\BOSS\userlist.txt" 0 +3
		CreateDirectory "$INSTDIR\Skyrim"
		Rename "$SK_Path\BOSS\userlist.txt" "$INSTDIR\Skyrim\userlist.txt"
	IfFileExists "$FO_Path\BOSS\userlist.txt" 0 +3
		CreateDirectory "$INSTDIR\Fallout 3"
		Rename "$FO_Path\BOSS\userlist.txt" "$INSTDIR\Fallout 3\userlist.txt"
	IfFileExists "$NV_Path\BOSS\userlist.txt" 0 +3
		CreateDirectory "$INSTDIR\Fallout New Vegas"
		Rename "$NV_Path\BOSS\userlist.txt" "$INSTDIR\Fallout New Vegas\userlist.txt"
	IfFileExists "$OB_Path\BOSS\BOSS.ini" 0 +2
		Rename "$OB_Path\BOSS\BOSS.ini" "$INSTDIR\BOSS.ini"
	IfFileExists "$NE_Path\BOSS\BOSS.ini" 0 +2
		Rename "$NE_Path\BOSS\BOSS.ini" "$INSTDIR\BOSS.ini"
	IfFileExists "$SK_Path\BOSS\BOSS.ini" 0 +2
		Rename "$SK_Path\BOSS\BOSS.ini" "$INSTDIR\BOSS.ini"
	IfFileExists "$FO_Path\BOSS\BOSS.ini" 0 +2
		Rename "$FO_Path\BOSS\BOSS.ini" "$INSTDIR\BOSS.ini"
	IfFileExists "$NV_Path\BOSS\BOSS.ini" 0 +2
		Rename "$NV_Path\BOSS\BOSS.ini" "$INSTDIR\BOSS.ini"
	;Now remove files from past BOSS installations.
	 ${If} $OB_Path != $Empty
		Delete "$OB_Path\Data\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
		Delete "$OB_Path\Data\modlist.*"
		Delete "$OB_Path\Data\masterlist.txt"
		Delete "$OB_Path\Data\BOSS\modlist.*"
		Delete "$OB_Path\Data\BOSS\masterlist.txt"
		Delete "$OB_Path\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
		RMDir  "$OB_Path\Data\BOSS"
	${EndIf}
	${If} $NE_Path != $Empty
		Delete "$NE_Path\Data\BOSS*"
		Delete "$NE_Path\Data\modlist.*"
		Delete "$NE_Path\Data\masterlist.txt"
		Delete "$NE_Path\Data\BOSS\modlist.*"
		Delete "$NE_Path\Data\BOSS\masterlist.txt"
		Delete "$NE_Path\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
		RMDir  "$NE_Path\Data\BOSS"
	${EndIf}
	${If} $SK_Path != $Empty
		Delete "$SK_Path\Data\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
		Delete "$SK_Path\Data\modlist.*"
		Delete "$SK_Path\Data\masterlist.txt"
		Delete "$SK_Path\Data\BOSS\modlist.*"
		Delete "$SK_Path\Data\BOSS\masterlist.txt"
		Delete "$SK_Path\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
		RMDir  "$SK_Path\Data\BOSS"
	${EndIf}
	${If} $FO_Path != $Empty
		Delete "$FO_Path\Data\BOSS*"
		Delete "$FO_Path\Data\modlist.*"
		Delete "$FO_Path\Data\masterlist.txt"
		Delete "$FO_Path\Data\BOSS\modlist.*"
		Delete "$FO_Path\Data\BOSS\masterlist.txt"
		Delete "$FO_Path\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
		RMDir  "$FO_Path\Data\BOSS"
	${EndIf}
	${If} $NV_Path != $Empty
		Delete "$NV_Path\Data\BOSS*"
		Delete "$NV_Path\Data\modlist.*"
		Delete "$NV_Path\Data\masterlist.txt"
		Delete "$NV_Path\Data\BOSS\modlist.*"
		Delete "$NV_Path\Data\BOSS\masterlist.txt"
		Delete "$NV_Path\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
		RMDir  "$NV_Path\Data\BOSS"
	${EndIf}
	

	;Rename BOSS.ini if it exists.
	IfFileExists "BOSS.ini" 0 +3
		Delete "BOSS.ini.old"
		Rename "BOSS.ini" "BOSS.ini.old"
  
	;Now install API DLLs.
	SetOutPath "$INSTDIR\API"
	File "code\boss-api\trunk\bin\Release-32\boss32.dll"
	File "code\boss-api\trunk\bin\Release-64\boss64.dll"
	
	;Now install readme files.
	SetOutPath "$INSTDIR\Docs"
	File "data\boss-common\BOSS API ReadMe.html"
	File "data\boss-common\BOSS Masterlist Syntax.html"
	File "data\boss-common\BOSS ReadMe.html"
	File "data\boss-common\BOSS User Rules ReadMe.html"
	File "data\boss-common\Licenses.txt"
	
	;Now install readme images.
	SetOutPath "$INSTDIR\Docs\images"
	File "data\boss-common\images\BOSS GUI Main.png"
	File "data\boss-common\images\BOSS GUI Settings 1.png"
	File "data\boss-common\images\BOSS GUI Settings 2.png"
	File "data\boss-common\images\BOSS GUI Settings 3.png"
	File "data\boss-common\images\BOSS GUI Settings 4.png"
	File "data\boss-common\images\BOSS GUI Settings 5.png"
	File "data\boss-common\images\BOSS GUI User Rules Editor.png"
	
	;Add Start Menu shortcuts.
	CreateDirectory "$SMPROGRAMS\BOSS"
	CreateShortCut "$SMPROGRAMS\BOSS\BOSS.lnk" "$INSTDIR\BOSS.exe"
	CreateShortCut "$SMPROGRAMS\BOSS\BOSS GUI.lnk" "$INSTDIR\BOSS GUI.exe"
	CreateShortCut "$SMPROGRAMS\BOSS\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	CreateDirectory "$SMPROGRAMS\BOSS\Docs"
	 CreateShortCut "$SMPROGRAMS\BOSS\Docs\Main ReadMe.lnk" "$INSTDIR\Docs\BOSS Readme.html"
	CreateShortCut "$SMPROGRAMS\BOSS\Docs\User Rules ReadMe.lnk" "$INSTDIR\Docs\BOSS User Rules ReadMe.html"
	CreateShortCut "$SMPROGRAMS\BOSS\Docs\API ReadMe.lnk" "$INSTDIR\Docs\BOSS API ReadMe.html"
	CreateShortCut "$SMPROGRAMS\BOSS\Docs\Masterlist Syntax Doc.lnk" "$INSTDIR\Docs\BOSS Masterlist Syntax.html"
	CreateShortCut "$SMPROGRAMS\BOSS\Docs\Copyright Licenses.lnk" "$INSTDIR\Docs\Licenses.txt"
  
	;Store installation folder in registry key.
	WriteRegStr HKLM "Software\BOSS" "Installed Path" $INSTDIR
	
	;Write registry keys for Windows' uninstaller.
	WriteRegStr HKLM "Software\BOSS" "Installer Path" "$EXEPATH"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "DisplayName" "BOSS"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "UninstallString" '"$INSTDIR\Uninstall.exe"'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "URLInfoAbout" 'http://better-oblivion-sorting-software.googlecode.com/'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "HelpLink" 'http://better-oblivion-sorting-software.googlecode.com/'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "Publisher" 'BOSS Development Team'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "DisplayVersion" '1.9.2'      
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "NoRepair" 1

	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

	;Change this to optionally remove userlist and BOSS.ini, rather than by default.

	;Remove main executables.
	Delete "$INSTDIR\BOSS.exe"
	Delete "$INSTDIR\BOSS GUI.exe"
	
	;Remove API DLLs.
	Delete "$INSTDIR\API\boss32.dll"
	Delete "$INSTDIR\API\boss64.dll"
	
	;Remove readme files.
	Delete "$INSTDIR\Docs\BOSS API ReadMe.html"
	Delete "$INSTDIR\Docs\BOSS Masterlist Syntax.html"
	Delete "$INSTDIR\Docs\BOSS ReadMe.html"
	Delete "$INSTDIR\Docs\BOSS User Rules ReadMe.html"
	Delete "$INSTDIR\Docs\Licenses.txt"
	
	;Remove readme images.
	Delete "$INSTDIR\Docs\images\BOSS GUI Main.png"
	Delete "$INSTDIR\Docs\images\BOSS GUI Settings 1.png"
	Delete "$INSTDIR\Docs\images\BOSS GUI Settings 2.png"
	Delete "$INSTDIR\Docs\images\BOSS GUI Settings 3.png"
	Delete "$INSTDIR\Docs\images\BOSS GUI Settings 4.png"
	Delete "$INSTDIR\Docs\images\BOSS GUI Settings 5.png"
	Delete "$INSTDIR\Docs\images\BOSS GUI User Rules Editor.png"
	
	;Now we have to remove the files BOSS generates when it runs.
	Delete "$INSTDIR\BOSSDebugLog.txt"
	;Trying to delete a file that doesn't exist doesn't cause an error, so delete all games' files.
	Delete "$INSTDIR\Oblivion\BOSSlog.txt"
	Delete "$INSTDIR\Oblivion\BOSSlog.html"
	Delete "$INSTDIR\Oblivion\masterlist.txt"
	Delete "$INSTDIR\Oblivion\modlist.txt"
	Delete "$INSTDIR\Oblivion\modlist.old"
	Delete "$INSTDIR\Nehrim\BOSSlog.txt"
	Delete "$INSTDIR\Nehrim\BOSSlog.html"
	Delete "$INSTDIR\Nehrim\masterlist.txt"
	Delete "$INSTDIR\Nehrim\modlist.txt"
	Delete "$INSTDIR\Nehrim\modlist.old"
	Delete "$INSTDIR\Skyrim\BOSSlog.txt"
	Delete "$INSTDIR\Skyrim\BOSSlog.html"
	Delete "$INSTDIR\Skyrim\masterlist.txt"
	Delete "$INSTDIR\Skyrim\modlist.txt"
	Delete "$INSTDIR\Skyrim\modlist.old"
	Delete "$INSTDIR\Fallout 3\BOSSlog.txt"
	Delete "$INSTDIR\Fallout 3\BOSSlog.html"
	Delete "$INSTDIR\Fallout 3\masterlist.txt"
	Delete "$INSTDIR\Fallout 3\modlist.txt"
	Delete "$INSTDIR\Fallout 3\modlist.old"
	Delete "$INSTDIR\Fallout New Vegas\BOSSlog.txt"
	Delete "$INSTDIR\Fallout New Vegas\BOSSlog.html"
	Delete "$INSTDIR\Fallout New Vegas\masterlist.txt"
	Delete "$INSTDIR\Fallout New Vegas\modlist.txt"
	Delete "$INSTDIR\Fallout New Vegas\modlist.old"
	;The following user files are only removed if set to.
	${If} $CheckState_RemoveUserFiles == ${BST_CHECKED}
		Delete "$INSTDIR\BOSS.ini"
		Delete "$INSTDIR\BOSS.ini.old"
		Delete "$INSTDIR\Oblivion\userlist.txt"
		Delete "$INSTDIR\Nehrim\userlist.txt"
		Delete "$INSTDIR\Skyrim\userlist.txt"
		Delete "$INSTDIR\Fallout 3\userlist.txt"
		Delete "$INSTDIR\Fallout New Vegas\userlist.txt"
	${EndIf}
	
	;Remove subfolders.
	RMDir "$INSTDIR\API"
	RMDir "$INSTDIR\Docs\images"
	RMDir "$INSTDIR\Docs"
	RMDir "$INSTDIR\Oblivion"
	RMDir "$INSTDIR\Nehrim"
	RMDir "$INSTDIR\Skyrim"
	RMDir "$INSTDIR\Fallout 3"
	RMDir "$INSTDIR\Fallout New Vegas"
	
	;Remove uninstaller.
	Delete "$INSTDIR\Uninstall.exe"

	;Remove install directory.
	RMDir "$INSTDIR"

	;Delete registry key.
	DeleteRegKey HKLM "Software\BOSS"
	
	;Delete stupid Windows created registry keys:
	DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\App Management\ARPCache\BOSS"
	DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR"
	DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\BOSS.exe"
	DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\BOSS GUI.exe"
	DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\Uninstall.exe"
	DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR"
	DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\BOSS.exe"
	DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\BOSS GUI.exe"
	DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\Uninstall.exe"
	DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$INSTDIR"
	DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$INSTDIR\BOSS.exe"
	DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$INSTDIR\BOSS GUI.exe"
	DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$INSTDIR\Uninstall.exe"
	
	;Delete Start Menu folder.
	RMDir /r "$SMPROGRAMS\BOSS"

SectionEnd