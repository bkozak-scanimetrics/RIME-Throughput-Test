################################################################################
#                                    PROJECT                                   #
################################################################################
export SOURCE_PROJECT=tpt_source
export SINK_PROJECT=tpt_sink
################################################################################
#                                     FILES                                    #
################################################################################
MAIN_MAKEFILE=$(CONTIKI)/Makefile.include

export CONTIKI=../../contiki_src/contiki

export SRC_PATH=./src
export INC_PATH=./include

export SRC_FILES = $(notdir $(wildcard $(SRC_PATH)/*.c))

export PROJECTDIRS += $(SRC_PATH) $(INC_PATH)
export PROJECT_SOURCEFILES += $(SRC_FILES)
################################################################################
#                                     FLAGS                                    #
################################################################################
#linker optimizations
export SMALL=1

ifeq ($(PREFIX),)
 export PREFIX = aaaa::1/64
endif

ifeq ($(TARGET),exp5438)
  export CFLAGS += -DEXP5438_DISABLE_UART=1
endif

export CONTIKI_WITH_RIME = 1

export CFLAGS+=-DPROJECT_CONF_H=\"project-conf.h\"
################################################################################
#                                    TARGETS                                   #
################################################################################
all:
	$(MAKE) NODEID=0xAAAA -f $(MAIN_MAKEFILE) $(SOURCE_PROJECT)
	$(MAKE) NODEID=0x0001 -f $(MAIN_MAKEFILE) $(SINK_PROJECT)

hex:
	$(MAKE) NODEID=0xAAAA -f $(MAIN_MAKEFILE) $(SOURCE_PROJECT).hex
	$(MAKE) NODEID=0x0001 -f $(MAIN_MAKEFILE) $(SINK_PROJECT).hex

%.bin:
	$(MAKE) -f $(MAIN_MAKEFILE) $(basename $@)

%:
	$(MAKE) -f $(MAIN_MAKEFILE) $@
