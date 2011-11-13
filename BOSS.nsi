; BOSS.nsi
;-------------------------------- Includes:
  !include MUI2.nsh
  !include LogicLib.nsh
  !include nsDialogs.nsh
;-------------------------------- Basic Installer Info:
    Name "BOSS v1.9.0"
    OutFile "BOSS Installer.exe"
    ; Request application privileges for Windows Vista
    RequestExecutionLevel admin
    VIProductVersion 1.9.0.0
    VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "BOSS"
    VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "BOSS Development Team"
    VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© BOSS Development Team"
    VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Installer for BOSS 1.9.0"
    VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "1.9.0.0"
;-------------------------------- Variables:
    Var Dialog
    Var Label
    Var Path_OB
    Var Path_FO
    Var Path_NV
    Var Path_Other
    Var Path_Nehrim
	Var Path_SK
    Var Empty
    Var Check_OB
    Var Check_FO
    Var Check_NV
    Var Check_Nehrim
	Var Check_SK
    Var Check_Other
    Var Check_RemoveUserFiles
    Var CheckState_OB
    Var CheckState_FO
    Var CheckState_NV
    Var CheckState_Nehrim
	Var CheckState_SK
    Var CheckState_Other
    Var CheckState_RemoveUserFiles
    Var PathDialogue_OB
    Var PathDialogue_FO
    Var PathDialogue_NV
    Var PathDialogue_Nehrim
	Var PathDialogue_SK
    Var PathDialogue_Other
    Var Browse_OB
    Var Browse_FO
    Var Browse_NV
    Var Browse_Nehrim
	Var Browse_SK
    Var Browse_Other
    Var Check_Readme
    Var Check_DeleteOldFiles
    Var Function_Browse
    Var Function_DirPrompt
    Var unFunction_Browse
;-------------------------------- Page List:
  !insertmacro MUI_PAGE_WELCOME
  Page custom PAGE_SELECT_GAMES PAGE_SELECT_GAMES_Leave
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_INSTFILES
  Page custom PAGE_FINISH PAGE_FINISH_Leave
  
  !insertmacro MUI_UNPAGE_WELCOME
  UninstPage custom un.PAGE_SELECT_GAMES un.PAGE_SELECT_GAMES_Leave
  !insertmacro MUI_UNPAGE_INSTFILES
;-------------------------------- Initialize Variables as required:
    Function un.onInit
        ReadRegStr $Path_OB HKLM "Software\BOSS" "Oblivion Path"
        ReadRegStr $Path_FO HKLM "Software\BOSS" "Fallout3 Path"
        ReadRegStr $Path_NV HKLM "Software\BOSS" "NewVegas Path"
        ReadRegStr $Path_Other HKLM "Software\BOSS" "Other Path"
        ReadRegStr $Path_Nehrim HKLM "Software\BOSS" "Nehrim Path"
		ReadRegStr $Path_SK HKLM "Software\BOSS" "Skyrim Path"
        FunctionEnd
    Function .onInit
        StrCpy $Empty ""
        ReadRegStr $Path_OB HKLM "Software\BOSS" "Oblivion Path"
        ReadRegStr $Path_FO HKLM "Software\BOSS" "Fallout3 Path"
        ReadRegStr $Path_NV HKLM "Software\BOSS" "NewVegas Path"
        ReadRegStr $Path_Other HKLM "Software\BOSS" "Other Path"
        ReadRegStr $Path_Nehrim HKLM "Software\BOSS" "Nehrim Path"
		ReadRegStr $Path_SK HKLM "Software\BOSS" "Skyrim Path"
		
		# Need to check if it is already installed, then launch that uninstaller if so.
		# Check if the uninstaller exists.
		IfFileExists "$COMMONFILES\BOSS\uninstall.exe" 0 +6
			MessageBox MB_OKCANCEL|MB_ICONQUESTION "BOSS is already installed, and must be uninstalled before continuing. $\n$\nClick `OK` to remove the previous version or `Cancel` to cancel this upgrade." IDOK cont IDCANCEL cancel
			cancel:
				Quit
			cont:
			ExecWait '$COMMONFILES\BOSS\uninstall.exe _?=$COMMONFILES\BOSS' ;Run the uninstaller in its folder and wait until it's done.
			
		#Now delete the uninstaller.
		IfFileExists "$COMMONFILES\BOSS\uninstall.exe" 0 +3
			Delete "$COMMONFILES\BOSS\uninstall.exe"
			RMDir "$COMMONFILES\BOSS"

        ${If} $Path_OB == $Empty
            ReadRegStr $Path_OB HKLM "Software\Bethesda Softworks\Oblivion" "Installed Path"
            ${If} $Path_OB == $Empty
                ReadRegStr $Path_OB HKLM "SOFTWARE\Wow6432Node\Bethesda Softworks\Oblivion" "Installed Path"
            ${EndIf}
        ${Else}
            StrCpy $CheckState_OB ${BST_CHECKED}
        ${EndIf}
        ${If} $Path_FO == $Empty
            ReadRegStr $Path_FO HKLM "Software\Bethesda Softworks\Fallout3" "Installed Path"
            ${If} $Path_FO == $Empty
                ReadRegStr $Path_FO HKLM "SOFTWARE\Wow6432Node\Bethesda Softworks\Fallout3" "Installed Path"
            ${EndIf}
        ${Else}
            StrCpy $CheckState_FO ${BST_CHECKED}
        ${EndIf}
        ${If} $Path_NV == $Empty
            ReadRegStr $Path_NV HKLM "Software\Bethesda Softworks\FalloutNV" "Installed Path"
            ${If} $Path_NV == $Empty
                ReadRegStr $Path_NV HKLM "SOFTWARE\Wow6432Node\Bethesda Softworks\FalloutNV" "Installed Path"
            ${EndIf}
        ${Else}
            StrCpy $CheckState_NV ${BST_CHECKED}
        ${EndIf}
        ${If} $Path_Nehrim == $Empty
            ReadRegStr $Path_Nehrim HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Nehrim - At Fate's Edge_is1" "InstallLocation"
        ${Else}
            StrCpy $CheckState_Nehrim ${BST_CHECKED}
        ${EndIf}
		${If} $Path_SK == $Empty
            ReadRegStr $Path_SK HKLM "Software\Bethesda Softworks\Skyrim" "Installed Path"
            ${If} $Path_SK == $Empty
                ReadRegStr $Path_SK HKLM "SOFTWARE\Wow6432Node\Bethesda Softworks\Skyrim" "Installed Path"
            ${EndIf}
        ${Else}
            StrCpy $CheckState_SK ${BST_CHECKED}
        ${EndIf}
        ${If} $Path_Other != $Empty
            StrCpy $CheckState_Other ${BST_CHECKED}
        ${EndIf}
        FunctionEnd
