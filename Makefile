GITHUB_DEPS += simplerobot/build-scripts
GITHUB_DEPS += simplerobot/test
GITHUB_DEPS += simplerobot/logger
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
RESOURCE_DIR = $(SOURCE_DIR)/resources

TEST_SOURCE_DIRS = $(MAIN_SOURCE_DIR) $(TEST_SOURCE_DIR) $(PKG_TEST_DIR) $(PKG_LOGGER_DIR) 
TEST_SOURCE_FILES = $(notdir $(wildcard $(TEST_SOURCE_DIRS:%=%/*.cpp) $(TEST_SOURCE_DIRS:%=%/*.c)))
TEST_O_FILES = $(addsuffix .o,$(basename $(TEST_SOURCE_FILES)))

MAIN_SOURCE_DIRS = $(MAIN_SOURCE_DIR) $(PKG_LOGGER_DIR) 
MAIN_SOURCE_FILES = $(notdir $(wildcard $(MAIN_SOURCE_DIRS:%=%/*.cpp) $(MAIN_SOURCE_DIRS:%=%/*.c)))
MAIN_O_FILES = $(addsuffix .o,$(basename $(MAIN_SOURCE_FILES)))

VPATH = $(TEST_SOURCE_DIRS) $(MAIN_SOURCE_DIRS)

TEST_CFLAGS += $(TEST_SOURCE_DIRS:%=-I%)
MAIN_CFLAGS += $(MAIN_SOURCE_DIRS:%=-I%)

.PHONY : default all deps test main release install-service uninstall-service clean

default : all

all : release

test : deps $(TEST_BUILD_DIR)/a.out
	$(TEST_BUILD_DIR)/a.out
	
$(TEST_BUILD_DIR)/a.out : $(TEST_O_FILES:%=$(TEST_BUILD_DIR)/%)
	$(CC) $(TEST_CFLAGS) -o $@ $^

$(TEST_BUILD_DIR)/%.o : %.cpp Makefile | $(TEST_BUILD_DIR)
	$(CC) -c $(TEST_CFLAGS) -o $@ $<

$(TEST_BUILD_DIR)/%.o : %.c Makefile | $(TEST_BUILD_DIR)
	$(CC) -c $(TEST_CFLAGS) -o $@ $<

$(TEST_BUILD_DIR) :
	mkdir -p $@

main : test $(MAIN_BUILD_DIR)/a.out

$(MAIN_BUILD_DIR)/a.out : $(MAIN_O_FILES:%=$(MAIN_BUILD_DIR)/%)
	$(CC) $(MAIN_CFLAGS) -o $@ $^

$(MAIN_BUILD_DIR)/%.o : %.cpp Makefile | $(MAIN_BUILD_DIR)
	$(CC) -c $(MAIN_CFLAGS) -o $@ $<

$(MAIN_BUILD_DIR)/%.o : %.c Makefile | $(MAIN_BUILD_DIR)
	$(CC) -c $(MAIN_CFLAGS) -o $@ $<

$(MAIN_BUILD_DIR) :
	mkdir -p $@

release : main $(RELEASE_BUILD_DIR)/sr-hw-test-agent

$(RELEASE_BUILD_DIR)/sr-hw-test-agent : $(MAIN_BUILD_DIR)/a.out | $(RELEASE_BUILD_DIR)
	cp $< $@
	
$(RELEASE_BUILD_DIR) :
	mkdir -p $@

.PHONY : install-service service-user

install-service :
	if [ ! -f $(RELEASE_BUILD_DIR)/sr-hw-test-agent ] ; then echo "Project not built."; /bin/false; fi
	- systemctl stop sr-hw-test-agent
	cp $(RELEASE_BUILD_DIR)/sr-hw-test-agent /usr/local/bin/sr-hw-test-agent
	id -u sr-hardware-test || useradd -r sr-hardware-test
	cp $(RESOURCE_DIR)/sr-hw-test-agent.service /etc/systemd/system/sr-hw-test-agent.service
	systemctl daemon-reload
	systemctl enable sr-hw-test-agent
	systemctl start sr-hw-test-agent

uninstall-service :
	systemctl stop sr-hw-test-agent
	systemctl disable sr-hw-test-agent
	rm /etc/systemd/system/sr-hw-test-agent.service
	systemctl daemon-reload
	systemctl reset-failed
	rm /usr/local/bin/sr-hw-test-agent

%.h : ;
%.hpp : ;

clean:
	rm -rf $(BUILD_DIR)

-include $(wildcard $(TEST_BUILD_DIR)/*.d)
-include $(wildcard $(MAIN_BUILD_DIR)/*.d)
