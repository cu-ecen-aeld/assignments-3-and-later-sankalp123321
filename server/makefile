# App name
APP_NAME=aesdsocket
# compiler
GCC=gcc
# Compiler flags
ifeq ($(CFLAGS),)
	C_FLAGS=-I. -g -Wall -Werror 
endif

ifeq ($(LDFLAGS),)
	LDFLAGS= -pthread -lrt
endif

# Include file if any exists
C_INCLUDE=.
# Obj file name
C_OBJ=build/aesdsocket.o
# Source file name
C_SRC=aesdsocket.c

# Command line variable
CROSS_COMPILE = 
# Append command line variable string
ifeq ($(CC),)
	CC:=$(CROSS_COMPILE)$(GCC)
endif

# Generate the object file using the source file
build/%.o : %.c $(C_INCLUDE)
	@mkdir -p build 
	$(CC) -c -o $@ $< $(C_FLAGS)
all: $(APP_NAME)
default: $(APP_NAME)

# Generate the app using the object file
$(APP_NAME) : $(C_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(C_FLAGS)

#clean the artifacts
clean:
	$(RM) -rf build $(APP_NAME)
