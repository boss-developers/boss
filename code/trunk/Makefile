CXXFLAGS = -O3 -Iboss-common -I../../../libgit2/include -std=c++11
#CFLAGS += -O3 -Iboss-common -I../../../libgit2/include -std=c++11
#CPPFLAGS += -I boss-common -I ../../../libgit2/include
LDFLAGS += -static -Llib
LDLIBS += -lboost_filesystem -lboost_regex -lboost_program_options -lboost_exception -lboost_system -lboost_locale-mt -lversion -lgit2 -lssl -lcrypto -lgdi32 -lz -lws2_32

#%.d: %.cpp
#		@set -e; rm -f $@; \
#		$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
#		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
#		rm -f $@.$$$$


DIR1 = boss-cli
DIR2 = boss-common
DIR3 = boss-gui

OBJECTS = 							$(DIR1)/BOSS-CLI.o $(DIR2)/Common/Globals.o \
									$(DIR2)/Common/Classes.o $(DIR2)/Common/Game.o \
									$(DIR2)/Common/Error.o $(DIR2)/Output/Output.o \
									$(DIR2)/Support/Helpers.o $(DIR2)/Support/Logger.o \
									$(DIR2)/Support/ModFormat.o $(DIR2)/Parsing/Grammar.o \
									$(DIR2)/Support/VersionRegex.o


bossCLI.exe :						$(OBJECTS)
									$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@


$(DIR1)/BOSS-CLI.o :				$(DIR2)/BOSS-Common.h $(DIR2)/Common/Globals.h $(DIR2)/Common/Classes.h $(DIR2)/Common/Game.h $(DIR2)/Common/Error.h $(DIR2)/Output/Output.h \
									$(DIR2)/Support/Helpers.h $(DIR2)/Support/Logger.h $(DIR2)/Updating/Updater.h
$(DIR2)/Common/Globals.o :			$(DIR2)/Common/Globals.h $(DIR2)/Common/Game.h $(DIR2)/Common/DllDef.h
$(DIR2)/Common/Classes.o :			$(DIR2)/Common/Classes.h $(DIR2)/Common/Game.h $(DIR2)/Common/Globals.h $(DIR2)/Output/Output.h $(DIR2)/Support/Logger.h $(DIR2)/Support/ModFormat.h \
									$(DIR2)/Parsing/Grammar.h $(DIR2)/Common/DllDef.h $(DIR2)/Common/Error.h $(DIR2)/Support/Helpers.h
$(DIR2)/Common/Game.o :				$(DIR2)/Common/Game.h $(DIR2)/Common/Globals.h $(DIR2)/Support/Helpers.h $(DIR2)/Support/Logger.h $(DIR2)/Common/Classes.h $(DIR2)/Output/Output.h
$(DIR2)/Common/Error.o :			$(DIR2)/Common/Error.h $(DIR2)/Common/DllDef.h
$(DIR2)/Output/Output.o :			$(DIR2)/Output/Output.h $(DIR2)/Common/Error.h $(DIR2)/Common/Globals.h $(DIR2)/Support/Helpers.h $(DIR2)/Common/Classes.h $(DIR2)/Common/DllDef.h
$(DIR2)/Support/Helpers.o :			$(DIR2)/Support/Helpers.h $(DIR2)/Support/Types.h $(DIR2)/Support/ModFormat.h $(DIR2)/Support/Logger.h $(DIR2)/Common/Globals.h $(DIR2)/alphanum.hpp $(DIR2)/Common/DllDef.h
$(DIR2)/Support/Logger.o :			$(DIR2)/Support/Logger.h $(DIR2)/Support/Platform.h $(DIR2)/Common/DllDef.h
$(DIR2)/Support/ModFormat.o :		$(DIR2)/Support/ModFormat.h $(DIR2)/Support/Types.h $(DIR2)/Support/Helpers.h $(DIR2)/Support/VersionRegex.h
$(DIR2)/Parsing/Grammar.o :			$(DIR2)/Parsing/Grammar.h $(DIR2)/Common/Globals.h $(DIR2)/Common/Error.h $(DIR2)/Support/Helpers.h $(DIR2)/Support/Logger.h \
									$(DIR2)/Output/Output.h $(DIR2)/Common/Classes.h $(DIR2)/Common/Game.h
$(DIR2)/Support/VersionRegex.o :	$(DIR2)/Support/VersionRegex.h

.PHONY :							clean
clean :
									rm -f $(OBJECTS)