;-------------------------------- Custom Installation Pages and their Functions:
    Function PAGE_SELECT_GAMES
        !insertmacro MUI_HEADER_TEXT $(PAGE_SELECT_GAMES_TITLE) $(PAGE_SELECT_GAMES_SUBTITLE)
        GetFunctionAddress $Function_Browse OnClick_Browse
        
        nsDialogs::Create 1018
            Pop $Dialog

        ${If} $Dialog == error
            Abort
        ${EndIf}
        
        ${NSD_CreateLabel} 0 0 100% 8u "Please select which game(s) to install BOSS for:"
            Pop $Label
        
        IntOp $0 0 + 9
        ${If} $Path_OB != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 13u "&Oblivion"
                Pop $Check_OB
                ${NSD_SetState} $Check_OB $CheckState_OB
            IntOp $0 $0 + 13
            ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_OB"
                Pop $PathDialogue_OB
            ${NSD_CreateBrowseButton} -10% $0u 5% 13u "..."
                Pop $Browse_OB
                nsDialogs::OnClick $Browse_OB $Function_Browse
            IntOp $0 $0 + 13
        ${EndIf}
        ${If} $Path_FO != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 13u "&Fallout 3"
                Pop $Check_FO
                ${NSD_SetState} $Check_FO $CheckState_FO
            IntOp $0 $0 + 13
            ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_FO"
                Pop $PathDialogue_FO
            ${NSD_CreateBrowseButton} -10% 48u 5% 13u "..."
                Pop $Browse_FO
                nsDialogs::OnClick $Browse_FO $Function_Browse
            IntOp $0 $0 + 13
        ${EndIf}
        ${If} $Path_NV != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 13u "Fallout: New &Vegas"
                Pop $Check_NV
                ${NSD_SetState} $Check_NV $CheckState_NV
            IntOp $0 $0 + 13
            ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_NV"
                Pop $PathDialogue_NV
            ${NSD_CreateBrowseButton} -10% $0u 5% 13u "..."
                Pop $Browse_NV
                nsDialogs::OnClick $Browse_NV $Function_Browse
            IntOp $0 $0 + 13
        ${EndIf}
        ${If} $Path_Nehrim != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 13u "Nehrim"
                Pop $Check_Nehrim
                ${NSD_SetState} $Check_Nehrim $CheckState_Nehrim
            IntOp $0 $0 + 13
            ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_Nehrim"
                Pop $PathDialogue_Nehrim
            ${NSD_CreateBrowseButton} -10% $0u 5% 13u "..."
                Pop $Browse_Nehrim
                nsDialogs::OnClick $Browse_Nehrim $Function_Browse
            IntOp $0 $0 + 13
        ${EndIf}
		${If} $Path_SK != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 13u "&Skyrim"
                Pop $Check_SK
                ${NSD_SetState} $Check_SK $CheckState_SK
            IntOp $0 $0 + 13
            ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_SK"
                Pop $PathDialogue_SK
            ${NSD_CreateBrowseButton} -10% $0u 5% 13u "..."
                Pop $Browse_SK
                nsDialogs::OnClick $Browse_SK $Function_Browse
            IntOp $0 $0 + 13
        ${EndIf}
        ;------Path_Other:
            ${NSD_CreateCheckBox} 0 $0u 100% 13u "Other Location (e.g. an undetected game)."
                Pop $Check_Other
                ${NSD_SetState} $Check_Other $CheckState_Other
            IntOp $0 $0 + 13
            ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_Other"
                Pop $PathDialogue_Other
            ${NSD_CreateBrowseButton} -10% $0u 5% 13u "..."
                Pop $Browse_Other
                nsDialogs::OnClick $Browse_Other $Function_Browse
        nsDialogs::Show
        FunctionEnd
    Function PAGE_SELECT_GAMES_Leave
        ${NSD_GetText} $PathDialogue_OB $Path_OB
        ${NSD_GetText} $PathDialogue_FO $Path_FO
        ${NSD_GetText} $PathDialogue_NV $Path_NV
        ${NSD_GetText} $PathDialogue_Nehrim $Path_Nehrim
		${NSD_GetText} $PathDialogue_SK $Path_SK
        ${NSD_GetText} $PathDialogue_Other $Path_Other
        ${NSD_GetState} $Check_OB $CheckState_OB
        ${NSD_GetState} $Check_FO $CheckState_FO
        ${NSD_GetState} $Check_NV $CheckState_NV
        ${NSD_GetState} $Check_Nehrim $CheckState_Nehrim
		${NSD_GetState} $Check_SK $CheckState_SK
        ${NSD_GetState} $Check_Other $CheckState_Other
        FunctionEnd
    Function PAGE_FINISH
        !insertmacro MUI_HEADER_TEXT $(PAGE_FINISH_TITLE) $(PAGE_FINISH_SUBTITLE)
        
        ReadRegStr $Path_OB HKLM "Software\BOSS" "Oblivion Path"
        ReadRegStr $Path_FO HKLM "Software\BOSS" "Fallout3 Path"
        ReadRegStr $Path_NV HKLM "Software\BOSS" "NewVegas Path"
        ReadRegStr $Path_Other HKLM "Software\BOSS" "Other Path"
        ReadRegStr $Path_Nehrim HKLM "Software\BOSS" "Nehrim Path"
		ReadRegStr $Path_SK HKLM "Software\BOSS" "Skyrim Path"
        
        nsDialogs::Create 1018
            Pop $Dialog

        ${If} $Dialog == error
            Abort
        ${EndIf}
        
        IntOp $0 0 + 0
        ${If} $Path_OB != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 8u "Run BOSS for Oblivion"
                Pop $Check_OB
                ${NSD_AddStyle} $Check_OB ${WS_GROUP}
                ${NSD_SetState} $Check_OB $CheckState_OB
            IntOp $0 $0 + 15
        ${EndIf}
        ${If} $Path_FO != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 8u "Run BOSS for Fallout 3"
                Pop $Check_FO
                ${NSD_AddStyle} $Check_FO ${WS_GROUP}
                ${NSD_SetState} $Check_FO $CheckState_FO
            IntOp $0 $0 + 15
        ${EndIf}
        ${If} $Path_NV != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 8u "Run BOSS for Fallout New Vegas"
                Pop $Check_NV
                ${NSD_SetState} $Check_NV $CheckState_NV
                ${NSD_AddStyle} $Check_NV ${WS_GROUP}
            IntOp $0 $0 + 15
        ${EndIf}
        ${If} $Path_Nehrim != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 8u "Run BOSS for Nehrim"
                Pop $Check_Nehrim
                ${NSD_AddStyle} $Check_Nehrim ${WS_GROUP}
                ${NSD_SetState} $Check_Nehrim $CheckState_Nehrim
            IntOp $0 $0 + 15
        ${EndIf}
		${If} $Path_SK != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 8u "Run BOSS for Skyrim"
                Pop $Check_SK
                ${NSD_AddStyle} $Check_SK ${WS_GROUP}
                ${NSD_SetState} $Check_SK $CheckState_SK
            IntOp $0 $0 + 15
        ${EndIf}
        ${If} $Path_Other != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 8u "Run BOSS for Other"
                Pop $Check_Other
                ${NSD_AddStyle} $Check_Other ${WS_GROUP}
                ${NSD_SetState} $Check_Other $CheckState_Other
            IntOp $0 $0 + 15
        ${EndIf}
        IntOp $1 0 + 0
        IfFileExists "$COMMONFILES\BOSS\BOSS ReadMe.html" 0 +6
            ${NSD_CreateCheckBox} $1% $0u 25% 8u "View Readme"
                Pop $Check_Readme
                ${NSD_AddStyle} $Check_Readme ${WS_GROUP}
                ${NSD_SetState} $Check_Readme ${BST_CHECKED}
                IntOp $1 $1 + 25
		${NSD_CreateCheckBox} $1% $0u 75% 8u "Delete files from old BOSS versions"
            Pop $Check_DeleteOldFiles
            ${NSD_AddStyle} $Check_DeleteOldFiles ${WS_GROUP}
            ${NSD_SetState} $Check_DeleteOldFiles ${BST_CHECKED}
			${NSD_CreateCheckBox} $1% $0u 75% 8u "Delete files from old BOSS versions"
		IntOp $0 $0 + 13
		IntOp $1 $1 + 4
		 ${NSD_CreateCheckBox} $1% $0u 75% 8u "Remove userlists if present"
			Pop $Check_RemoveUserFiles
			${NSD_AddStyle} $Check_RemoveUserFiles ${WS_GROUP}
        nsDialogs::Show
        FunctionEnd
    Function PAGE_FINISH_Leave
        ${NSD_GetState} $Check_OB $CheckState_OB
        ${NSD_GetState} $Check_FO $CheckState_FO
        ${NSD_GetState} $Check_NV $CheckState_NV
        ${NSD_GetState} $Check_Nehrim $CheckState_Nehrim
		${NSD_GetState} $Check_SK $CheckState_SK
        ${NSD_GetState} $Check_Other $CheckState_Other
        
        ${If} $CheckState_OB == ${BST_CHECKED}
			SetOutPath "$Path_OB\BOSS"
            Exec '"$Path_OB\BOSS\Boss.exe"'
        ${EndIf}
        ${If} $CheckState_FO == ${BST_CHECKED}
            SetOutPath "$Path_FO\BOSS"
            Exec '"$Path_FO\BOSS\Boss.exe"'
        ${EndIf}
        ${If} $CheckState_NV == ${BST_CHECKED}
			SetOutPath "$Path_NV\BOSS"
            Exec '"$Path_NV\BOSS\Boss.exe"'
        ${EndIf}
        ${If} $CheckState_Nehrim == ${BST_CHECKED}
            SetOutPath "$Path_Nehrim\BOSS"
            Exec '"$Path_Nehrim\BOSS\Boss.exe"'
        ${EndIf}
		${If} $CheckState_SK == ${BST_CHECKED}
            SetOutPath "$Path_SK\BOSS"
            Exec '"$Path_SK\BOSS\Boss.exe"'
        ${EndIf}
        ${If} $CheckState_Other == ${BST_CHECKED}
            SetOutPath "$Path_Other\BOSS"
            Exec '"$Path_Other\BOSS\Boss.exe"'
        ${EndIf}
        ${NSD_GetState} $Check_Readme $0
        ${If} $0 == ${BST_CHECKED}
            SetOutPath "$COMMONFILES\BOSS"
            ExecShell "open" '"$COMMONFILES\BOSS\BOSS ReadMe.html"'
        ${EndIf}
        ${NSD_GetState} $Check_DeleteOldFiles $0
		${NSD_GetState} $Check_RemoveUserFiles $1
        ${If} $0 == ${BST_CHECKED}
            ${If} $Path_OB != $Empty
                Delete "$Path_OB\Data\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
                Delete "$Path_OB\Data\modlist.*"
                Delete "$Path_OB\Data\masterlist.txt"
				Delete "$Path_OB\Data\BOSS\modlist.*"
				Delete "$Path_OB\Data\BOSS\masterlist.txt"
				Delete "$Path_OB\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
				${If} $1 == ${BST_CHECKED}
					Delete "$Path_OB\Data\BOSS\userlist.txt"
				${EndIf}
				RMDir  "$Path_OB\Data\BOSS"
            ${EndIf}
            ${If} $Path_FO != $Empty
                Delete "$Path_FO\Data\BOSS*"
                Delete "$Path_FO\Data\modlist.*"
                Delete "$Path_FO\Data\masterlist.txt"
				Delete "$Path_FO\Data\BOSS\modlist.*"
				Delete "$Path_FO\Data\BOSS\masterlist.txt"
				Delete "$Path_FO\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
				${If} $1 == ${BST_CHECKED}
					Delete "$Path_FO\Data\BOSS\userlist.txt"
				${EndIf}
				RMDir  "$Path_FO\Data\BOSS"
            ${EndIf}
            ${If} $Path_NV != $Empty
                Delete "$Path_NV\Data\BOSS*"
                Delete "$Path_NV\Data\modlist.*"
                Delete "$Path_NV\Data\masterlist.txt"
				Delete "$Path_NV\Data\BOSS\modlist.*"
				Delete "$Path_NV\Data\BOSS\masterlist.txt"
				Delete "$Path_NV\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
				${If} $1 == ${BST_CHECKED}
					Delete "$Path_NV\Data\BOSS\userlist.txt"
				${EndIf}
				RMDir  "$Path_NV\Data\BOSS"
            ${EndIf}
            ${If} $Path_Nehrim != $Empty
                Delete "$Path_Nehrim\Data\BOSS*"
                Delete "$Path_Nehrim\Data\modlist.*"
                Delete "$Path_Nehrim\Data\masterlist.txt"
				Delete "$Path_Nehrim\Data\BOSS\modlist.*"
				Delete "$Path_Nehrim\Data\BOSS\masterlist.txt"
				Delete "$Path_Nehrim\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
				${If} $1 == ${BST_CHECKED}
					Delete "$Path_Nehrim\Data\BOSS\userlist.txt"
				${EndIf}
				RMDir  "$Path_Nehrim\Data\BOSS"
            ${EndIf}
			 ${If} $Path_SK != $Empty
                Delete "$Path_SK\Data\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
                Delete "$Path_SK\Data\modlist.*"
                Delete "$Path_SK\Data\masterlist.txt"
				Delete "$Path_SK\Data\BOSS\modlist.*"
				Delete "$Path_SK\Data\BOSS\masterlist.txt"
				Delete "$Path_SK\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
				${If} $1 == ${BST_CHECKED}
					Delete "$Path_SK\Data\BOSS\userlist.txt"
				${EndIf}
				RMDir  "$Path_SK\Data\BOSS"
            ${EndIf}
            ${If} $Path_Other != $Empty
                Delete "$Path_Other\Data\BOSS*"
                Delete "$Path_Other\Data\modlist.*"
                Delete "$Path_Other\Data\masterlist.txt"
				Delete "$Path_Other\Data\BOSS\modlist.*"
				Delete "$Path_Other\Data\BOSS\masterlist.txt"
				Delete "$Path_Other\Data\BOSS\BOSS*" #Gets rid of readmes, logs and bat files in one fell swoop.
				${If} $1 == ${BST_CHECKED}
					Delete "$Path_Other\Data\BOSS\userlist.txt"
				${EndIf}
				RMDir  "$Path_Other\Data\BOSS"
            ${EndIf}
        ${EndIf}
		
        FunctionEnd
    Function OnClick_Browse
        Pop $0
        ${If} $0 == $Browse_OB
            StrCpy $1 $PathDialogue_OB
        ${ElseIf} $0 == $Browse_FO
            StrCpy $1 $PathDialogue_FO
        ${ElseIf} $0 == $Browse_NV
            StrCpy $1 $PathDialogue_NV
        ${ElseIf} $0 == $Browse_Nehrim
            StrCpy $1 $PathDialogue_Nehrim
		 ${ElseIf} $0 == $Browse_SK
            StrCpy $1 $PathDialogue_SK
        ${ElseIf} $0 == $Browse_Other
            StrCpy $1 $PathDialogue_Other
        ${EndIf}
        ${NSD_GetText} $1 $Function_DirPrompt
        nsDialogs::SelectFolderDialog /NOUNLOAD "Please select a target directory" $Function_DirPrompt
        Pop $0

        ${If} $0 == error
            Abort
        ${EndIf}

        ${NSD_SetText} $1 $0
        FunctionEnd
