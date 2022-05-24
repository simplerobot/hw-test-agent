GITHUB_DEPS += simplerobot/build-scripts
GITHUB_DEPS += simplerobot/test
include ../build-scripts/build/release/include.make

CC = g++
CFLAGS = -Wall -Werror -pthread -MMD
TEST_CFLAGS = $(CFLAGS) -DTEST -fsanitize=address -static-libasan -g -Og
MAIN_CFLAGS = $(CFLAGS) -O3

BUILD_DIR = build
TEST_BUILD_DIR = $(BUILD_DIR)/test
MAIN_BUILD_DIR = $(BUILD_DIR)/main
RELEASE_BUILD_DIR = $(BUILD_DIR)/release

SOURCE_DIR = source
MAIN_SOURCE_DIR = $(SOURCE_DIR)/main
TEST_SOURCE_DIR = $(SOURCE_DIR)/test

VPATH = $(MAIN_SOURCE_DIR) : $(TEST_SOURCE_DIR) : $(PKG_TEST_DIR)
TEST_SOURCE_DIRS = $(MAIN_SOURCE_DIR) $(TEST_SOURCE_DIR) $(PKG_TEST_DIR)
TEST_SOURCE_FILES = $(notdir $(wildcard $(TEST_SOURCE_DIRS:%=%/*.cpp)))
MAIN_SOURCE_DIRS = $(MAIN_SOURCE_DIR)
MAIN_SOURCE_FILES = $(notdir $(wildcard $(MAIN_SOURCE_DIRS:%=%/*.cpp)))

TEST_CFLAGS += $(TEST_SOURCE_DIRS:%=-I%)
MAIN_CFLAGS += $(MAIN_SOURCE_DIRS:%=-I%)

.PHONY : default deps test main release clean

default : release

test : deps $(TEST_BUILD_DIR)/a.out
	$(TEST_BUILD_DIR)/a.out
	
$(TEST_BUILD_DIR)/a.out : $(TEST_SOURCE_FILES:%.cpp=$(TEST_BUILD_DIR)/%.o)
	$(CC) $(TEST_CFLAGS) -o $@ $^

$(TEST_BUILD_DIR)/%.o : %.cpp Makefile | $(TEST_BUILD_DIR)
	$(CC) -c $(TEST_CFLAGS) -o $@ $<

$(TEST_BUILD_DIR) :
	mkdir -p $@

main : test $(MAIN_BUILD_DIR)/a.out

$(MAIN_BUILD_DIR)/a.out : $(MAIN_SOURCE_FILES:%.cpp=$(MAIN_BUILD_DIR)/%.o)
	$(CC) $(MAIN_CFLAGS) -o $@ $^

$(MAIN_BUILD_DIR)/%.o : %.cpp Makefile | $(MAIN_BUILD_DIR)
	$(CC) -c $(MAIN_CFLAGS) -o $@ $<

$(MAIN_BUILD_DIR) :
	mkdir -p $@

release : main $(RELEASE_BUILD_DIR)/sr-hw-test-agent

$(RELEASE_BUILD_DIR)/sr-hw-test-agent : $(MAIN_BUILD_DIR)/a.out | $(RELEASE_BUILD_DIR)
	cp $< $@
	
$(RELEASE_BUILD_DIR) :
	mkdir -p $@




%.h : ;
%.hpp : ;

clean:
	rm -rf $(BUILD_DIR)

-include $(wildcard $(TEST_BUILD_DIR)/*.d)
-include $(wildcard $(MAIN_BUILD_DIR)/*.d)
