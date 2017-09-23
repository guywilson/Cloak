###############################################################################
#                                                                             #
# MAKEFILE for Cloak application                                              #
#                                                                             #
# © Guy Wilson 2012                                                           #
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
TARGET=cloak.exe

# Deploy Location
DEPLOYLOC=C:\Users\guy\bin

# libpng includes
PNGH=.\pnglib

# libpng dependencies
PNGLIB=.\pnglib\vcwin32

# C++ compiler
CPP=cl

# Linker
LINKER=link

# Target Architecture
MACHINE=/MACHINE:x64

# C++ compiler flags
CFLAGS_REL=-c -W3 -Ot -Gs -nologo -I $(PNGH) -D_CRT_SECURE_NO_WARNINGS
CFLAGS_DBG=-c -W3 -Ot -Gs -Zi -nologo -I $(PNGH) -D_CRT_SECURE_NO_WARNINGS -DDEBUG_MEMORY

CPPFLAGS_REL=-c -W3 -Ot -Gs -EHsc -nologo -I $(PNGH) -D_CRT_SECURE_NO_WARNINGS
CPPFLAGS_DBG=-c -W3 -Ot -Gs -EHsc -Zi -nologo -I $(PNGH) -D_CRT_SECURE_NO_WARNINGS -DDEBUG_MEMORY

CFLAGS=$(CFLAGS_REL)
CPPFLAGS=$(CPPFLAGS_REL)

# Linker flags (Release)
LFLAGS_REL=$(MACHINE) /NOLOGO
LFLAGS_DBG=$(LFLAGS_REL) /DEBUG /ASSEMBLYDEBUG

LFLAGS=$(LFLAGS_REL)

# Object files (in linker ',' seperated format)
OBJFILES=$(BUILD)\main.obj $(BUILD)\md5.obj $(BUILD)\exception.obj $(BUILD)\iterator.obj $(BUILD)\image.obj $(BUILD)\data.obj $(BUILD)\encryption.obj $(BUILD)\cloak.obj $(BUILD)\pngreadwrite.obj $(BUILD)\aes.obj

# Target
all: $(TARGET) pngtopng.exe savepng.exe

# Compile C++ source files
#
$(BUILD)\md5.obj: $(SOURCE)\md5.cpp $(SOURCE)\md5.h $(SOURCE)\types.h
	$(CPP) $(CPPFLAGS) -Fd$(BUILD)\md5.pdb -Fo$(BUILD)\md5.obj $(SOURCE)\md5.cpp

$(BUILD)\exception.obj: $(SOURCE)\exception.cpp $(SOURCE)\exception.h $(SOURCE)\errorcodes.h $(SOURCE)\types.h
	$(CPP) $(CPPFLAGS) -Fd$(BUILD)\exception.pdb -Fo$(BUILD)\exception.obj $(SOURCE)\exception.cpp

$(BUILD)\iterator.obj: $(SOURCE)\iterator.cpp $(SOURCE)\iterator.h $(SOURCE)\types.h
	$(CPP) $(CPPFLAGS) -Fd$(BUILD)\iterator.pdb -Fo$(BUILD)\iterator.obj $(SOURCE)\iterator.cpp

$(BUILD)\image.obj: $(SOURCE)\image.cpp $(SOURCE)\image.h $(SOURCE)\iterator.h $(SOURCE)\exception.h $(SOURCE)\errorcodes.h $(SOURCE)\types.h $(SOURCE)\pngreadwrite.h
	$(CPP) $(CPPFLAGS) -Fd$(BUILD)\image.pdb -Fo$(BUILD)\image.obj $(SOURCE)\image.cpp
	
$(BUILD)\data.obj: $(SOURCE)\data.cpp $(SOURCE)\data.h $(SOURCE)\iterator.h $(SOURCE)\encryption.h $(SOURCE)\exception.h $(SOURCE)\errorcodes.h $(SOURCE)\types.h
	$(CPP) $(CPPFLAGS) -Fd$(BUILD)\data.pdb -Fo$(BUILD)\data.obj $(SOURCE)\data.cpp
	
