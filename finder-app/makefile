APP_NAME=writer
GCC=gcc
C_FLAGS=-I. -g -Wall
C_INCLUDE=.
C_OBJ=build/writer.o
C_SRC=writer.c

CROSS_COMPILE = 
CC:=$(CROSS_COMPILE)$(GCC)

build/%.o : %.c $(C_INCLUDE)
	@mkdir build
	$(CC) -c -o $@ $< $(C_FLAGS)

$(APP_NAME) : $(C_OBJ)
	$(CC) -o $@ $^ $(C_FLAGS)

clean:
	@rm -rf build
	@rm $(APP_NAME)