;-------------------------------- The Installation Sections:
    Section "BOSS (required)" Main
      
        SectionIn 1 2 RO
        ${If} $CheckState_OB == ${BST_CHECKED}
            SetOutPath $Path_OB\BOSS
            File code\boss-cli\trunk\bin\Release\BOSS.exe
            WriteRegStr HKLM "SOFTWARE\BOSS" "Oblivion Path" "$Path_OB"
			IfFileExists BOSS.ini 0 +3
				Delete BOSS.ini.old
				Rename BOSS.ini BOSS.ini.old
        ${EndIf}
        ${If} $CheckState_FO == ${BST_CHECKED}
            SetOutPath $Path_FO\BOSS
            File code\boss-cli\trunk\bin\Release\BOSS.exe
            WriteRegStr HKLM "SOFTWARE\BOSS" "Fallout3 Path" "$Path_FO"
			IfFileExists BOSS.ini 0 +3
				Delete BOSS.ini.old
				Rename BOSS.ini BOSS.ini.old
        ${EndIf}
        ${If} $CheckState_NV == ${BST_CHECKED}
            SetOutPath $Path_NV\BOSS
            File code\boss-cli\trunk\bin\Release\BOSS.exe
            WriteRegStr HKLM "SOFTWARE\BOSS" "NewVegas Path" "$Path_NV"
			IfFileExists BOSS.ini 0 +3
				Delete BOSS.ini.old
				Rename BOSS.ini BOSS.ini.old
        ${EndIf}
        ${If} $CheckState_Nehrim == ${BST_CHECKED}
            SetOutPath $Path_Nehrim\BOSS
            File code\boss-cli\trunk\bin\Release\BOSS.exe
            WriteRegStr HKLM "SOFTWARE\BOSS" "Nehrim Path" "$Path_Nehrim"
			IfFileExists BOSS.ini 0 +3
				Delete BOSS.ini.old
				Rename BOSS.ini BOSS.ini.old
        ${EndIf}
		${If} $CheckState_SK == ${BST_CHECKED}
            SetOutPath $Path_SK\BOSS
            File code\boss-cli\trunk\bin\Release\BOSS.exe
            WriteRegStr HKLM "SOFTWARE\BOSS" "Skyrim Path" "$Path_SK"
			IfFileExists BOSS.ini 0 +3
				Delete BOSS.ini.old
				Rename BOSS.ini BOSS.ini.old
        ${EndIf}
        ${If} $CheckState_Other == ${BST_CHECKED}
            SetOutPath $Path_Other\BOSS
            File code\boss-cli\trunk\bin\Release\BOSS.exe
            WriteRegStr HKLM "SOFTWARE\BOSS" "Other Path" "$Path_Other"
			IfFileExists BOSS.ini 0 +3
				Delete BOSS.ini.old
				Rename BOSS.ini BOSS.ini.old
        ${EndIf}
      
      ; Write the uninstall keys for Windows
      WriteRegStr HKLM "Software\BOSS" "Installer Path" "$EXEPATH"
      WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "DisplayName" "BOSS"
      WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "UninstallString" '"$COMMONFILES\BOSS\uninstall.exe"'
      WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "URLInfoAbout" 'http://better-oblivion-sorting-software.googlecode.com/'
      WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "HelpLink" 'http://better-oblivion-sorting-software.googlecode.com/'
      WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "Publisher" 'BOSS Development Team'
      WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "DisplayVersion" '1.9.0'      
      WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "NoModify" 1
      WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "NoRepair" 1
      CreateDirectory "$COMMONFILES\BOSS"
      WriteUninstaller "$COMMONFILES\BOSS\uninstall.exe"
    SectionEnd
	Section "BOSS GUI" GUI
		SectionIn 1 2
        ${If} $CheckState_OB == ${BST_CHECKED}
            SetOutPath $Path_OB\BOSS
            File "code\boss-gui\trunk\bin\Release\BOSS GUI.exe"
        ${EndIf}
        ${If} $CheckState_FO == ${BST_CHECKED}
            SetOutPath $Path_FO\BOSS
            File "code\boss-gui\trunk\bin\Release\BOSS GUI.exe"
        ${EndIf}
        ${If} $CheckState_NV == ${BST_CHECKED}
            SetOutPath $Path_NV\BOSS
            File "code\boss-gui\trunk\bin\Release\BOSS GUI.exe"
        ${EndIf}
        ${If} $CheckState_Nehrim == ${BST_CHECKED}
            SetOutPath $Path_Nehrim\BOSS
            File "code\boss-gui\trunk\bin\Release\BOSS GUI.exe"
        ${EndIf}
		 ${If} $CheckState_SK == ${BST_CHECKED}
            SetOutPath $Path_SK\BOSS
            File "code\boss-gui\trunk\bin\Release\BOSS GUI.exe"
        ${EndIf}
        ${If} $CheckState_Other == ${BST_CHECKED}
            SetOutPath $Path_Other\BOSS
            File "code\boss-gui\trunk\bin\Release\BOSS GUI.exe"
        ${EndIf}
    SectionEnd
    SectionGroup "Start Menu Shortcuts" Shortcuts_SM
        Section "Main" StartMenu
            SectionIn 1
            CreateDirectory "$SMPROGRAMS\BOSS"
            CreateShortCut "$SMPROGRAMS\BOSS\Uninstall.lnk" "$COMMONFILES\BOSS\uninstall.exe" "" "$COMMONFILES\BOSS\uninstall.exe" 0
            ${If} $CheckState_OB == ${BST_CHECKED}
                SetOutPath "$Path_OB\BOSS"
                CreateShortCut "$SMPROGRAMS\BOSS\BOSS - Oblivion.lnk" "$Path_OB\BOSS\BOSS.exe" "" "$Path_OB\BOSS\BOSS.exe" 0
				CreateShortCut "$SMPROGRAMS\BOSS\BOSS GUI - Oblivion.lnk" "$Path_OB\BOSS\BOSS GUI.exe" "" "$Path_OB\BOSS\BOSS GUI.exe" 0
            ${EndIf}
            ${If} $CheckState_FO == ${BST_CHECKED}
                SetOutPath "$Path_FO\BOSS"
                CreateShortCut "$SMPROGRAMS\BOSS\BOSS - Fallout3.lnk" "$Path_FO\BOSS\BOSS.exe" "" "$Path_FO\BOSS\BOSS.exe" 0
				CreateShortCut "$SMPROGRAMS\BOSS\BOSS GUI - Fallout3.lnk" "$Path_FO\BOSS\BOSS GUI.exe" "" "$Path_FO\BOSS\BOSS GUI.exe" 0
            ${EndIf}
            ${If} $CheckState_NV == ${BST_CHECKED}
                SetOutPath "$Path_NV\BOSS"
                CreateShortCut "$SMPROGRAMS\BOSS\BOSS - Fallout New Vegas.lnk" "$Path_NV\BOSS\BOSS.exe" "" "$Path_NV\BOSS\BOSS.exe" 0
				CreateShortCut "$SMPROGRAMS\BOSS\BOSS GUI - Fallout New Vegas.lnk" "$Path_NV\BOSS\BOSS GUI.exe" "" "$Path_NV\BOSS\BOSS GUI.exe" 0
            ${EndIf}
            ${If} $CheckState_Nehrim == ${BST_CHECKED}
                SetOutPath "$Path_Nehrim\BOSS"
                CreateShortCut "$SMPROGRAMS\BOSS\BOSS - Nehrim.lnk" "$Path_Nehrim\BOSS\BOSS.exe" "" "$Path_Nehrim\BOSS\BOSS.exe" 0
				CreateShortCut "$SMPROGRAMS\BOSS\BOSS GUI - Nehrim.lnk" "$Path_Nehrim\BOSS\BOSS GUI.exe" "" "$Path_Nehrim\BOSS\BOSS GUI.exe" 0
            ${EndIf}
			${If} $CheckState_SK == ${BST_CHECKED}
                SetOutPath "$Path_SK\BOSS"
                CreateShortCut "$SMPROGRAMS\BOSS\BOSS - Skyrim.lnk" "$Path_SK\BOSS\BOSS.exe" "" "$Path_SK\BOSS\BOSS.exe" 0
				CreateShortCut "$SMPROGRAMS\BOSS\BOSS GUI - Skyrim.lnk" "$Path_SK\BOSS\BOSS GUI.exe" "" "$Path_SK\BOSS\BOSS GUI.exe" 0
            ${EndIf}
            ${If} $CheckState_Other == ${BST_CHECKED}
                SetOutPath "$Path_Other\BOSS"
                CreateShortCut "$SMPROGRAMS\BOSS\BOSS - Other.lnk" "$Path_Other\BOSS\BOSS.exe" "" "$Path_Other\BOSS\BOSS.exe" 0
				CreateShortCut "$SMPROGRAMS\BOSS\BOSS GUI - Other.lnk" "$Path_Other\BOSS\BOSS GUI.exe" "" "$Path_Other\BOSS\BOSS GUI.exe" 0
            ${EndIf}
            SectionEnd
        Section "Documentation Shortcuts" Shortcuts_SM_Docs
            SectionIn 1
            CreateShortCut "$SMPROGRAMS\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
            CreateShortCut "$SMPROGRAMS\BOSS\BOSS User Rules ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
