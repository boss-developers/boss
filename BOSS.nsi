;BOSS NSIS Installer Script

;--------------------------------
;Include NSIS files.
Unicode true
	!include "MUI2.nsh"
	!include "x64.nsh"
	!include "LogicLib.nsh"
	!include "nsDialogs.nsh"

;--------------------------------
;General

	;Name, file and version info for installer.
	Name "BOSS v2.3.1"
	OutFile "BOSS Installer.exe"
	VIProductVersion 2.3.1.0

	;Request application privileges for Windows Vista/7
	RequestExecutionLevel admin

	;Icon for installer\uninstaller
	!define MUI_ICON "BOSS.ico"
	!define MUI_UNICON "BOSS.ico"

	; This causes an "are you sure?" message to be displayed if you try to quit the installer or uninstaller.
	!define MUI_ABORTWARNING
	!define MUI_UNABORTWARNING

	;Checks that the installer's CRC is correct (means we can remove installer CRC checking from BOSS).
	CRCCheck force

	;The SOLID lzma compressor gives the best compression ratio.
	SetCompressor /SOLID lzma

;--------------------------------
;Interface Settings



;--------------------------------
;Pages

	!define MUI_CUSTOMFUNCTION_GUIINIT onGUIInit

	!insertmacro MUI_PAGE_WELCOME
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES
	!define MUI_FINISHPAGE_NOAUTOCLOSE
	!define MUI_FINISHPAGE_RUN "$INSTDIR\boss.exe"
	!define MUI_FINISHPAGE_RUN_TEXT "$(Text_Run)"
	!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\Docs\BOSS ReadMe.html"
	!define MUI_FINISHPAGE_SHOWREADME_TEXT "$(Text_ShowReadme)"
	!insertmacro MUI_PAGE_FINISH

	!insertmacro MUI_UNPAGE_WELCOME
	!insertmacro MUI_UNPAGE_COMPONENTS
	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

	!insertmacro MUI_LANGUAGE "English"
	!insertmacro MUI_LANGUAGE "Russian"
	!insertmacro MUI_LANGUAGE "German"
	!insertmacro MUI_LANGUAGE "Spanish"
	!insertmacro MUI_LANGUAGE "SimpChinese"
	!insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
;English Strings

	VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "BOSS"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© 2009-2015 BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Installer for BOSS 2.3.1"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "2.3.1"

	LangString TEXT_MESSAGEBOX ${LANG_ENGLISH} "BOSS is already installed, and must be uninstalled before continuing. $\n$\nClick `OK` to remove the previous version or `Cancel` to cancel this upgrade."
	LangString TEXT_RUN ${LANG_ENGLISH} "Run BOSS"
	LangString TEXT_SHOWREADME ${LANG_ENGLISH} "View Readme"
	LangString TEXT_MAIN ${LANG_ENGLISH} "All BOSS's files, minus userlists and the BOSS.ini."
	LangString TEXT_USERFILES ${LANG_ENGLISH} "BOSS's userlist files and BOSS.ini file."

;--------------------------------
;Russian (Русский) Strings

	VIAddVersionKey /LANG=${LANG_RUSSIAN} "ProductName" "BOSS"
	VIAddVersionKey /LANG=${LANG_RUSSIAN} "CompanyName" "BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_RUSSIAN} "LegalCopyright" "© 2009-2015 BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_RUSSIAN} "FileDescription" "Установщик для BOSS 2.3.1"
	VIAddVersionKey /LANG=${LANG_RUSSIAN} "FileVersion" "2.3.1"

	LangString TEXT_MESSAGEBOX ${LANG_RUSSIAN} "BOSS уже установлен и должен быть удален перед продолжением. $\n$\nНажмите `OK` для удаления предыдущей версии или `Отмена` для отмены обновления."
	LangString TEXT_RUN ${LANG_RUSSIAN} "Запустить BOSS"
	LangString TEXT_SHOWREADME ${LANG_RUSSIAN} "Смотреть BOSS-Readme"
	LangString TEXT_MAIN ${LANG_RUSSIAN} "Все файлы BOSS, кроме пользовательских списков и BOSS.ini"
	LangString TEXT_USERFILES ${LANG_RUSSIAN} "Файлы пользовательских списков и BOSS.ini."

