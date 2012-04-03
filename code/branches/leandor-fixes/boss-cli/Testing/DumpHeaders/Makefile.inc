#
# Testing/DumpHeaders/Makefile.inc


#
# Variables
#

PROG_DUMP_HEADERS  := DumpHeaders
TARGET_DUMP_HEADERS  := $(BUILD_DIR)/$(PROG_DUMP_HEADERS)

DUMP_HEADERS_SRC_DIR := Testing/DumpHeaders
DUMP_HEADERS_BLD_DIR := $(BUILD_DIR)/$(DUMP_HEADERS_SRC_DIR)
DUMP_HEADERS_SRCS    := $(wildcard $(DUMP_HEADERS_SRC_DIR)/*.cpp)
DUMP_HEADERS_OBJS    := $(addprefix $(BUILD_DIR)/,$(DUMP_HEADERS_SRCS:.cpp=.o))
DUMP_HEADERS_DEPS    := $(addprefix $(BUILD_DIR)/,$(DUMP_HEADERS_SRCS:.cpp=.d))
DUMP_HEADERS_LIBS    := boost_system boost_regex boost_filesystem


#
# Targets
#

.PHONY: $(PROG_DUMP_HEADERS)

$(PROG_DUMP_HEADERS): $(TARGET_SUPPORT) $(DUMP_HEADERS_BLD_DIR) $(TARGET_DUMP_HEADERS)

$(DUMP_HEADERS_BLD_DIR):
	mkdir -p $@

$(TARGET_DUMP_HEADERS): $(SUPPORT_OBJS) $(DUMP_HEADERS_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(addprefix -l,$(DUMP_HEADERS_LIBS)) $^

-include $(DUMP_HEADERS_DEPS)

# vim:ft=make
