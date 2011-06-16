#
# Testing/Makefile.inc

include Testing/DumpHeaders/Makefile.inc
include Testing/RegexTesting/Makefile.inc

TESTING_SRC_DIR = Testing
TARGET_TESTING := $(TESTING_SRC_DIR)

.PHONY: Testing

$(TARGET_TESTING): $(PROG_DUMP_HEADERS) $(PROG_REGEX_TESTING)

# vim:ft=make