;--------------------------------
;German (Deutsch) Strings

	VIAddVersionKey /LANG=${LANG_GERMAN} "ProductName" "BOSS"
	VIAddVersionKey /LANG=${LANG_GERMAN} "CompanyName" "BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_GERMAN} "LegalCopyright" "© 2009-2015 BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_GERMAN} "FileDescription" "Installer für BOSS 2.3.1"
	VIAddVersionKey /LANG=${LANG_GERMAN} "FileVersion" "2.3.1"

	LangString TEXT_MESSAGEBOX ${LANG_GERMAN} "BOSS ist bereits installiert und muss deinstalliert werden, bevor fortgefahren wird. $\n$\nKlicke auf `Ok` um die vorherige Version zu entfernen oder auf `Abbrechen` um das Upgrade abzubrechen."
	LangString TEXT_RUN ${LANG_GERMAN} "BOSS starten"
	LangString TEXT_SHOWREADME ${LANG_GERMAN} "Readme lesen"
	LangString TEXT_MAIN ${LANG_GERMAN} "Alle Dateien von BOSS ohne die Benutzerlisten und die BOSS.ini."
	LangString TEXT_USERFILES ${LANG_GERMAN} "Benutzerliste von BOSS und die BOSS.ini-Datei."

;--------------------------------
;Spanish (castellano) Strings

	VIAddVersionKey /LANG=${LANG_SPANISH} "ProductName" "BOSS"
	VIAddVersionKey /LANG=${LANG_SPANISH} "CompanyName" "BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_SPANISH} "LegalCopyright" "© 2009-2015 BOSS Development Team"
	VIAddVersionKey /LANG=${LANG_SPANISH} "FileDescription" "El instalador para BOSS 2.3.1"
	VIAddVersionKey /LANG=${LANG_SPANISH} "FileVersion" "2.3.1"

	LangString TEXT_MESSAGEBOX ${LANG_SPANISH} "BOSS está instalado, y debe ser desinstalado antes de continuar. $\n$\nPresione `OK` para eliminar la versión anterior o `Cancel` para cancelar la actualización."
	LangString TEXT_RUN ${LANG_SPANISH} "Ejecutar BOSS"
	LangString TEXT_SHOWREADME ${LANG_SPANISH} "Ver Léame"
	LangString TEXT_MAIN ${LANG_SPANISH} "Todos los archivos de BOSS, menos BOSS.ini y listas de usuarios."
	LangString TEXT_USERFILES ${LANG_SPANISH} "BOSS.ini y listas de usuarios."

;--------------------------------
;Simplified Chinese (简体中文) Strings

VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "ProductName" "BOSS"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "CompanyName" "BOSS Development Team"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "LegalCopyright" "© 2009-2015 BOSS Development Team"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "FileDescription" "BOSS 2.3.1安装包"
VIAddVersionKey /LANG=${LANG_SIMPCHINESE} "FileVersion" "2.3.1"

LangString TEXT_MESSAGEBOX ${LANG_SIMPCHINESE} "检测到旧版BOSS，您需要先卸载旧版才能安装新版。$\n$\n单击“确定”卸载旧版本或者“取消”取消更新。"
LangString TEXT_RUN ${LANG_SIMPCHINESE} "运行BOSS"
LangString TEXT_SHOWREADME ${LANG_SIMPCHINESE} "查看说明"
LangString TEXT_MAIN ${LANG_SIMPCHINESE} "所有BOSS文件（除userlist和BOSS.ini）"
LangString TEXT_USERFILES ${LANG_SIMPCHINESE} "BOSS的userlist和BOSS.ini文件。"

