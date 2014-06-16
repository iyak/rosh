CC = gcc
CFLAGS += -Wall -Wextra -MMD -O3
LDFLAGS += -lm
TARGET = rosh
SRCS = $(wildcard *.c) $(wildcard mlib/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)
RM = rm -fv
MODE = release
ifeq ($(MODE),debug)
	CFLAGS += -ggdb -DNDEBUG
	CFLAGS := $(filter-out -O3, $(CFLAGS))
endif
ifeq ($(MODE),profile)
	CFLAGS += -pg -fno-inline-functions-called-once -fno-optimize-sibling-calls
endif

.PHONY:all information
all: information $(TARGET) 
information:
ifneq ($(MODE),release)
ifneq ($(MODE),debug)
ifneq ($(MODE),profile)
	@echo "Invalid build mode." 
	@exit 1
endif
endif
endif
	@echo "Building $(TARGET) on "$(MODE)" MODE"
	@echo "............................."

$(TARGET): $(OBJS) Makefile
	@echo linking $(OBJS)
	@$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

-include $(DEPS)
%.o:%.c Makefile
	@echo compiling $<
	@$(CC) -c $(CFLAGS) -o $@ $<

.SILENT:debug profile clean d prof p c
debug d:
	$(MAKE) MODE=debug   --always-make --no-print-directory
profile prof p:
	$(MAKE) MODE=profile --always-make --no-print-directory
clean c:
	$(RM) $(TARGET) $(OBJS) $(DEPS) gmon.out .*.swp .*.swo *~
