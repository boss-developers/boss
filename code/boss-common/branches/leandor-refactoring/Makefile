#Makefile for use in Linux

CC      = g++

C_FLAGS = -Wall -Wshadow -Wformat-security -Wpointer-arith -Wcast-align -Wredundant-decls

C_FILES = Common/BOSS.cpp Common/Globals.cpp Common/Masterlist.cpp Common/Sorting.cpp Common/Updater.cpp Common/Userlist.cpp \
          Support/VersionRegex.cpp Support/ModFormat.cpp Support/Helpers.cpp

O_FILES = $(C_FILES:.cpp=.o)

.cpp.o:
	$(CC) $(C_FLAGS) -c $<

all: boss

boss: $(O_FILES)
	$(CC) -o BOSS $(O_FILES)
