xgettext --keyword=translate:1,1t --directory=..\boss-common\Common --directory=..\boss-gui\GUI --directory=. Error.h Error.cpp Game.cpp ..\boss-common\Output\Output.cpp ..\boss-cli\BOSS-CLI.cpp MainWindow.cpp SettingsWindow.cpp UserRuleEditor.cpp

msgen --output-file=messages.po messages.po