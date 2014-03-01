#
# Testing/Makefile.inc

include Testing/DumpHeaders/Makefile.inc
include Testing/RegexTesting/Makefile.inc

.PHONY: Testing

Testing: $(PROG_DUMP_HEADERS) $(PROG_REGEX_TESTING)

# vim:ft=make
