; BOSS.nsi

;Includes:
  !include MUI2.nsh
  !include LogicLib.nsh
  !include nsDialogs.nsh

;--------------------------------
; Basic Installer Info:
Name "BOSS [Version 1.6.3]"
OutFile "BOSS Installer.exe"
; Request application privileges for Windows Vista
RequestExecutionLevel admin
VIProductVersion 0.1.6.3
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "BOSS"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "BOSS development team"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© BOSS development team"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Installer for BOSS 1.6.3"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "0.1.6.3"

;--------------------------------
; Variables:
Var Dialog
Var Label
Var Path_OB
Var Path_FO
Var Path_NV
Var Path_Other
Var Path_Nehrim
Var Empty
Var Check_OB
Var Check_FO
Var Check_NV
Var Check_Nehrim
Var Check_Other
Var CheckState_OB
Var CheckState_FO
Var CheckState_NV
Var CheckState_Nehrim
Var CheckState_Other
Var PathDialogue_OB
Var PathDialogue_FO
Var PathDialogue_NV
Var PathDialogue_Nehrim
Var PathDialogue_Other
Var Browse_OB
Var Browse_FO
Var Browse_NV
Var Browse_Nehrim
Var Browse_Other
Var Function_Browse
Var Function_DirPrompt
Var unFunction_Browse

;-------------------------------- Install Types:
InstType "Full"
InstType "Minimal"
;-------------------------------- Pages:
  !insertmacro MUI_PAGE_WELCOME
  Page custom PAGE_SELECT_GAMES PAGE_SELECT_GAMES_Leave
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_WELCOME
  UninstPage custom un.PAGE_SELECT_GAMES un.PAGE_SELECT_GAMES_Leave
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
Function un.onInit
    ReadRegStr $Path_OB HKLM "Software\BOSS" "Oblivion Path"
    ReadRegStr $Path_FO HKLM "Software\BOSS" "Fallout3 Path"
    ReadRegStr $Path_NV HKLM "Software\BOSS" "NewVegas Path"
    ReadRegStr $Path_Other HKLM "Software\BOSS" "Other Path"
    ReadRegStr $Path_Nehrim HKLM "Software\BOSS" "Nehrim Path"
FunctionEnd

Function .onInit
    StrCpy $Empty ""
    ReadRegStr $Path_OB HKLM "Software\BOSS" "Oblivion Path"
    ReadRegStr $Path_FO HKLM "Software\BOSS" "Fallout3 Path"
    ReadRegStr $Path_NV HKLM "Software\BOSS" "NewVegas Path"
    ReadRegStr $Path_Other HKLM "Software\BOSS" "Other Path"
    ReadRegStr $Path_Nehrim HKLM "Software\BOSS" "Nehrim Path"

    ${If} $Path_OB == $Empty
        ReadRegStr $Path_OB HKLM "Software\Bethesda Softworks\Oblivion" "Installed Path"
    ${EndIf}
    ${If} $Path_FO == $Empty
        ReadRegStr $Path_FO HKLM "Software\Bethesda Softworks\Fallout3" "Installed Path"
    ${EndIf}
    ${If} $Path_NV == $Empty
        ReadRegStr $Path_NV HKLM "Software\Bethesda Softworks\NewVegas" "Installed Path"
    ${EndIf}
FunctionEnd

Function PAGE_SELECT_GAMES

    GetFunctionAddress $Function_Browse OnClick_Browse
    
	nsDialogs::Create 1018
	Pop $Dialog

	${If} $Dialog == error
		Abort
	${EndIf}
    
	${NSD_CreateLabel} 0 0 100% 8u "Please Select Game(s) to Install BOSS for:"
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
    ;--------------  
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
    ;-------------- 
    ${NSD_CreateCheckBox} 0 $0u 100% 13u "Other Location (f.e. an undetected game.)"
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
    ${NSD_GetText} $PathDialogue_Other $Path_Other
    ${NSD_GetState} $Check_OB $CheckState_OB
    ${NSD_GetState} $Check_FO $CheckState_FO
    ${NSD_GetState} $Check_NV $CheckState_NV
    ${NSD_GetState} $Check_Nehrim $CheckState_Nehrim
    ${NSD_GetState} $Check_Other $CheckState_Other
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

