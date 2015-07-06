CXXFLAGS = -O3 -Isrc -I../libgit2/include -std=c++11
#CFLAGS += -O3 -Iboss-common -I../../../libgit2/include -std=c++11
#CPPFLAGS += -I boss-common -I ../../../libgit2/include
LDFLAGS += -static -Llib
LDLIBS += -lboost_filesystem -lboost_regex -lboost_program_options -lboost_exception -lboost_system -lboost_locale-mt -lversion -lgit2 -lssl -lcrypto -lgdi32 -lz -lws2_32

#%.d: %.cpp
#		@set -e; rm -f $@; \
#		$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
#		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
#		rm -f $@.$$$$


DIR1 = src
DIR2 = src
DIR3 = src

OBJECTS = 							$(DIR1)/boss_cli.o $(DIR2)/common/globals.o \
									$(DIR2)/common/classes.o $(DIR2)/common/game.o \
									$(DIR2)/common/error.o $(DIR2)/output/output.o \
									$(DIR2)/support/helpers.o $(DIR2)/support/logger.o \
									$(DIR2)/support/mod_format.o $(DIR2)/parsing/grammar.o \
									$(DIR2)/support/version_regex.o


bossCLI.exe :						$(OBJECTS)
									$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@


$(DIR1)/boss_cli.o :				$(DIR2)/boss_common.h $(DIR2)/common/globals.h $(DIR2)/common/classes.h $(DIR2)/common/game.h $(DIR2)/common/error.h $(DIR2)/output/output.h \
									$(DIR2)/support/helpers.h $(DIR2)/support/logger.h $(DIR2)/updating/updater.h
$(DIR2)/common/globals.o :			$(DIR2)/common/globals.h $(DIR2)/common/game.h $(DIR2)/common/dll_def.h
$(DIR2)/common/classes.o :			$(DIR2)/common/classes.h $(DIR2)/common/game.h $(DIR2)/common/globals.h $(DIR2)/output/output.h $(DIR2)/support/logger.h $(DIR2)/support/mod_format.h \
									$(DIR2)/parsing/grammar.h $(DIR2)/common/dll_def.h $(DIR2)/common/error.h $(DIR2)/support/helpers.h
$(DIR2)/common/game.o :				$(DIR2)/common/game.h $(DIR2)/common/globals.h $(DIR2)/support/helpers.h $(DIR2)/support/logger.h $(DIR2)/common/classes.h $(DIR2)/output/output.h
$(DIR2)/common/error.o :			$(DIR2)/common/error.h $(DIR2)/common/dll_def.h
$(DIR2)/output/output.o :			$(DIR2)/output/output.h $(DIR2)/common/error.h $(DIR2)/common/globals.h $(DIR2)/support/helpers.h $(DIR2)/common/classes.h $(DIR2)/common/dll_def.h
$(DIR2)/support/helpers.o :			$(DIR2)/support/helpers.h $(DIR2)/support/types.h $(DIR2)/support/mod_format.h $(DIR2)/support/logger.h \
									$(DIR2)/common/globals.h $(DIR2)/alphanum.hpp $(DIR2)/common/dll_def.h
$(DIR2)/support/logger.o :			$(DIR2)/support/logger.h $(DIR2)/support/platform.h $(DIR2)/common/dll_def.h
$(DIR2)/support/mod_format.o :		$(DIR2)/support/mod_format.h $(DIR2)/support/types.h $(DIR2)/support/helpers.h $(DIR2)/support/version_regex.h
$(DIR2)/parsing/grammar.o :			$(DIR2)/parsing/grammar.h $(DIR2)/common/globals.h $(DIR2)/common/error.h $(DIR2)/support/helpers.h $(DIR2)/support/logger.h \
									$(DIR2)/output/output.h $(DIR2)/common/classes.h $(DIR2)/common/game.h
$(DIR2)/support/version_regex.o :	$(DIR2)/support/version_regex.h

.PHONY :							clean
clean :
									rm -f $(OBJECTS)