$(BUILD)\encryption.obj: $(SOURCE)\encryption.cpp $(SOURCE)\encryption.h $(SOURCE)\md5.h $(SOURCE)\exception.h $(SOURCE)\errorcodes.h $(SOURCE)\types.h $(SOURCE)\salt.h $(SOURCE)\key.h
	$(CPP) $(CPPFLAGS) -Fd$(BUILD)\encryption.pdb -Fo$(BUILD)\encryption.obj $(SOURCE)\encryption.cpp

$(BUILD)\cloak.obj: $(SOURCE)\cloak.cpp $(SOURCE)\cloak.h $(SOURCE)\image.h $(SOURCE)\data.h $(SOURCE)\iterator.h $(SOURCE)\exception.h $(SOURCE)\errorcodes.h $(SOURCE)\types.h
	$(CPP) $(CPPFLAGS) -Fd$(BUILD)\cloak.pdb -Fo$(BUILD)\cloak.obj $(SOURCE)\cloak.cpp

$(BUILD)\main.obj: $(SOURCE)\main.cpp $(SOURCE)\cloak.h $(SOURCE)\image.h $(SOURCE)\data.h $(SOURCE)\iterator.h $(SOURCE)\exception.h $(SOURCE)\errorcodes.h $(SOURCE)\types.h
	$(CPP) $(CPPFLAGS) -Fd$(BUILD)\main.pdb -Fo$(BUILD)\main.obj $(SOURCE)\main.cpp

$(BUILD)\aes.obj: $(SOURCE)\aes.c $(SOURCE)\aes.h
	$(CPP) $(CFLAGS) -Fd$(BUILD)\aes.pdb -Fo$(BUILD)\aes.obj /Tc$(SOURCE)\aes.c

$(BUILD)\pngtopng.obj: $(SOURCE)\pngtopng.c
	$(CPP) $(CFLAGS) -Fd$(BUILD)\pngtopng.pdb -Fo$(BUILD)\pngtopng.obj /Tc$(SOURCE)\pngtopng.c

$(BUILD)\savepng.obj: $(SOURCE)\savepng.c
	$(CPP) $(CFLAGS) -Fd$(BUILD)\savepng.pdb -Fo$(BUILD)\savepng.obj /Tc$(SOURCE)\savepng.c

$(BUILD)\pngreadwrite.obj: $(SOURCE)\pngreadwrite.c $(SOURCE)\pngreadwrite.h $(SOURCE)\writepng.h
	$(CPP) $(CFLAGS) -Fd$(BUILD)\pngreadwrite.pdb -Fo$(BUILD)\pngreadwrite.obj /Tc$(SOURCE)\pngreadwrite.c

$(TARGET): $(OBJFILES)
	$(LINKER) $(LFLAGS) /MAP:out.map /PDB:out.pdb /OUT:$(TARGET) $(OBJFILES) /NODEFAULTLIB:libcmt msvcrt.lib $(PNGLIB)\libpng.lib $(PNGLIB)\zlib.lib
	copy $(TARGET) $(DEPLOYLOC)

pngtopng.exe: $(BUILD)\pngtopng.obj
	$(LINKER) $(LFLAGS_REL) /OUT:pngtopng.exe $(BUILD)\pngtopng.obj /NODEFAULTLIB:libcmt msvcrt.lib $(PNGLIB)\libpng.lib $(PNGLIB)\zlib.lib

savepng.exe: $(BUILD)\savepng.obj
	$(LINKER) $(LFLAGS_REL) /OUT:savepng.exe $(BUILD)\savepng.obj /NODEFAULTLIB:libcmt msvcrt.lib $(PNGLIB)\libpng.lib $(PNGLIB)\zlib.lib
