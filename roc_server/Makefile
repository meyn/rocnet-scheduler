CC := g++
CFLAGS := -Wall -std=c++0x
LIBS := sqlite3

PROJ_DIRS := src includes
INCDIR := includes
SBIN_DIR := bin

AUXFILES := Makefile README db
SRCFILES := $(shell find $(PROJ_DIRS) -type f -name "*.cc")
SRCFILES += $(shell find $(PROJ_DIRS) -type f -name "*.c")
HDRFILES := $(shell find $(PROJ_DIRS) -type f -name "*.hh")
HDRFILES += $(shell find $(PROJ_DIRS) -type f -name "*.h")
OBJFILES := $(patsubst %.cc,%.o,$(SRCFILES)) 
DEPFILES := $(patsubst %.cc,%.d,$(SRCFILES))

EXE := roc_server

.PHONY: all clean

%.o: %.cc Makefile
	$(CC) -MMD -MP -c $< -o $@ -I$(INCDIR) $(CFLAGS)

all: $(OBJFILES)
	@mkdir -p $(SBIN_DIR)
	$(CC) -o $(SBIN_DIR)/$(EXE) $(OBJFILES) $(CFLAGS) -l $(LIBS)

clean:
	@echo "Cleaning..."
	@rm -rf $(SBIN_DIR) $(BUILD_DIR) $(OBJFILES) $(DEPFILES) *.o *~ core $(INCDIR)/*~ $(EXE)
