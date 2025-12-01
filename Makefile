# ** TEST MAKEFILE **
# It doesn't do anything!
# 
# #####
# 
# This Makefile is just an example how to compile your
# own source files. 

SRC_DIR = $(shell pwd -P)
DICT_DIR  = $(SRC_DIR) # for example.
BUILD_DIR = $(SRC_DIR) # for example.

CXXFLAGS := $(shell root-config --cflags) -Wall -MMD -MP -fPIC \

include common.mk

LDFLAGS := $(shell root-config --ldflags)
LIBS := $(shell root-config --libs) -lROOTNTuple

SRC := $(wildcard $(SRC_DIR)/*.cxx)
HEADERS := $(wildcard $(SRC_DIR)/*.h)

OBJ := $(patsubst $(SRC_DIR)/%.cxx, $(BUILD_DIR)/%.o, $(SRC))

# Target of this example Makefile is to build and compile
# together a dictionary for all the structures needed in the project,
# and stuff it into a lib.
TARGET := Structures
DICT := $(DICT_DIR)/$(TARGET)Dict.cxx
DICT_PCM := $(DICT_DIR)/$(TARGET)Dict_rdict.pcm

DICT_OBJ := $(patsubst $(DICT_DIR)/%.cxx, $(BUILD_DIR)/%.o, $(DICT))
GEN_LIB = $(BUILD_DIR)/lib$(TARGET).so
GEN_MAP = $(BUILD_DIR)/lib$(TARGET).rootmap

RDFLAGS = -I./ -I../ -v3 \
		  -Wall -MMD -MP -rml $(GEN_LIB) -rmf $(GEN_MAP)

.PHONY: all
all: __test__

__test__ :
	@echo "Empty make, I'm just printing..."
	@echo "Have fun using Monad."
	@echo "-- 𝒦𝓁𝒶𝓎𝓏𝓮"

# Rules' sequence should be something like:

#all: $(GEN_LIB) $(DICT)
#
#$(GEN_LIB) : $(OBJ) $(DICT_OBJ)
#	$(CXX) $(LDFLAGS) -shared -o $@ $^ $(LIBS)	
#
#$(OBJ) : $(BUILD_DIR)/%.o : $(SRC_DIR)/%.cxx $(SRC_DIR)/%.h $(DICT_OBJ)
#	@mkdir -p $(@D)
#	$(CXX) -c -o $@ $< $(CXXFLAGS)
#
#$(DICT_OBJ) : $(DICT)
#	@mkdir -p $(@D)
#	$(CXX) -c -o $@ $^ $(CXXFLAGS)
#
#$(DICT) : $(HEADERS) LinkDef.hh 
#	@mkdir -p $(DICT_DIR) $(BUILD_DIR)
#	rootcling -f $@ $(RDFLAGS) -p $^
#
#clean:
#	@rm -rf $(OBJ) $(GEN_LIB) $(GEN_MAP) $(DICT) $(DICT_PCM)
