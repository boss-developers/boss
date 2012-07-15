msgunfmt --output-file=oldMessages.po messages.mo

xgettext --output=newMessages.po --keyword=translate:1,1t --directory=..\boss-common\Common --directory=..\boss-gui\GUI --directory=. Error.h Error.cpp Game.cpp ..\boss-common\Output\Output.cpp ..\boss-cli\BOSS-CLI.cpp MainWindow.cpp SettingsWindow.cpp UserRuleEditor.cpp

msgmerge --output-file=messages.po oldMessages.po newMessages.po