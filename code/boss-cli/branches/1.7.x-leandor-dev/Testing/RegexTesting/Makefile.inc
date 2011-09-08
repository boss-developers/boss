#
# Testing/RegexTesting/Makefile.inc


#
# Variables
#

PROG_REGEX_TESTING   := RegexTesting
TARGET_REGEX_TESTING := $(BUILD_DIR)/$(PROG_REGEX_TESTING)

REGEX_TESTING_SRC_DIR := Testing/RegexTesting
REGEX_TESTING_BLD_DIR := $(BUILD_DIR)/$(REGEX_TESTING_SRC_DIR)
REGEX_TESTING_SRCS    := $(wildcard $(REGEX_TESTING_SRC_DIR)/*.cpp)
REGEX_TESTING_OBJS    := $(addprefix $(BUILD_DIR)/,$(REGEX_TESTING_SRCS:.cpp=.o))
REGEX_TESTING_DEPS    := $(addprefix $(BUILD_DIR)/,$(REGEX_TESTING_SRCS:.cpp=.d))
REGEX_TESTING_LIBS    := boost_system boost_filesystem boost_regex


#
# Targets
#

.PHONY: $(PROG_REGEX_TESTING)

$(PROG_REGEX_TESTING): $(TARGET_SUPPORT) $(REGEX_TESTING_BLD_DIR) $(TARGET_REGEX_TESTING)

$(REGEX_TESTING_BLD_DIR):
	mkdir -p $@

$(TARGET_REGEX_TESTING): $(SUPPORT_OBJS) $(REGEX_TESTING_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(addprefix -l,$(REGEX_TESTING_LIBS)) $^

-include $(REGEX_TESTING_DEPS)

# vim:ft=make