#			CreateShortCut "$SMPROGRAMS\BOSS\BOSS API ReadMe.lnk" "$COMMONFILES\BOSS\BOSS API ReadMe.html" "" "$COMMONFILES\BOSS\BOSS API ReadMe.html" 0
			CreateShortCut "$SMPROGRAMS\BOSS\BOSS Masterlist Syntax.lnk" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" "" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" 0
            SectionEnd
        SectionGroupEnd
    Section "Documentation" Documentation
        SectionIn 1
        SetOutPath $COMMONFILES\BOSS
        File "data\boss-common\BOSS ReadMe.html"
        File "data\boss-common\BOSS User Rules ReadMe.html"
#		File "data\boss-common\BOSS API ReadMe.html"
		File "data\boss-common\BOSS Masterlist Syntax.html"
        ${If} $CheckState_OB == ${BST_CHECKED}
            CreateDirectory "$Path_OB\BOSS"
            CreateShortCut "$Path_OB\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
            CreateShortCut "$Path_OB\BOSS\BOSS User Rules ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
#			CreateShortCut "$Path_OB\BOSS\BOSS API ReadMe.lnk" "$COMMONFILES\BOSS\BOSS API ReadMe.html" "" "$COMMONFILES\BOSS\BOSS API ReadMe.html" 0
			CreateShortCut "$Path_OB\BOSS\BOSS Masterlist Syntax.lnk" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" "" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" 0
        ${EndIf}
        ${If} $CheckState_FO == ${BST_CHECKED}
                CreateDirectory "$Path_FO\BOSS"
            CreateShortCut "$Path_FO\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
            CreateShortCut "$Path_FO\BOSS\BOSS User Rules ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
