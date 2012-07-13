xgettext --keyword=translate:1,1t ..\boss-common\Common\Error.h ..\boss-common\Common\Error.cpp ..\boss-common\Common\Game.cpp ..\boss-common\Output\Output.cpp

msgen --output-file=messages.po messages.po

msgfmt --check messages.po