;--------------------------------
;Variables

	Var OB_Path
	Var NE_Path
	Var SK_Path
	Var FO_Path
	Var NV_Path
	Var Empty ;An empty string.
	Var InstallPath ;Path to existing BOSS install.

;--------------------------------
;Initialisations

	Function .onInit

		!insertmacro MUI_LANGDLL_DISPLAY

		StrCpy $Empty ""

		; Look for games, setting their paths if found.
		ReadRegStr $OB_Path HKLM "Software\Bethesda Softworks\Oblivion" "Installed Path"
		${If} $OB_Path == $Empty ;Try 64 bit path.
			ReadRegStr $OB_Path HKLM "Software\Wow6432Node\Bethesda Softworks\Oblivion" "Installed Path"
		${EndIf}
		ReadRegStr $NE_Path HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Nehrim - At Fate's Edge_is1" "InstallLocation" ;No 64 bit path.
		ReadRegStr $SK_Path HKLM "Software\Bethesda Softworks\Skyrim" "Installed Path"
		${If} $SK_Path == $Empty ;Try 64 bit path.
			ReadRegStr $SK_Path HKLM "Software\Wow6432Node\Bethesda Softworks\Skyrim" "Installed Path"
		${EndIf}
		ReadRegStr $FO_Path HKLM "Software\Bethesda Softworks\Fallout3" "Installed Path"
		${If} $FO_Path == $Empty ;Try 64 bit path.
			ReadRegStr $FO_Path HKLM "Software\Wow6432Node\Bethesda Softworks\Fallout3" "Installed Path"
		${EndIf}
		ReadRegStr $NV_Path HKLM "Software\Bethesda Softworks\FalloutNV" "Installed Path"
		${If} $NV_Path == $Empty ;Try 64 bit path.
			ReadRegStr $NV_Path HKLM "Software\Wow6432Node\Bethesda Softworks\FalloutNV" "Installed Path"
		${EndIf}
		StrCpy $INSTDIR "C:\BOSS"

	FunctionEnd

	Function onGUIInit
		; Have to do this now as language isn't actually set until
		; First check to see if BOSS is already installed via installer, and launch the existing uninstaller if so.
		IfFileExists "$COMMONFILES\BOSS\uninstall.exe" 0 +9
			MessageBox MB_OKCANCEL|MB_ICONQUESTION "$(Text_MessageBox)" IDOK oldCont IDCANCEL oldCancel
			oldCancel:
				Quit
			oldCont:
				ExecWait '$COMMONFILES\BOSS\uninstall.exe _?=$COMMONFILES\BOSS' ;Run the uninstaller in its folder and wait until it's done.
				BringToFront
				Delete "$COMMONFILES\BOSS\uninstall.exe"
				RMDir "$COMMONFILES\BOSS"

		;That was the old uninstaller location, now see if the current version is already installed.
		ReadRegStr $InstallPath HKLM "Software\BOSS" "Installed Path"
		${If} $InstallPath == $Empty ;Try 64 bit path.
			ReadRegStr $InstallPath HKLM "Software\Wow6432Node\BOSS" "Installed Path"
		${EndIf}
		${If} $InstallPath != $Empty
			StrCpy $INSTDIR $InstallPath  ;Set the default install path to the previous install's path.
			IfFileExists "$InstallPath\Uninstall.exe" 0 +9
				MessageBox MB_OKCANCEL|MB_ICONQUESTION "$(Text_MessageBox)" IDOK cont IDCANCEL cancel
				cancel:
					Quit
				cont:
					ExecWait '$InstallPath\Uninstall.exe _?=$InstallPath' ;Run the uninstaller in its folder and wait until it's done.
					BringToFront
					Delete "$InstallPath\Uninstall.exe"
					RMDir "$InstallPath"
		${EndIf}
	FunctionEnd

	Function un.onInit

		!insertmacro MUI_LANGDLL_DISPLAY

	FunctionEnd