Section "BOSS (required)" Main
  
    SectionIn 1 2 RO
    ${If} $CheckState_OB == ${BST_CHECKED}
        SetOutPath $Path_OB\Data
        File code\boss-common\trunk\bin\Release\BOSS.exe
        WriteRegStr HKLM "SOFTWARE\BOSS" "Oblivion Path" "$Path_OB"
    ${EndIf}
    ${If} $CheckState_FO == ${BST_CHECKED}
        SetOutPath $Path_FO\Data
        File code\boss-common\trunk\bin\Release\BOSS.exe
        WriteRegStr HKLM "SOFTWARE\BOSS" "Fallout3 Path" "$Path_FO"
    ${EndIf}
    ${If} $CheckState_NV == ${BST_CHECKED}
        SetOutPath $Path_NV\Data
        File code\boss-common\trunk\bin\Release\BOSS.exe
        WriteRegStr HKLM "SOFTWARE\BOSS" "NewVegas Path" "$Path_NV"
    ${EndIf}
    ${If} $CheckState_Nehrim == ${BST_CHECKED}
        SetOutPath $Path_Nehrim\Data
        File code\boss-common\trunk\bin\Release\BOSS.exe
        WriteRegStr HKLM "SOFTWARE\BOSS" "Nehrim Path" "$Path_Nehrim"
    ${EndIf}
    ${If} $CheckState_Other == ${BST_CHECKED}
        SetOutPath $Path_Other\Data
        File code\boss-common\trunk\bin\Release\BOSS.exe
        WriteRegStr HKLM "SOFTWARE\BOSS" "Other Path" "$Path_Other"
    ${EndIf}
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "DisplayName" "BOSS"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "UninstallString" '"$COMMONFILES\BOSS\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "NoRepair" 1
  CreateDirectory "$COMMONFILES\BOSS"
  WriteUninstaller "$COMMONFILES\BOSS\uninstall.exe"
SectionEnd

; Optional sections (can be disabled by the user)
SectionGroup "Start Menu Shortcuts" Shortcuts_SM
    Section "Main" StartMenu
        SectionIn 1
        CreateDirectory "$SMPROGRAMS\BOSS"
        CreateShortCut "$SMPROGRAMS\BOSS\Uninstall.lnk" "$COMMONFILES\BOSS\uninstall.exe" "" "$COMMONFILES\BOSS\uninstall.exe" 0
        ${If} $CheckState_OB == ${BST_CHECKED}
            CreateShortCut "$SMPROGRAMS\BOSS\BOSS - Oblivion.lnk" "$Path_OB\Data\BOSS.exe" "" "$Path_OB\Data\BOSS.exe" 0
        ${EndIf}
        ${If} $CheckState_FO == ${BST_CHECKED}
            CreateShortCut "$SMPROGRAMS\BOSS\BOSS - Fallout3.lnk" "$Path_FO\Data\BOSS.exe" "" "$Path_FO\Data\BOSS.exe" 0
        ${EndIf}
        ${If} $CheckState_NV == ${BST_CHECKED}
            CreateShortCut "$SMPROGRAMS\BOSS\BOSS - Fallout NewVegas.lnk" "$Path_NV\Data\BOSS.exe" "" "$Path_NV\Data\BOSS.exe" 0
        ${EndIf}
        ${If} $CheckState_Nehrim == ${BST_CHECKED}
            CreateShortCut "$SMPROGRAMS\BOSS\BOSS - Nehrim.lnk" "$Path_Nehrim\Data\BOSS.exe" "" "$Path_Nehrim\Data\BOSS.exe" 0
        ${EndIf}
        ${If} $CheckState_Other == ${BST_CHECKED}
            CreateShortCut "$SMPROGRAMS\BOSS\BOSS - Other.lnk" "$Path_Other\Data\BOSS.exe" "" "$Path_Other\Data\BOSS.exe" 0
        ${EndIf}
    SectionEnd

    Section "Documentation Shortcuts" Shortcuts_SM_Docs
        SectionIn 1
        CreateShortCut "$SMPROGRAMS\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
        CreateShortCut "$SMPROGRAMS\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
    SectionEnd
SectionGroupEnd

Section "Documentation" Documentation
    SectionIn 1
    SetOutPath $COMMONFILES\BOSS
    File "data\boss-common\BOSS ReadMe.html"
    File "data\boss-common\BOSS User Rules ReadMe.html"
    ${If} $CheckState_OB == ${BST_CHECKED}
        CreateShortCut "$Path_OB\Data\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
        CreateShortCut "$Path_OB\Data\BOSS\BOSS User Rules ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
    ${EndIf}
    ${If} $CheckState_FO == ${BST_CHECKED}
        CreateShortCut "$Path_FO\Data\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
        CreateShortCut "$Path_FO\Data\BOSS\BOSS User Rules ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
    ${EndIf}
    ${If} $CheckState_NV == ${BST_CHECKED}
        CreateShortCut "$Path_NV\Data\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
        CreateShortCut "$Path_NV\Data\BOSS\BOSS User Rules ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
    ${EndIf}
    ${If} $CheckState_Nehrim == ${BST_CHECKED}
        CreateShortCut "$Path_Nehrim\Data\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
        CreateShortCut "$Path_Nehrim\Data\BOSS\BOSS User Rules ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
    ${EndIf}
    ${If} $CheckState_Other == ${BST_CHECKED}
        CreateShortCut "$Path_Other\Data\BOSS\BOSS ReadMe.lnk" "$COMMONFILES\BOSS\BOSS Readme.html" "" "$COMMONFILES\BOSS\BOSS Readme.html" 0
        CreateShortCut "$Path_Other\Data\BOSS\BOSS User Rules ReadMe.lnk" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" "" "$COMMONFILES\BOSS\BOSS User Rules ReadMe.html" 0
    ${EndIf} 
