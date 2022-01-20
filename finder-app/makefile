# App name
APP_NAME=writer
# compiler
GCC=gcc
# Compiler flags
C_FLAGS=-I. -g -Wall
# Include file if any exists
C_INCLUDE=.
# Obj file name
C_OBJ=build/writer.o
# Source file name
C_SRC=writer.c

# Command line variable
CROSS_COMPILE = 
# Append command line variable string
CC:=$(CROSS_COMPILE)$(GCC)

# Generate the object file using the source file
build/%.o : %.c $(C_INCLUDE)
	@mkdir build
	$(CC) -c -o $@ $< $(C_FLAGS)

# Generate the app using the object file
$(APP_NAME) : $(C_OBJ)
	$(CC) -o $@ $^ $(C_FLAGS)

#clean the artifacts
clean:
	@rm -rf build
	@rm $(APP_NAME)