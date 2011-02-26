#
# Parsing/Userlist/Makefile.inc


#
# Variables
#

USERLIST_SRC_DIR := Parsing/Userlist
USERLIST_BLD_DIR := $(BUILD_DIR)/$(USERLIST_SRC_DIR)
USERLIST_SRCS    := $(wildcard $(USERLIST_SRC_DIR)/*.cpp)
USERLIST_OBJS    := $(addprefix $(BUILD_DIR)/,$(USERLIST_SRCS:.cpp=.o))
USERLIST_DEPS    := $(addprefix $(BUILD_DIR)/,$(USERLIST_SRCS:.cpp=.d))

TARGET_USERLIST := Userlist


#
# Targets
#

.PHONY: $(TARGET_USERLIST)

$(TARGET_USERLIST): $(USERLIST_BLD_DIR) $(USERLIST_OBJS)

$(USERLIST_BLD_DIR):
	mkdir -p $@

-include $(USERLIST_DEPS)

# vim:ft=make
