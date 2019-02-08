#=============================================================================#
# Unity Native Testing Makefile
#
# author: Charlie Vorbach
# last change: 2019-01-08
#
# This makefile is based off of examples at www.throwtheswitch.org/unity/
# It trys to compile only the test files and their dependent source files
# This minimizes the amount of finagling with HAL libraries during unit tests
#=============================================================================#

CLEANUP = rm -f
MKDIR = mkdir -p
TARGET_EXTENSION=out

.PHONY: clean
.PHONY: test

# A bunch of paths, including for each step of the build process
PATHU = ../../lib/unity/src/
PATHS = src/
PATHI = inc/
PATHT = test/

# This is wherever you stick your emulation of HAL stuff
PATHH = ../../lib/stm32f4-hal-emulaton/
PATHB = build/
PATHD = build/depends/
PATHO = build/objs/
PATHR = build/results/
PATHL = libs/
PATHLB = libs/build/

# We'll create these directories if they're missing
BUILD_PATHS = $(PATHB) $(PATHD) $(PATHO) $(PATHR)

# These are our tests' source files
SRCT = $(wildcard $(PATHT)*.c)

# These are the source files for the hal emulation
SRCH = $(wildcard $(PATHH)*.c)
HAL_EMU = $(patsubst $(PATHH)%.c, $(PATHO)%.o, $(SRCH))

WARNING= #-Wall #-Wextra #-Werror could be added here

# Command for each step of build process
COMPILE=gcc -c
LINK=gcc
DEPEND=gcc -MM -MG -MF
CFLAGS= $(WARNING) -I. -I$(PATHU) -I$(PATHI) -I$(PATHH) -DTEST

# This monstrosity looks for source dependencies of our test files using their included headers
GET_HEADERS=gcc -MM -MG
GET_SOURCE_DEP      = $(foreach file, $(subst .h,.c, $(filter %.h, $(shell $(GET_HEADERS) $(1)))), $(filter $(PATHS)$(file), $(wildcard $(PATHS)*.c)))
SOURCE_TO_OBJ       = $(patsubst $(PATHS)%.c, $(PATHO)%.o, $(1))
SOURCE_DEPENDENCIES = $(call SOURCE_TO_OBJ, $(foreach test_file, $(wildcard $(PATHT)*.c), $(call GET_SOURCE_DEP, $(test_file))))

# We'll sumarized the results in a text file for each test source file
RESULTS = $(patsubst $(PATHT)test_%.c,$(PATHR)test_%.txt,$(SRCT) )

PASSED = `grep -s PASS $(PATHR)*.txt`
FAIL = `grep -s FAIL $(PATHR)*.txt`
IGNORE = `grep -s IGNORE $(PATHR)*.txt`

#-include $(PATHD)*.d

# We group and print the results of all our tests here
test: $(BUILD_PATHS) $(RESULTS)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

# Results depend on test output files
$(PATHR)%.txt: $(PATHB)%.$(TARGET_EXTENSION)
	-./$< > $@ 2>&1

# Link everything together
$(PATHB)test_%.$(TARGET_EXTENSION): $(PATHO)test_%.o $(PATHO)%.o $(PATHO)unity.o $(PATHD)%.d $(HAL_EMU) $(SOURCE_DEPENDENCIES) 
	$(LINK) -o $@ $(patsubst %.d,,$^)

# Compile test source files
$(PATHO)test_%.o:: $(PATHT)test_%.c
	$(COMPILE) $(CFLAGS) $< -o $@

# Compile project's source files
$(PATHO)%.o:: $(PATHS)%.c
	$(COMPILE) $(CFLAGS) $< -o $@

# Compile hal emulation files
$(PATHO)%.o:: $(PATHH)%.c 
	$(COMPILE) $(CFLAGS) $< -o $@

# Compile unity
$(PATHO)%.o:: $(PATHU)%.c $(PATHU)%.h
	$(COMPILE) $(CFLAGS) $< -o $@

# Generate dependencies
$(PATHD)%.d:: $(PATHT)%.c
	$(DEPEND) $@ $<

$(PATHD)%.d:: $(PATHS)%.c
	$(DEPEND) $@ $<

# Make any missing directories
$(PATHB):
	$(MKDIR) $(PATHB)

$(PATHD):
	$(MKDIR) $(PATHD)

$(PATHO):
	$(MKDIR) $(PATHO)

$(PATHR):
	$(MKDIR) $(PATHR)

$(PATHH):
	$(MKDIR) $(PATHH)

# Remove everything
clean:
	$(CLEANUP) $(PATHO)*.o
	$(CLEANUP) $(PATHB)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATHR)*.txt
	$(CLEANUP) $(PATHH)*.o

# Keeps intermediate files
.PRECIOUS: $(PATHB)test_%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATHD)%.d
.PRECIOUS: $(PATHO)%.o
.PRECIOUS: $(PATHR)%.txt
