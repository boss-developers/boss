#
# Support/Makefile.inc


#
# Variables
#

SUPPORT_SRC_DIR := Support
SUPPORT_BLD_DIR := $(BUILD_DIR)/$(SUPPORT_SRC_DIR)
SUPPORT_SRCS    := $(wildcard $(SUPPORT_SRC_DIR)/*.cpp)
SUPPORT_OBJS    := $(addprefix $(BUILD_DIR)/,$(SUPPORT_SRCS:.cpp=.o))
SUPPORT_DEPS    := $(addprefix $(BUILD_DIR)/,$(SUPPORT_SRCS:.cpp=.d))

TARGET_SUPPORT := $(SUPPORT_SRC_DIR)


#
# Targets
#

.PHONY: $(TARGET_SUPPORT)

$(TARGET_SUPPORT): $(SUPPORT_BLD_DIR) $(SUPPORT_OBJS)

$(SUPPORT_BLD_DIR):
	mkdir -p $@

-include $(SUPPORT_DEPS)

# vim:ft=make