#			CreateShortCut "$Path_FO\BOSS\BOSS API ReadMe.lnk" "$COMMONFILES\BOSS\BOSS API ReadMe.html" "" "$COMMONFILES\BOSS\BOSS API ReadMe.html" 0
			CreateShortCut "$Path_FO\BOSS\BOSS Masterlist Syntax.lnk" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" "" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" 0
        ${EndIf}
        ${If} $CheckState_NV == ${BST_CHECKED}
            CreateDirectory "$Path_NV\BOSS"
            CreateShortCut "$Path_NV\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
            CreateShortCut "$Path_NV\BOSS\BOSS User Rules ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
#			CreateShortCut "$Path_NV\BOSS\BOSS API ReadMe.lnk" "$COMMONFILES\BOSS\BOSS API ReadMe.html" "" "$COMMONFILES\BOSS\BOSS API ReadMe.html" 0
			CreateShortCut "$Path_NV\BOSS\BOSS Masterlist Syntax.lnk" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" "" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" 0
        ${EndIf}
        ${If} $CheckState_Nehrim == ${BST_CHECKED}
            CreateDirectory "$Path_Nehrim\BOSS"
            CreateShortCut "$Path_Nehrim\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
            CreateShortCut "$Path_Nehrim\BOSS\BOSS User Rules ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
#			CreateShortCut "$Path_Nehrim\BOSS\BOSS API ReadMe.lnk" "$COMMONFILES\BOSS\BOSS API ReadMe.html" "" "$COMMONFILES\BOSS\BOSS API ReadMe.html" 0
			CreateShortCut "$Path_Nehrim\BOSS\BOSS Masterlist Syntax.lnk" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" "" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" 0
        ${EndIf}
		${If} $CheckState_SK == ${BST_CHECKED}
            CreateDirectory "$Path_SK\BOSS"
            CreateShortCut "$Path_SK\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
            CreateShortCut "$Path_SK\BOSS\BOSS User Rules ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
