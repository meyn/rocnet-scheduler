CC := gcc
LD := ld

CFLAGS := -Wall
LDFLAGS := -static

PROJ_DIRS := includes src
OBJDIR := build

EXE := roc_agent
PKG_NAME := roc_agent.tgz
SBIN_DIR := sbin
BUILD_DIR := build

AUXFILES := Makefile README INSTALL etc rc.d
SRCFILES := $(shell find $(PROJ_DIRS) -type f -name "*.c")
HDRFILES := $(shell find $(PROJ_DIRS) -type f -name "*.h")

OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
DEPFILES := $(patsubst %.c,%.d,$(SRCFILES))

ALLFILES := $(SRCFILES) $(HDRFILES) $(AUXFILES)

.PHONY: all clean install dist pkg old

%.o: %.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@ -Iincludes

all: $(OBJFILES)
	@mkdir -p $(SBIN_DIR)
	$(CC) -o $(SBIN_DIR)/$(EXE) $(OBJFILES) $(CFLAGS) $(LDFLAGS)

bipkg: all
	@mkdir -p opt/rocnet
	@cp -r README $(SBIN_DIR) etc/ rc.d/ opt/rocnet/
	@tar -czf $(PKG_NAME) opt/rocnet/
	@rm -r opt/

clean:
	@echo "Cleaning..."
	@rm -rf $(SBIN_DIR) $(BUILD_DIR) $(OBJFILES) $(DEPFILES) *~ core $(PKG_NAME)
