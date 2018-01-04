###############################################################################
#                                                                             #
# MAKEFILE for Cloak application                                              #
#                                                                             #
# � Guy Wilson 2012                                                           #
#                                                                             #
# Build using Visual Studio C++                                               #
# Use 'nmake' to build with this makefile, 'nmake -a' to rebuild all          #
#                                                                             #
###############################################################################

# Source directory
SOURCE=src

# Build output directory
BUILD=build

# What is our target
TARGET=cloak

# C++ compiler
CPP=g++
C=gcc

# Linker
LINKER=g++

DBG=-g

# C++ compiler flags
CPPFLAGS_REL=-c -fpermissive -Wall -std=c++11
CPPFLAGS_DBG=-c -fpermissive -Wall -std=c++11 $(DBG)

CFLAGS_REL=-c -Wall
CFLAGS_DBG=-c -Wall $(DBG)

CPPFLAGS=$(CPPFLAGS_REL)
CFLAGS=$(CFLAGS_REL)

#Linker flags
LFLAGS_REL=-lstdc++
LFLAGS_DBG=-lstdc++ $(DBG)

LFLAGS=$(LFLAGS_REL)
LIBS=-lpng -lz -lgcrypt

# Object files (in linker ',' seperated format)
OBJFILES=$(BUILD)/main.o $(BUILD)/exception.o $(BUILD)/iterator.o $(BUILD)/image.o $(BUILD)/data.o $(BUILD)/encryption.o $(BUILD)/cloak.o $(BUILD)/pngreadwrite.o

# Target
all: $(TARGET)

# Compile C++ source files
#
$(BUILD)/exception.o: $(SOURCE)/exception.cpp $(SOURCE)/exception.h $(SOURCE)/errorcodes.h $(SOURCE)/types.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/exception.o $(SOURCE)/exception.cpp

$(BUILD)/iterator.o: $(SOURCE)/iterator.cpp $(SOURCE)/iterator.h $(SOURCE)/types.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/iterator.o $(SOURCE)/iterator.cpp

$(BUILD)/image.o: $(SOURCE)/image.cpp $(SOURCE)/image.h $(SOURCE)/iterator.h $(SOURCE)/exception.h $(SOURCE)/errorcodes.h $(SOURCE)/types.h $(SOURCE)/pngreadwrite.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/image.o $(SOURCE)/image.cpp

$(BUILD)/data.o: $(SOURCE)/data.cpp $(SOURCE)/data.h $(SOURCE)/iterator.h $(SOURCE)/encryption.h $(SOURCE)/exception.h $(SOURCE)/errorcodes.h $(SOURCE)/types.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/data.o $(SOURCE)/data.cpp

$(BUILD)/encryption.o: $(SOURCE)/encryption.cpp $(SOURCE)/encryption.h $(SOURCE)/exception.h $(SOURCE)/errorcodes.h $(SOURCE)/types.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/encryption.o $(SOURCE)/encryption.cpp

$(BUILD)/cloak.o: $(SOURCE)/cloak.cpp $(SOURCE)/cloak.h $(SOURCE)/image.h $(SOURCE)/data.h $(SOURCE)/iterator.h $(SOURCE)/exception.h $(SOURCE)/errorcodes.h $(SOURCE)/types.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/cloak.o $(SOURCE)/cloak.cpp

$(BUILD)/main.o: $(SOURCE)/main.cpp $(SOURCE)/cloak.h $(SOURCE)/image.h $(SOURCE)/data.h $(SOURCE)/iterator.h $(SOURCE)/exception.h $(SOURCE)/errorcodes.h $(SOURCE)/types.h
	$(CPP) $(CPPFLAGS) -o $(BUILD)/main.o $(SOURCE)/main.cpp

$(BUILD)/pngreadwrite.o: $(SOURCE)/pngreadwrite.c $(SOURCE)/pngreadwrite.h $(SOURCE)/writepng.h
	$(C) $(CFLAGS) -I/usr/local/include -o $(BUILD)/pngreadwrite.o $(SOURCE)/pngreadwrite.c

$(TARGET): $(OBJFILES)
	$(LINKER) $(LFLAGS) -o $(TARGET) $(OBJFILES) $(LIBS)

install: $(TARGET)
	cp ./$(TARGET) /usr/local/bin