SectionEnd

Section "Batch Files" Batch_Files
    SectionIn 1
    ${If} $CheckState_OB == ${BST_CHECKED}
        SetOutPath $Path_OB\Data
        File "data\boss-common\*.bat"
    ${EndIf}
    ${If} $CheckState_FO == ${BST_CHECKED}
        SetOutPath $Path_FO\Data
        File "data\boss-common\*.bat"
    ${EndIf}
    ${If} $CheckState_NV == ${BST_CHECKED}
        SetOutPath $Path_NV\Data
        File "data\boss-common\*.bat"
    ${EndIf}
    ${If} $CheckState_Nehrim == ${BST_CHECKED}
        SetOutPath $Path_Nehrim\Data
        File "data\boss-common\*.bat"
    ${EndIf}
    ${If} $CheckState_Other == ${BST_CHECKED}
        SetOutPath $Path_Other\Data
        File "data\boss-common\*.bat"
    ${EndIf}
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  !insertmacro MUI_LANGUAGE "English"
  LangString DESC_Main ${LANG_ENGLISH} "The main executable."
  LangString DESC_Shortcuts_SM ${LANG_ENGLISH} "StartMenu Shortcuts for the uninstaller & BOSS.exe for each game."
  LangString DESC_Shortcuts_SM_Docs ${LANG_ENGLISH} "StartMenu Shortcuts for the BOSS Documentation."
  LangString DESC_Documentation ${LANG_ENGLISH} "The documentation."
  LangString DESC_Batch_Files ${LANG_ENGLISH} "Batch files to enable quicker use of the command functions Update and Revert (Level 1) and Debug print out (level 2)."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${Main} $(DESC_Main)
    !insertmacro MUI_DESCRIPTION_TEXT ${Shortcuts_SM} $(DESC_Shortcuts_SM)
    !insertmacro MUI_DESCRIPTION_TEXT ${Shortcuts_SM_Docs} $(DESC_Shortcuts_SM_Docs)
    !insertmacro MUI_DESCRIPTION_TEXT ${Documentation} $(DESC_Documentation)
    !insertmacro MUI_DESCRIPTION_TEXT ${Batch_Files} $(DESC_Batch_Files)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

Function un.PAGE_SELECT_GAMES

    GetFunctionAddress $unFunction_Browse un.OnClick_Browse
    
	nsDialogs::Create 1018
	Pop $Dialog

	${If} $Dialog == error
		Abort
	${EndIf}
    
	${NSD_CreateLabel} 0 0 100% 8u "Please Select Game(s) to uninstall BOSS from:"
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
    ${If} $Path_Other != $Empty 
        ${NSD_CreateCheckBox} 0 $0u 100% 13u "Other Location (f.e. an undetected game.)"
        Pop $Check_Other
        ${NSD_SetState} $Check_Other $CheckState_Other
        IntOp $0 $0 + 13
        ${NSD_CreateDirRequest} 0 $0u 90% 13u "$Path_Other"
        Pop $PathDialogue_Other
        ${NSD_CreateBrowseButton} -10% $0u 5% 13u "..."
        Pop $Browse_Other
        nsDialogs::OnClick $Browse_Other $Function_Browse
        nsDialogs::Show
    ${EndIf}
FunctionEnd

