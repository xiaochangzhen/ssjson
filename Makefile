############################################################################
#
# Makefile for ssj
#
# (C) Copyright xiaochangzhen@yeah.net
#
# Thu Nov 29 10:40:23 CST 2018	xcz		File created
#
############################################################################

CROSS	=
PLATFORM =
PLATFORM_FLAGS = 

ifeq (x$(PLATFORM)y,xy)
PLATFORM=x86
endif

CC	= @echo " GCC	$@"; $(CROSS)gcc
CPP	= @echo " G++	$@"; $(CROSS)g++
LD	= @echo " LD	$@"; $(CROSS)ld
AR	= @echo " AR	$@"; $(CROSS)ar
STRIP	= @echo " STRIP $@"; $(CROSS)strip

CFLAGS += -Iinclude 
TARGET = libjjs.a 

SOURCE = $(wildcard src/*.c)

OBJS = $(patsubst src/%.c,src/%.o,$(SOURCE))

TEST = test_check test_fbdio

all: $(TARGET) $(TEST)

$(TARGET): $(OBJS)
	$(AR) -rv  $@ $^

test_check: test/test.o $(TARGET)
	$(CC) $(PLATFORM_FLAGS) $^ -o $@ $(LIBS) -lm
	$(STRIP) $@

test_fbdio: test/fbdio.o $(TARGET)
	$(CC) $(PLATFORM_FLAGS) $^ -o $@ $(LIBS) -lm
	$(STRIP) $@

.c.o:
	$(CC)  $(CFLAGS) $(PLATFORM_FLAGS) -c -o $@ $<
.cpp.o:
	$(CPP) $(CFLAGS) $(PLATFORM_FLAGS) -c -o $@ $<

clean:
	@find ./ -type f -name '*.o' | xargs rm -f
	@rm -f $(TARGET) $(OBJS) $(TEST) 

.PHONY: all clean