;--------------------------------
;Installer Sections
	Section "New Files"
		;Rename BOSS.ini if it exists.
		IfFileExists "$INSTDIR\BOSS.ini" 0 +3
			Delete "$INSTDIR\BOSS.ini.old"
			Rename "$INSTDIR\BOSS.ini" "$INSTDIR\BOSS.ini.old"

		;Install new BOSS ini.
		SetOutPath "$INSTDIR"
		File "data\BOSS.ini"

		;Write language ini setting to BOSS.ini. The space is there because otherwise it would be printed as =russian or whatever. Purely to look good.
		StrCmp $LANGUAGE ${LANG_RUSSIAN} 0 +2
		WriteINIStr $INSTDIR\BOSS.ini "General Settings" "sLanguage" " russian"
		StrCmp $LANGUAGE ${LANG_GERMAN} 0 +2
		WriteINIStr $INSTDIR\BOSS.ini "General Settings" "sLanguage" " german"
		StrCmp $LANGUAGE ${LANG_SPANISH} 0 +2
		WriteINIStr $INSTDIR\BOSS.ini "General Settings" "sLanguage" " spanish"
		StrCmp $LANGUAGE ${LANG_SIMPCHINESE} 0 +2
		WriteINIStr $INSTDIR\BOSS.ini "General Settings" "sLanguage" " chinese"

		;Install main executables.
		SetOutPath "$INSTDIR"
		SetRegView 32
		ReadRegStr $0 HKLM Software\Microsoft\Windows\CurrentVersion ProgramFilesDir
		StrCpy $0 $0 "" 1
		StrCmp $0 ":\Program Files (x86)" 0 +5
		SetRegView 64
		File "bin\Release-64\boss.exe"
		File "bin\Release-64\boss_gui.exe"
		Goto +4
		SetRegView 32
		File "bin\Release-32\boss.exe"
		File "bin\Release-32\boss_gui.exe"

		;Now install readme files.
		SetOutPath "$INSTDIR\Docs"
		File "Docs\BOSS Masterlist Syntax.html"
		File "Docs\BOSS ReadMe.html"
		File "Docs\BOSS Userlist Syntax.html"
		File "Docs\BOSS Version History.html"
		File "Docs\Licenses.txt"

		SetOutPath "$INSTDIR\Docs\css"
		File "Docs\css\readme.css"
		File "Docs\css\readme_aux.css"

		;Now install readme images.
		SetOutPath "$INSTDIR\Docs\images"
		File "Docs\images\GUI-Main.png"
		File "Docs\images\GUI-Select-Game.png"
		File "Docs\images\GUI-Settings.png"
		File "Docs\images\GUI-User-Rules-Manager.png"
		File "Docs\images\HTML-Log.png"
		File "Docs\images\Userlist.png"
		File "Docs\images\Ini.png"
		File "Docs\images\CLI.png"

        ;Install resource files.
        SetOutPath "$INSTDIR\resources"
		File "resources\style.css"
		File "resources\script.js"
		File "resources\octokit.js"
		File "resources\promise-1.0.0.min.js"


		;Now install language files.
		SetOutPath "$INSTDIR\resources\l10n\ru\LC_MESSAGES"
		IfFileExists "$INSTDIR\resources\l10n\ru\LC_MESSAGES\messages.mo" 0 +2
			DELETE "$INSTDIR\resources\l10n\ru\LC_MESSAGES\messages.mo"
		File "resources\l10n\ru\LC_MESSAGES\boss.mo"
		File "resources\l10n\ru\LC_MESSAGES\wxstd.mo"
		SetOutPath "$INSTDIR\resources\l10n\es\LC_MESSAGES"
		IfFileExists "$INSTDIR\resources\l10n\es\LC_MESSAGES\messages.mo" 0 +2
			DELETE "$INSTDIR\resources\l10n\es\LC_MESSAGES\messages.mo"
		File "resources\l10n\es\LC_MESSAGES\wxstd.mo"
		File "resources\l10n\es\LC_MESSAGES\boss.mo"
		SetOutPath "$INSTDIR\resources\l10n\de\LC_MESSAGES"
		File "resources\l10n\de\LC_MESSAGES\wxstd.mo"
		IfFileExists "$INSTDIR\resources\l10n\zh\*.*" 0 +2
			RENAME "$INSTDIR\resources\l10n\zh" "$INSTDIR\resources\l10n\zh_CN"
		IfFileExists "$INSTDIR\resources\l10n\zh_CN\LC_MESSAGES\messages.mo" 0 +2
			DELETE "$INSTDIR\resources\l10n\zh_CN\LC_MESSAGES\messages.mo"
		SetOutPath "$INSTDIR\resources\l10n\zh_CN\LC_MESSAGES"
		File "resources\l10n\zh_CN\LC_MESSAGES\boss.mo"
		File "resources\l10n\zh_CN\LC_MESSAGES\wxstd.mo"

		; The repositories can take a while to fetch for the first time, so by
		; bundling in that data, we reduce the first-time run time.
		; This assumes that the boss repository is beside all the masterlist
		; repositories.

		; Oblivion

		IfFileExists "$INSTDIR\Oblivion\*.*" 0 +2
			RENAME "$INSTDIR\Oblivion" "$INSTDIR\oblivion"
		SetOutPath "$INSTDIR\oblivion\.git"
		File "..\oblivion\.git\config"
		File "..\oblivion\.git\HEAD"
		File "..\oblivion\.git\index"
		File "..\oblivion\.git\packed-refs"
		SetOutPath "$INSTDIR\oblivion\.git\refs\heads"
		File "..\oblivion\.git\refs\heads\master"
		SetOutPath "$INSTDIR\oblivion\.git\refs\remotes\origin"
		File "..\oblivion\.git\refs\remotes\origin\HEAD"
		SetOutPath "$INSTDIR\oblivion\.git\objects"
		File /r "..\oblivion\.git\objects\*"

		; Skyrim
		IfFileExists "$INSTDIR\Skyrim\*.*" 0 +2
			RENAME "$INSTDIR\Skyrim" "$INSTDIR\skyrim"
		SetOutPath "$INSTDIR\skyrim\.git"
		File "..\skyrim\.git\config"
		File "..\skyrim\.git\HEAD"
		File "..\skyrim\.git\index"
		File "..\skyrim\.git\packed-refs"
		SetOutPath "$INSTDIR\skyrim\.git\refs\heads"
		File "..\skyrim\.git\refs\heads\master"
		SetOutPath "$INSTDIR\skyrim\.git\refs\remotes\origin"
		File "..\skyrim\.git\refs\remotes\origin\HEAD"
		SetOutPath "$INSTDIR\skyrim\.git\objects"
		File /r "..\skyrim\.git\objects\*"

		; Nehrim
		IfFileExists "$INSTDIR\Nehrim\*.*" 0 +2
			RENAME "$INSTDIR\Nehrim" "$INSTDIR\nehrim"
		SetOutPath "$INSTDIR\nehrim\.git"
		File "..\nehrim\.git\config"
		File "..\nehrim\.git\HEAD"
		File "..\nehrim\.git\index"
		File "..\nehrim\.git\packed-refs"
		SetOutPath "$INSTDIR\nehrim\.git\refs\heads"
		File "..\nehrim\.git\refs\heads\master"
		SetOutPath "$INSTDIR\nehrim\.git\refs\remotes\origin"
		File "..\nehrim\.git\refs\remotes\origin\HEAD"
		SetOutPath "$INSTDIR\nehrim\.git\objects"
		File /r "..\nehrim\.git\objects\*"

		; Fallout 3
		IfFileExists "$INSTDIR\Fallout 3\*.*" 0 +2
			RENAME "$INSTDIR\Fallout 3" "$INSTDIR\fallout3"
		SetOutPath "$INSTDIR\fallout3\.git"
		File "..\fallout3\.git\config"
		File "..\fallout3\.git\HEAD"
		File "..\fallout3\.git\index"
		File "..\fallout3\.git\packed-refs"
		SetOutPath "$INSTDIR\fallout3\.git\refs\heads"
		File "..\fallout3\.git\refs\heads\master"
		SetOutPath "$INSTDIR\fallout3\.git\refs\remotes\origin"
		File "..\fallout3\.git\refs\remotes\origin\HEAD"
		SetOutPath "$INSTDIR\fallout3\.git\objects"
		File /r "..\fallout3\.git\objects\*"

		; Fallout New Vegas
		IfFileExists "$INSTDIR\Fallout New Vegas\*.*" 0 +2
			RENAME "$INSTDIR\Fallout New Vegas" "$INSTDIR\falloutnv"
		SetOutPath "$INSTDIR\falloutnv\.git"
		File "..\falloutnv\.git\config"
		File "..\falloutnv\.git\HEAD"
		File "..\falloutnv\.git\index"
		File "..\falloutnv\.git\packed-refs"
		SetOutPath "$INSTDIR\falloutnv\.git\refs\heads"
		File "..\falloutnv\.git\refs\heads\master"
		SetOutPath "$INSTDIR\falloutnv\.git\refs\remotes\origin"
		File "..\falloutnv\.git\refs\remotes\origin\HEAD"
		SetOutPath "$INSTDIR\falloutnv\.git\objects"
		File /r "..\falloutnv\.git\objects\*"

		;Add Start Menu shortcuts. Set out path back to $INSTDIR otherwise the shortcuts start in the wrong place.
		;Set Shell Var Context to all so that shortcuts are installed for all users, not just admin.
		SetOutPath "$INSTDIR"
		SetShellVarContext all
		CreateDirectory "$SMPROGRAMS\BOSS"
		CreateShortCut "$SMPROGRAMS\BOSS\BOSS.lnk" "$INSTDIR\boss.exe"
		CreateShortCut "$SMPROGRAMS\BOSS\BOSS GUI.lnk" "$INSTDIR\boss_gui.exe"
		CreateShortCut "$SMPROGRAMS\BOSS\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
		CreateDirectory "$SMPROGRAMS\BOSS\Docs"
		CreateShortCut "$SMPROGRAMS\BOSS\Docs\Main ReadMe.lnk" "$INSTDIR\Docs\BOSS Readme.html"
		CreateShortCut "$SMPROGRAMS\BOSS\Docs\Userlist Syntax.lnk" "$INSTDIR\Docs\BOSS Userlist Syntax.html"
		CreateShortCut "$SMPROGRAMS\BOSS\Docs\Version History.lnk" "$INSTDIR\Docs\BOSS Version History.html"
		CreateShortCut "$SMPROGRAMS\BOSS\Docs\Masterlist Syntax.lnk" "$INSTDIR\Docs\BOSS Masterlist Syntax.html"
		CreateShortCut "$SMPROGRAMS\BOSS\Docs\Copyright Licenses.lnk" "$INSTDIR\Docs\Licenses.txt"


		;Store installation folder in registry key.
		WriteRegStr HKLM "Software\BOSS" "Installed Path" "$INSTDIR"
		;Write registry keys for Windows' uninstaller.
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "DisplayName" "BOSS"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "UninstallString" '"$INSTDIR\Uninstall.exe"'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "URLInfoAbout" 'http://boss-developers.github.io/'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "HelpLink" 'http://boss-developers.github.io/'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "Publisher" 'BOSS Development Team'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "DisplayVersion" '2.3.1'
		WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "NoModify" 1
		WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "NoRepair" 1

		;Create uninstaller
		WriteUninstaller "$INSTDIR\Uninstall.exe"

	SectionEnd