Function un.PAGE_SELECT_GAMES_Leave
    ${NSD_GetText} $PathDialogue_OB $Path_OB
    ${NSD_GetText} $PathDialogue_FO $Path_FO
    ${NSD_GetText} $PathDialogue_NV $Path_NV
    ${NSD_GetText} $PathDialogue_Nehrim $Path_Nehrim
    ${NSD_GetText} $PathDialogue_Other $Path_Other
    ${NSD_GetState} $Check_OB $CheckState_OB
    ${NSD_GetState} $Check_FO $CheckState_FO
    ${NSD_GetState} $Check_NV $CheckState_NV
    ${NSD_GetState} $Check_Nehrim $CheckState_Nehrim
    ${NSD_GetState} $Check_Other $CheckState_Other
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
Section "Uninstall"
    ${If} $CheckState_OB == ${BST_CHECKED}
        Delete "$Path_OB\Data\BOSS\*.lnk"
        Delete "$Path_OB\Data\BOSS\masterlist.txt"
        Delete "$Path_OB\Data\BOSS*.bat"
        Delete "$Path_OB\Data\BOSS\modlist.*"
        Delete "$Path_OB\Data\BOSS\uninstall.exe"
        RMDir "$Path_OB\Data\BOSS"
        Delete "$Path_OB\BOSS.exe"
        Delete "$SMPROGRAMS\BOSS\BOSS - Oblivion.lnk"
        DeleteRegValue HKLM "SOFTWARE\BOSS" "Oblivion Path"
        StrCpy $Path_OB $Empty
    ${EndIf}
    ${If} $CheckState_FO == ${BST_CHECKED}
        Delete "$Path_FO\Data\BOSS\*.lnk"
        Delete "$Path_FO\Data\BOSS\masterlist.txt"
        Delete "$Path_FO\Data\BOSS*.bat"
        Delete "$Path_FO\Data\BOSS\modlist.*"
        Delete "$Path_FO\Data\BOSS\uninstall.exe"
        RMDir "$Path_FO\Data\BOSS"
        Delete "$Path_FO\BOSS.exe"
        Delete "$SMPROGRAMS\BOSS\BOSS - Fallout3.lnk"
        DeleteRegValue HKLM "SOFTWARE\BOSS" "Fallout3 Path"
        StrCpy $Path_FO $Empty
    ${EndIf}
    ${If} $CheckState_NV == ${BST_CHECKED}
        Delete "$Path_NV\Data\BOSS\*.lnk"
        Delete "$Path_NV\Data\BOSS\masterlist.txt"
        Delete "$Path_NV\Data\BOSS*.bat"
        Delete "$Path_NV\Data\BOSS\modlist.*"
        Delete "$Path_NV\Data\BOSS\uninstall.exe"
        RMDir "$Path_NV\Data\BOSS"
        Delete "$Path_NV\BOSS.exe"
        Delete "$SMPROGRAMS\BOSS\BOSS - Fallout NewVegas.lnk"
        DeleteRegValue HKLM "SOFTWARE\BOSS" "NewVegas Path"
        StrCpy $Path_NV $Empty
    ${EndIf}
    ${If} $CheckState_Nehrim == ${BST_CHECKED}
        Delete "$Path_Nehrim\Data\BOSS\*.lnk"
        Delete "$Path_Nehrim\Data\BOSS\masterlist.txt"
        Delete "$Path_Nehrim\Data\BOSS*.bat"
        Delete "$Path_Nehrim\Data\BOSS\modlist.*"
        Delete "$Path_Nehrim\Data\BOSS\uninstall.exe"
        RMDir "$Path_Nehrim\Data\BOSS"
        Delete "$Path_Nehrim\BOSS.exe"
        Delete "$SMPROGRAMS\BOSS\BOSS - Nehrim.lnk"
        DeleteRegValue HKLM "SOFTWARE\BOSS" "Nehrim Path"
        StrCpy $Path_Nehrim $Empty
    ${EndIf}
    ${If} $CheckState_Other == ${BST_CHECKED}
        Delete "$Path_Other\Data\BOSS\*.lnk"
        Delete "$Path_Other\Data\BOSS\masterlist.txt"
        Delete "$Path_Other\Data\BOSS*.bat"
        Delete "$Path_Other\Data\BOSS\modlist.*"
        Delete "$Path_Other\Data\BOSS\uninstall.exe"
        RMDir "$Path_Other\Data\BOSS"
        Delete "$Path_Other\BOSS.exe"
        Delete "$SMPROGRAMS\BOSS\BOSS - Other.lnk"
        DeleteRegValue HKLM "SOFTWARE\BOSS" "Other Path"
        StrCpy $Path_Other $Empty
    ${EndIf}
    
    ;Is it a complete uninstall?
    ${If} $Path_OB == $Empty
        ${If} $Path_FO == $Empty
            ${If} $Path_NV == $Empty
                ${If} $Path_Nehrim == $Empty
                    ${If} $Path_Other == $Empty
                        DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS"
                        DeleteRegKey HKLM "SOFTWARE\BOSS" 
                        Delete "$SMPROGRAMS\BOSS\*.*"
                        RMDir "$SMPROGRAMS\BOSS"
                        Delete "$COMMONFILES\BOSS\*.*"
                        RMDir "$COMMONFILES\BOSS"
                    ${EndIf}
                ${EndIf}
            ${EndIf}
        ${EndIf}
    ${EndIf}
SectionEnd