#			CreateShortCut "$Path_SK\BOSS\BOSS API ReadMe.lnk" "$COMMONFILES\BOSS\BOSS API ReadMe.html" "" "$COMMONFILES\BOSS\BOSS API ReadMe.html" 0
			CreateShortCut "$Path_SK\BOSS\BOSS Masterlist Syntax.lnk" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" "" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" 0
        ${EndIf}
        ${If} $CheckState_Other == ${BST_CHECKED}
            CreateDirectory "$Path_Other\BOSS"
            CreateShortCut "$Path_Other\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
            CreateShortCut "$Path_Other\BOSS\BOSS User Rules ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
#			CreateShortCut "$Path_Other\BOSS\BOSS API ReadMe.lnk" "$COMMONFILES\BOSS\BOSS API ReadMe.html" "" "$COMMONFILES\BOSS\BOSS API ReadMe.html" 0
			CreateShortCut "$Path_Other\BOSS\BOSS Masterlist Syntax.lnk" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" "" "$COMMONFILES\BOSS\BOSS Masterlist Syntax.html" 0
        ${EndIf} 
        SectionEnd
;-------------------------------- Custom Uninstallation Pages and their Functions:
    Function un.PAGE_SELECT_GAMES
        !insertmacro MUI_HEADER_TEXT $(PAGE_SELECT_GAMES_TITLE) $(unPAGE_SELECT_GAMES_SUBTITLE)
        GetFunctionAddress $unFunction_Browse un.OnClick_Browse
        
        nsDialogs::Create 1018
            Pop $Dialog

        ${If} $Dialog == error
            Abort
        ${EndIf}
        
        ${NSD_CreateLabel} 0 0 100% 8u "Please select which game(s) to uninstall BOSS from:"
        Pop $Label
        
        IntOp $0 0 + 9
        ${If} $Path_OB != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 13u "&Oblivion"
                Pop $Check_OB
                ${NSD_SetState} $Check_OB $CheckState_OB
            IntOp $0 $0 + 13
            ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_OB"
                Pop $PathDialogue_OB
            ${NSD_CreateBrowseButton} -10% $0u 5% 13u "..."
                Pop $Browse_OB
                nsDialogs::OnClick $Browse_OB $Function_Browse
            IntOp $0 $0 + 13
        ${EndIf}
        ${If} $Path_FO != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 13u "&Fallout 3"
                Pop $Check_FO
                ${NSD_SetState} $Check_FO $CheckState_FO
            IntOp $0 $0 + 13
            ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_FO"
                Pop $PathDialogue_FO
            ${NSD_CreateBrowseButton} -10% 48u 5% 13u "..."
                Pop $Browse_FO
                nsDialogs::OnClick $Browse_FO $Function_Browse
            IntOp $0 $0 + 13
        ${EndIf}
        ${If} $Path_NV != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 13u "Fallout: New &Vegas"
                Pop $Check_NV
                ${NSD_SetState} $Check_NV $CheckState_NV
            IntOp $0 $0 + 13
            ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_NV"
                Pop $PathDialogue_NV
            ${NSD_CreateBrowseButton} -10% $0u 5% 13u "..."
                Pop $Browse_NV
                nsDialogs::OnClick $Browse_NV $Function_Browse
            IntOp $0 $0 + 13
        ${EndIf}
        ${If} $Path_Nehrim != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 13u "Nehrim"
                Pop $Check_Nehrim
                ${NSD_SetState} $Check_Nehrim $CheckState_Nehrim
            IntOp $0 $0 + 13
            ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_Nehrim"
                Pop $PathDialogue_Nehrim
            ${NSD_CreateBrowseButton} -10% $0u 5% 13u "..."
                Pop $Browse_Nehrim
                nsDialogs::OnClick $Browse_Nehrim $Function_Browse
            IntOp $0 $0 + 13
        ${EndIf}
		${If} $Path_SK != $Empty
            ${NSD_CreateCheckBox} 0 $0u 100% 13u "Skyrim"
                Pop $Check_SK
                ${NSD_SetState} $Check_SK $CheckState_SK
            IntOp $0 $0 + 13
            ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_SK"
                Pop $PathDialogue_SK
            ${NSD_CreateBrowseButton} -10% $0u 5% 13u "..."
                Pop $Browse_SK
                nsDialogs::OnClick $Browse_SK $Function_Browse
            IntOp $0 $0 + 13
        ${EndIf}
        ${If} $Path_Other != $Empty 
            ${NSD_CreateCheckBox} 0 $0u 100% 13u "Other Location (e.g. an undetected game)."
                Pop $Check_Other
                ${NSD_SetState} $Check_Other $CheckState_Other
            IntOp $0 $0 + 13
            ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_Other"
                Pop $PathDialogue_Other
            ${NSD_CreateBrowseButton} -10% $0u 5% 13u "..."
                Pop $Browse_Other
                nsDialogs::OnClick $Browse_Other $Function_Browse
            IntOp $0 $0 + 13
        ${EndIf}
        ${NSD_CreateCheckBox} 0 $0u 100% 13u "Uninstall userlist and ini file if they exist."
            Pop $Check_RemoveUserFiles
        nsDialogs::Show
        FunctionEnd
    Function un.PAGE_SELECT_GAMES_Leave
        ${NSD_GetText} $PathDialogue_OB $Path_OB
        ${NSD_GetText} $PathDialogue_FO $Path_FO
        ${NSD_GetText} $PathDialogue_NV $Path_NV
        ${NSD_GetText} $PathDialogue_Nehrim $Path_Nehrim
		${NSD_GetText} $PathDialogue_SK $Path_SK
        ${NSD_GetText} $PathDialogue_Other $Path_Other
        ${NSD_GetState} $Check_OB $CheckState_OB
        ${NSD_GetState} $Check_FO $CheckState_FO
        ${NSD_GetState} $Check_NV $CheckState_NV
        ${NSD_GetState} $Check_Nehrim $CheckState_Nehrim
		${NSD_GetState} $Check_SK $CheckState_SK
        ${NSD_GetState} $Check_Other $CheckState_Other
        ${NSD_GetState} $Check_RemoveUserFiles $CheckState_RemoveUserFiles
        FunctionEnd
    Function un.OnClick_Browse
        Pop $0
        ${If} $0 == $Browse_OB
            StrCpy $1 $PathDialogue_OB
        ${ElseIf} $0 == $Browse_FO
            StrCpy $1 $PathDialogue_FO
        ${ElseIf} $0 == $Browse_NV
            StrCpy $1 $PathDialogue_NV
        ${ElseIf} $0 == $Browse_Nehrim
            StrCpy $1 $PathDialogue_Nehrim
		${ElseIf} $0 == $Browse_SK
            StrCpy $1 $PathDialogue_SK
        ${ElseIf} $0 == $Browse_Other
            StrCpy $1 $PathDialogue_Other
        ${EndIf}
        ${NSD_GetText} $1 $Function_DirPrompt
        nsDialogs::SelectFolderDialog /NOUNLOAD "Please select a target directory" $Function_DirPrompt
        Pop $0

        ${If} $0 == error
            Abort
        ${EndIf}

        ${NSD_SetText} $1 $0
        FunctionEnd  
