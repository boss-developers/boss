@echo off
if exist messages.mo (
cmd /k "msgunfmt -o oldMessages.po messages.mo && xgettext --output=newMessages.po --keyword=translate:1,1t --keyword=translate:1c,2,2t --directory=..\src\boss-common\Common --directory=..\src\boss-gui\GUI --directory=. Error.h Error.cpp Game.cpp ..\src\boss-common\Output\Output.cpp ..\src\boss-cli\BOSS-CLI.cpp ElementIDs.cpp MainWindow.cpp SettingsWindow.cpp UserRuleEditor.cpp && msgmerge --output-file=messages.po oldMessages.po newMessages.po && DEL oldMessages.po && DEL newMessages.po"
) else (
 cmd /k "xgettext --keyword=translate:1,1t --directory=..\src\boss-common\Common --directory=..\src\boss-gui\GUI --directory=. Error.h Error.cpp Game.cpp ..\src\boss-common\Output\Output.cpp ..\src\boss-cli\BOSS-CLI.cpp ElementIDs.cpp MainWindow.cpp SettingsWindow.cpp UserRuleEditor.cpp"
)
