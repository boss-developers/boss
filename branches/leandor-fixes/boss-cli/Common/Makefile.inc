#
# Common/Makefile.inc


#
# Variables
#

COMMON_SRC_DIR := Common
COMMON_BLD_DIR := $(BUILD_DIR)/$(COMMON_SRC_DIR)
COMMON_SRCS    := $(wildcard $(COMMON_SRC_DIR)/*.cpp)
COMMON_OBJS    := $(addprefix $(BUILD_DIR)/,$(COMMON_SRCS:.cpp=.o))
COMMON_DEPS    := $(addprefix $(BUILD_DIR)/,$(COMMON_SRCS:.cpp=.d))

TARGET_COMMON := $(COMMON_SRC_DIR)


#
# Targets
#

.PHONY: $(TARGET_COMMON)

$(TARGET_COMMON): $(COMMON_BLD_DIR) $(COMMON_OBJS)

$(COMMON_BLD_DIR):
	mkdir -p $@

-include $(COMMON_DEPS)

# vim:ft=make