;-------------------------------- The Uninstallation Code:
    Section "Uninstall"
        ${If} $CheckState_OB == ${BST_CHECKED}
            Delete "$Path_OB\BOSS\*.lnk"
            Delete "$Path_OB\BOSS\masterlist.txt"
            Delete "$Path_OB\BOSS\modlist.*"
            Delete "$Path_OB\BOSS\BOSSlog.html"
			Delete "$Path_OB\BOSS\BOSSlog.txt"
			Delete "$Path_OB\BOSS\BOSS.exe"
			Delete "$Path_OB\BOSS\BOSS GUI.exe"
			Delete "$Path_OB\BOSS\BOSSCommandLineLog.txt"
            ${If} $CheckState_RemoveUserFiles == ${BST_CHECKED}
                Delete "$Path_OB\BOSS\userlist.txt"
				Delete "$Path_OB\BOSS\BOSS.ini"
				Delete "$Path_OB\BOSS\BOSS.ini.old"
            ${EndIf}
            RMDir "$Path_OB\BOSS"
            Delete "$SMPROGRAMS\BOSS\BOSS - Oblivion.lnk"
			Delete "$SMPROGRAMS\BOSS\BOSS GUI - Oblivion.lnk"
            DeleteRegValue HKLM "SOFTWARE\BOSS" "Oblivion Path"
            ;Delete the stupid MUICache Windows created registry references to the BOSS executables...
                DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_OB\BOSS\BOSS.exe"
                DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_OB\BOSS\BOSS GUI.exe"
                DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_OB\BOSS\BOSS.exe"
                DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_OB\BOSS\BOSS GUI.exe"
                DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$Path_OB\BOSS\BOSS.exe"
                DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$Path_OB\BOSS\BOSS GUI.exe"
            StrCpy $Path_OB $Empty
        ${EndIf}
        ${If} $CheckState_FO == ${BST_CHECKED}
            Delete "$Path_FO\BOSS\*.lnk"
            Delete "$Path_FO\BOSS\masterlist.txt"
            Delete "$Path_FO\BOSS\modlist.*"
            Delete "$Path_FO\BOSS\BOSSlog.html"
			Delete "$Path_FO\BOSS\BOSSlog.txt"
			Delete "$Path_FO\BOSS\BOSS.exe"
			Delete "$Path_FO\BOSS\BOSS GUI.exe"
			Delete "$Path_FO\BOSS\BOSSCommandLineLog.txt"
            ${If} $CheckState_RemoveUserFiles == ${BST_CHECKED}
                Delete "$Path_FO\BOSS\userlist.txt"
				Delete "$Path_FO\BOSS\BOSS.ini"
				Delete "$Path_FO\BOSS\BOSS.ini.old"
            ${EndIf}
            RMDir "$Path_FO\BOSS"
            Delete "$SMPROGRAMS\BOSS\BOSS - Fallout3.lnk"
			Delete "$SMPROGRAMS\BOSS\BOSS GUI - Fallout3.lnk"
            DeleteRegValue HKLM "SOFTWARE\BOSS" "Fallout3 Path"
            ;Delete the stupid MUICache Windows created registry references to the BOSS executables...
                DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_FO\BOSS\BOSS.exe"
                DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_FO\BOSS\BOSS GUI.exe"
                DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_FO\BOSS\BOSS.exe"
                DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_FO\BOSS\BOSS GUI.exe"
                DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$Path_FO\BOSS\BOSS.exe"
                DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$Path_FO\BOSS\BOSS GUI.exe"
            StrCpy $Path_FO $Empty
        ${EndIf}
        ${If} $CheckState_NV == ${BST_CHECKED}
            Delete "$Path_NV\BOSS\*.lnk"
            Delete "$Path_NV\BOSS\masterlist.txt"
            Delete "$Path_NV\BOSS\modlist.*"
            Delete "$Path_NV\BOSS\BOSSlog.html"
			Delete "$Path_NV\BOSS\BOSSlog.txt"
			Delete "$Path_NV\BOSS\BOSS.exe"
			Delete "$Path_NV\BOSS\BOSS GUI.exe"
			Delete "$Path_NV\BOSS\BOSSCommandLineLog.txt"
            ${If} $CheckState_RemoveUserFiles == ${BST_CHECKED}
                Delete "$Path_NV\BOSS\userlist.txt"
				Delete "$Path_NV\BOSS\BOSS.ini"
				Delete "$Path_NV\BOSS\BOSS.ini.old"
            ${EndIf}
            RMDir "$Path_NV\BOSS"
            Delete "$SMPROGRAMS\BOSS\BOSS - Fallout New Vegas.lnk"
			Delete "$SMPROGRAMS\BOSS\BOSS GUI - Fallout New Vegas.lnk"
            DeleteRegValue HKLM "SOFTWARE\BOSS" "NewVegas Path"
            ;Delete the stupid MUICache Windows created registry references to the BOSS executables...
                DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_NV\BOSS\BOSS.exe"
                DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_NV\BOSS\BOSS GUI.exe"
                DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_NV\BOSS\BOSS.exe"
                DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_NV\BOSS\BOSS GUI.exe"
                DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$Path_NV\BOSS\BOSS.exe"
                DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$Path_NV\BOSS\BOSS GUI.exe"
            StrCpy $Path_NV $Empty
        ${EndIf}
        ${If} $CheckState_Nehrim == ${BST_CHECKED}
            Delete "$Path_Nehrim\BOSS\*.lnk"
            Delete "$Path_Nehrim\BOSS\masterlist.txt"
            Delete "$Path_Nehrim\BOSS\modlist.*"
            Delete "$Path_Nehrim\BOSS\BOSSlog.html"
			Delete "$Path_Nehrim\BOSS\BOSSlog.txt"
			Delete "$Path_Nehrim\BOSS\BOSS.exe"
			Delete "$Path_Nehrim\BOSS\BOSS GUI.exe"
			Delete "$Path_Nehrim\BOSS\BOSSCommandLineLog.txt"
            ${If} $CheckState_RemoveUserFiles == ${BST_CHECKED}
                Delete "$Path_Nehrim\BOSS\userlist.txt"
				Delete "$Path_Nehrim\BOSS\BOSS.ini"
				Delete "$Path_Nehrim\BOSS\BOSS.ini.old"
            ${EndIf}
            RMDir "$Path_Nehrim\BOSS"
            Delete "$SMPROGRAMS\BOSS\BOSS - Nehrim.lnk"
			Delete "$SMPROGRAMS\BOSS\BOSS GUI - Nehrim.lnk"
            DeleteRegValue HKLM "SOFTWARE\BOSS" "Nehrim Path"
            ;Delete the stupid MUICache Windows created registry references to the BOSS executables...
                DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_Nehrim\BOSS\BOSS.exe"
                DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_Nehrim\BOSS\BOSS GUI.exe"
                DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_Nehrim\BOSS\BOSS.exe"
                DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_Nehrim\BOSS\BOSS GUI.exe"
                DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$Path_Nehrim\BOSS\BOSS.exe"
                DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$Path_Nehrim\BOSS\BOSS GUI.exe"
            StrCpy $Path_Nehrim $Empty
        ${EndIf}
		${If} $CheckState_SK == ${BST_CHECKED}
            Delete "$Path_SK\BOSS\*.lnk"
            Delete "$Path_SK\BOSS\masterlist.txt"
            Delete "$Path_SK\BOSS\modlist.*"
            Delete "$Path_SK\BOSS\BOSSlog.html"
			Delete "$Path_SK\BOSS\BOSSlog.txt"
			Delete "$Path_SK\BOSS\BOSS.exe"
			Delete "$Path_SK\BOSS\BOSS GUI.exe"
			Delete "$Path_SK\BOSS\BOSSCommandLineLog.txt"
            ${If} $CheckState_RemoveUserFiles == ${BST_CHECKED}
                Delete "$Path_SK\BOSS\userlist.txt"
				Delete "$Path_SK\BOSS\BOSS.ini"
				Delete "$Path_SK\BOSS\BOSS.ini.old"
            ${EndIf}
            RMDir "$Path_SK\BOSS"
            Delete "$SMPROGRAMS\BOSS\BOSS - Skyrim.lnk"
			Delete "$SMPROGRAMS\BOSS\BOSS GUI - Skyrim.lnk"
            DeleteRegValue HKLM "SOFTWARE\BOSS" "Skyrim Path"
            ;Delete the stupid MUICache Windows created registry references to the BOSS executables...
                DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_SK\BOSS\BOSS.exe"
                DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_SK\BOSS\BOSS GUI.exe"
                DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_SK\BOSS\BOSS.exe"
                DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_SK\BOSS\BOSS GUI.exe"
                DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$Path_SK\BOSS\BOSS.exe"
                DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$Path_SK\BOSS\BOSS GUI.exe"
            StrCpy $Path_SK $Empty
        ${EndIf}
        ${If} $CheckState_Other == ${BST_CHECKED}
            Delete "$Path_Other\BOSS\*.lnk"
            Delete "$Path_Other\BOSS\masterlist.txt"
            Delete "$Path_Other\BOSS\modlist.*"
            Delete "$Path_Other\BOSS\BOSSlog.html"
			Delete "$Path_Other\BOSS\BOSSlog.txt"
			Delete "$Path_Other\BOSS\BOSS.exe"
			Delete "$Path_Other\BOSS\BOSS GUI.exe"
			Delete "$Path_Other\BOSS\BOSSCommandLineLog.txt"
            ${If} $CheckState_RemoveUserFiles == ${BST_CHECKED}
                Delete "$Path_Other\BOSS\userlist.txt"
				Delete "$Path_Other\BOSS\BOSS.ini"
				Delete "$Path_Other\BOSS\BOSS.ini.old"
            ${EndIf}
            RMDir "$Path_Other\BOSS"
            Delete "$SMPROGRAMS\BOSS\BOSS - Other.lnk"
			Delete "$SMPROGRAMS\BOSS\BOSS GUI - Other.lnk"
            DeleteRegValue HKLM "SOFTWARE\BOSS" "Other Path"
            ;Delete the stupid MUICache Windows created registry references to the BOSS executables...
                DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_Other\BOSS\BOSS.exe"
                DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_Other\BOSS\BOSS GUI.exe"
                DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_Other\BOSS\BOSS.exe"
                DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$Path_Other\BOSS\BOSS GUI.exe"
                DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$Path_Other\BOSS\BOSS.exe"
                DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$Path_Other\BOSS\BOSS GUI.exe"
            StrCpy $Path_Other $Empty
        ${EndIf}
        
        ;Is it a complete uninstall?
        ${If} $Path_OB == $Empty
            ${If} $Path_FO == $Empty
                ${If} $Path_NV == $Empty
                    ${If} $Path_Nehrim == $Empty
						${If} $Path_SK == $Empty
							${If} $Path_Other == $Empty
								DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS"
								ReadRegStr $0 HKLM "Software\BOSS" "Installer Path"
								DeleteRegKey HKLM "SOFTWARE\BOSS"
								;Delete stupid Windows created registry keys:
									DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\App Management\ARPCache\BOSS"
									DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$COMMONFILES\BOSS\Uninstall.exe"
									DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$COMMONFILES\BOSS\Uninstall.exe"
									DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$COMMONFILES\BOSS\Uninstall.exe"
									DeleteRegValue HKCR "Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$0"
									DeleteRegValue HKCU "Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache" "$0"
									DeleteRegValue HKCU "Software\Microsoft\Windows\ShellNoRoam\MuiCache" "$0"
								RMDir /r "$SMPROGRAMS\BOSS"
								RMDir /r "$COMMONFILES\BOSS"
							${EndIf}
						${EndIf}
                    ${EndIf}
                ${EndIf}
            ${EndIf}
        ${EndIf}
        SectionEnd
