ifneq (,$(findstring mingw,$(CXX)))
	include Makefile.win
else ifneq (,$(findstring .exe,$(suffix $(CXX))))
	include Makefile.win
else
	include Makefile.nix
endif

