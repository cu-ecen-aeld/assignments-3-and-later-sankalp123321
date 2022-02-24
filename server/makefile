# App name
APP_NAME=aesdsocket
# compiler
GCC=gcc
# Compiler flags
C_FLAGS=-I. -g -Wall -lpthread
# Include file if any exists
C_INCLUDE=.
# Obj file name
C_OBJ=build/aesdsocketThread.o
# Source file name
C_SRC=aesdsocketThread.c

# Command line variable
CROSS_COMPILE = 
# Append command line variable string
CC:=$(CROSS_COMPILE)$(GCC)

# Generate the object file using the source file
build/%.o : %.c $(C_INCLUDE)
	@mkdir -p build 
	$(CC) -c -o $@ $< $(C_FLAGS)
all: $(APP_NAME)
default: $(APP_NAME)

# Generate the app using the object file
$(APP_NAME) : $(C_OBJ)
	$(CC) -o $@ $^ $(C_FLAGS)

#clean the artifacts
clean:
	$(RM) -rf build $(APP_NAME)