;--------------------------------
;Uninstaller Section


	Section "un.BOSS" Main

		;Remove main executables.
		Delete "$INSTDIR\boss.exe"
		Delete "$INSTDIR\boss_gui.exe"

		;Remove readme files.
		Delete "$INSTDIR\Docs\BOSS Masterlist Syntax.html"
		Delete "$INSTDIR\Docs\BOSS ReadMe.html"
		Delete "$INSTDIR\Docs\BOSS Userlist Syntax.html"
		Delete "$INSTDIR\Docs\BOSS Version History.html"
		Delete "$INSTDIR\Docs\Licenses.txt"

		Delete "Docs\css\readme.css"
		Delete "Docs\css\readme_aux.css"

		;Remove readme images.
		Delete "$INSTDIR\Docs\images\GUI-Main.png"
		Delete "$INSTDIR\Docs\images\GUI-Select-Game.png"
		Delete "$INSTDIR\Docs\images\GUI-Settings.png"
		Delete "$INSTDIR\Docs\images\GUI-User-Rules-Manager.png"
		Delete "$INSTDIR\Docs\images\HTML-Log.png"
		Delete "$INSTDIR\Docs\images\Userlist.png"
		Delete "$INSTDIR\Docs\images\Ini.png"
		Delete "$INSTDIR\Docs\images\CLI.png"

		;Remove language files.
		Delete "$INSTDIR\resources\l10n\ru\LC_MESSAGES\boss.mo"
		Delete "$INSTDIR\resources\l10n\ru\LC_MESSAGES\wxstd.mo"
		Delete "$INSTDIR\resources\l10n\es\LC_MESSAGES\boss.mo"
		Delete "$INSTDIR\resources\l10n\es\LC_MESSAGES\wxstd.mo"
		Delete "$INSTDIR\resources\l10n\de\LC_MESSAGES\wxstd.mo"
		Delete "$INSTDIR\resources\l10n\zh_CN\LC_MESSAGES\boss.mo"
		Delete "$INSTDIR\resources\l10n\zh_CN\LC_MESSAGES\wxstd.mo"

        ;Remove other resources.
		Delete "$INSTDIR\resources\style.css"
		Delete "$INSTDIR\resources\script.js"
		Delete "$INSTDIR\resources\octokit.js"
		Delete "$INSTDIR\resources\promise-1.0.0.min.js"

		;Now we have to remove the files BOSS generates when it runs.
		Delete "$INSTDIR\BOSSDebugLog.txt"
		;Trying to delete a file that doesn't exist doesn't cause an error, so delete all games' files.
		Delete "$INSTDIR\oblivion\BOSSlog.txt"
		Delete "$INSTDIR\oblivion\BOSSlog.html"
		Delete "$INSTDIR\oblivion\masterlist.txt"
		Delete "$INSTDIR\oblivion\modlist.txt"
		Delete "$INSTDIR\oblivion\modlist.old"
		Delete "$INSTDIR\nehrim\BOSSlog.txt"
		Delete "$INSTDIR\nehrim\BOSSlog.html"
		Delete "$INSTDIR\nehrim\masterlist.txt"
		Delete "$INSTDIR\nehrim\modlist.txt"
		Delete "$INSTDIR\nehrim\modlist.old"
		Delete "$INSTDIR\skyrim\BOSSlog.txt"
		Delete "$INSTDIR\skyrim\BOSSlog.html"
		Delete "$INSTDIR\skyrim\masterlist.txt"
		Delete "$INSTDIR\skyrim\modlist.txt"
		Delete "$INSTDIR\skyrim\modlist.old"
		Delete "$INSTDIR\fallout3\BOSSlog.txt"
		Delete "$INSTDIR\fallout3\BOSSlog.html"
		Delete "$INSTDIR\fallout3\masterlist.txt"
		Delete "$INSTDIR\fallout3\modlist.txt"
		Delete "$INSTDIR\fallout3\modlist.old"
		Delete "$INSTDIR\falloutnv\BOSSlog.txt"
		Delete "$INSTDIR\falloutnv\BOSSlog.html"
		Delete "$INSTDIR\falloutnv\masterlist.txt"
		Delete "$INSTDIR\falloutnv\modlist.txt"
		Delete "$INSTDIR\falloutnv\modlist.old"

		;Delete repositories.
		RMDir /r "$INSTDIR\oblivion\.git"
		RMDir /r "$INSTDIR\nehrim\.git"
		RMDir /r "$INSTDIR\skyrim\.git"
		RMDir /r "$INSTDIR\fallout3\.git"
		RMDir /r "$INSTDIR\falloutnv\.git"

		;Remove subfolders.
		RMDir "$INSTDIR\API"
		RMDir "$INSTDIR\Docs\images"
		RMDir "$INSTDIR\Docs"
		;RMDir "$INSTDIR\oblivion"
		;RMDir "$INSTDIR\nehrim"
		;RMDir "$INSTDIR\skyrim"
		;RMDir "$INSTDIR\fallout3"
		;RMDir "$INSTDIR\falloutnv"
		RMDir "$INSTDIR\resources\l10n\ru\LC_MESSAGES"
		RMDir "$INSTDIR\resources\l10n\ru"
		RMDir "$INSTDIR\resources\l10n\es\LC_MESSAGES"
		RMDir "$INSTDIR\resources\l10n\es"
		RMDir "$INSTDIR\resources\l10n\de\LC_MESSAGES"
		RMDir "$INSTDIR\resources\l10n\de"
		RMDir "$INSTDIR\resources\l10n\zh_CN\LC_MESSAGES"
		RMDir "$INSTDIR\resources\l10n\zh_CN"
		RMDir "$INSTDIR\resources\l10n"
		RMDir "$INSTDIR\resources"

		;Remove uninstaller.
		Delete "$INSTDIR\Uninstall.exe"

		;Remove install directory.
		;RMDir "$INSTDIR"

		;Delete registry key.
		DeleteRegKey HKLM "Software\BOSS"

		;Delete stupid Windows created registry keys:
		DeleteRegKey HKCU "Software\BOSS"
		DeleteRegKey HKLM "Software\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\BOSS"
		DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS"
		DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\App Management\ARPCache\BOSS"
		DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR"
		DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\boss.exe"
		DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\boss_gui.exe"
		DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\Uninstall.exe"
		DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR"
		DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\boss.exe"
		DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\boss_gui.exe"
		DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$INSTDIR\Uninstall.exe"
		DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$INSTDIR"
		DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$INSTDIR\boss.exe"
		DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$INSTDIR\boss_gui.exe"
		DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$INSTDIR\Uninstall.exe"

		;Delete Start Menu folder.
		SetShellVarContext all
		RMDir /r "$SMPROGRAMS\BOSS"

	SectionEnd

	Section /o "un.User Files" UserFiles
		;The following user files are only removed if set to.
		Delete "$INSTDIR\BOSS.ini"
		Delete "$INSTDIR\BOSS.ini.old"
		Delete "$INSTDIR\oblivion\userlist.txt"
		Delete "$INSTDIR\nehrim\userlist.txt"
		Delete "$INSTDIR\skyrim\userlist.txt"
		Delete "$INSTDIR\fallout3\userlist.txt"
		Delete "$INSTDIR\falloutnv\userlist.txt"
		;Also try removing the folders storing them, in case they are otherwise empty.
		RMDir "$INSTDIR\oblivion"
		RMDir "$INSTDIR\nehrim"
		RMDir "$INSTDIR\skyrim"
		RMDir "$INSTDIR\fallout3"
		RMDir "$INSTDIR\falloutnv"
		;Try removing install directory.
		RMDir "$INSTDIR"
	SectionEnd

;--------------------------------
;Languages - Description Strings

	!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Main} "$(Text_Main)"
	!insertmacro MUI_DESCRIPTION_TEXT ${UserFiles} "$(Text_UserFiles)"
	!insertmacro MUI_UNFUNCTION_DESCRIPTION_END

;--------------------------------
