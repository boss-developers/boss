; BOSS.nsi
;
; This script is based on example2.nsi from the NSIS installation.

;--------------------------------

; The name of the installer
Name "BOSS"

; The file to write
OutFile "BOSS Installer.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Bethesda Softworks\Oblivion\Data"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\BOSS" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "BOSS (required)"

  SectionIn RO
  
  ; Set output path to the correct installation directory and install files.
  SetOutPath $INSTDIR
  File code\boss-common\trunk\bin\Release\BOSS.exe
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\BOSS" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "DisplayName" "BOSS"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "UninstallString" '"$INSTDIR\BOSS\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS" "NoRepair" 1
  WriteUninstaller "$INSTDIR\BOSS\uninstall.exe"
  
SectionEnd

; Optional sections (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\BOSS"
  CreateShortCut "$SMPROGRAMS\BOSS\Uninstall.lnk" "$INSTDIR\BOSS\uninstall.exe" "" "$INSTDIR\BOSS\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\BOSS\BOSS.lnk" "$INSTDIR\BOSS.exe" "" "$INSTDIR\BOSS.exe" 0
  
SectionEnd

Section "Documentation"

  SetOutPath $INSTDIR\BOSS
  File "data\boss-common\BOSS ReadMe.html"
  File "data\boss-common\BOSS User Rules ReadMe.html"
  
SectionEnd

Section "Batch Files"

  SetOutPath $INSTDIR
  File "data\boss-common\*.bat"
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  Var /GLOBAL indir
  ReadRegStr $indir HKLM "Software\BOSS" "Install_Dir"
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BOSS"
  DeleteRegKey HKLM "SOFTWARE\BOSS"

  ; Remove files and uninstaller - but not userlist.
  ; Directories are only deleted if empty.
  Delete "$indir\BOSS\*.html"
  Delete "$indir\BOSS\masterlist.txt"
  Delete "$indir\BOSS*.bat"
  Delete "$indir\BOSS\modlist.*"
  Delete "$indir\BOSS\uninstall.exe"
  RMDir "$indir\BOSS"
  Delete "$indir\BOSS.exe"
  
  ; Remove StartMenu stuf if created
  Delete "$SMPROGRAMS\BOSS\*.*"
  RMDir "$SMPROGRAMS\BOSS"

SectionEnd