;-------------------------------- Descriptions/Subtitles/Language Strins:

  ;Language strings
  !insertmacro MUI_LANGUAGE "English"
  LangString DESC_Main ${LANG_ENGLISH} "The main executable."
  LangString DESC_GUI ${LANG_ENGLISH} "The Graphical User Interface for BOSS."
  LangString DESC_Shortcuts_SM ${LANG_ENGLISH} "Start Menu shortcuts for the uninstaller, BOSS.exe and BOSS GUI.exe for each game."
  LangString DESC_Shortcuts_SM_Docs ${LANG_ENGLISH} "Start Menu shortcuts for the BOSS Documentation."
  LangString DESC_Documentation ${LANG_ENGLISH} "The documentation."
  LangString PAGE_SELECT_GAMES_TITLE ${LANG_ENGLISH} "Choose Games"
  LangString PAGE_SELECT_GAMES_SUBTITLE ${LANG_ENGLISH} "Please select which game(s) you want to install BOSS for, and confirm the desired install path."
  LangString unPAGE_SELECT_GAMES_SUBTITLE ${LANG_ENGLISH} "Please select which game(s) you want to uninstall BOSS from."
  LangString PAGE_FINISH_TITLE ${LANG_ENGLISH} "Finished installing BOSS - Better Oblivion Sorting Software v1.9.0"
  LangString PAGE_FINISH_SUBTITLE ${LANG_ENGLISH} "Please select post-install tasks."
  
  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${Main} $(DESC_Main)
    !insertmacro MUI_DESCRIPTION_TEXT ${Shortcuts_SM} $(DESC_Shortcuts_SM)
    !insertmacro MUI_DESCRIPTION_TEXT ${Shortcuts_SM_Docs} $(DESC_Shortcuts_SM_Docs)
    !insertmacro MUI_DESCRIPTION_TEXT ${Documentation} $(DESC_Documentation)
	!insertmacro MUI_DESCRIPTION_TEXT ${GUI} $(DESC_GUI)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END