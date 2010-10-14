#
# Makefile for use in Linux
#
# build requirements: boost, curl


#
# Variables
#

TARGET = BOSS

SRCS := $(wildcard Common/*cpp) $(wildcard Support/*cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)

CXX      := g++
CXXFLAGS := -O2 -pipe -Wall -Wno-multichar
LIBS     := boost_system boost_regex boost_filesystem curl


#
# Targets
#

.PHONY: all clean

all: $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(<:.cpp=.d) -o $@ -c $<

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(addprefix -l,$(LIBS)) $^

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)
