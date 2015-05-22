@echo off
if exist messages.mo (
cmd /k "msgunfmt -o oldMessages.po messages.mo && xgettext --output=newMessages.po --keyword=translate:1,1t --keyword=translate:1c,2,2t --directory=..\boss-common\Common --directory=..\boss-gui\GUI --directory=. Error.h Error.cpp Game.cpp ..\boss-common\Output\Output.cpp ..\boss-cli\BOSS-CLI.cpp ElementIDs.cpp MainWindow.cpp SettingsWindow.cpp UserRuleEditor.cpp && msgmerge --output-file=messages.po oldMessages.po newMessages.po && DEL oldMessages.po && DEL newMessages.po"
) else (
 cmd /k "xgettext --keyword=translate:1,1t --directory=..\boss-common\Common --directory=..\boss-gui\GUI --directory=. Error.h Error.cpp Game.cpp ..\boss-common\Output\Output.cpp ..\boss-cli\BOSS-CLI.cpp ElementIDs.cpp MainWindow.cpp SettingsWindow.cpp UserRuleEditor.cpp"
)
