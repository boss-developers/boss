#
# Parsing/Makefile.inc


#
# Variables
#

PARSING_SRC_DIR := Parsing
PARSING_BLD_DIR := $(BUILD_DIR)/$(PARSING_SRC_DIR)
PARSING_SRCS    := $(wildcard $(PARSING_SRC_DIR)/*.cpp)
PARSING_OBJS    := $(addprefix $(BUILD_DIR)/,$(PARSING_SRCS:.cpp=.o))
PARSING_DEPS    := $(addprefix $(BUILD_DIR)/,$(PARSING_SRCS:.cpp=.d))

TARGET_PARSING := $(PARSING_SRC_DIR)


#
# Targets
#

.PHONY: $(TARGET_PARSING)

$(TARGET_PARSING): $(PARSING_BLD_DIR) $(PARSING_OBJS)

$(PARSING_BLD_DIR):
	mkdir -p $@

-include $(PARSING_DEPS)

# vim:ft=make
