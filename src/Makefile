#   BSD LICENSE

# binary name
APP = mao-dpdk-framework

SRCDIR := $(shell pwd)

# Mao: useless
#VPATH += $(SRCDIR)/
#VPATH += $(SRCDIR)/SegmentRouting
#VPATH += $(SRCDIR)/IpSrTunnel
#VPATH += $(SRCDIR)/RouterParam
#subdirs := SegmentRouting IpSrTunnel RouterParam symmetric_mp


maodir := -I$(SRCDIR)/
#maodir += -I$(SRCDIR)/SegmentRouting
#maodir += -I$(SRCDIR)/IpSrTunnel
#maodir += -I$(SRCDIR)/RouterParam
#maodir += -I$(SRCDIR)/PacketInspector




# all source are stored in SRCS-y
# Mao: may just need main.c
SRCS-y := main.c
#SRCS-y += IpSrTunnel/MaoIpRouteTableLPM.c
#SRCS-y += SegmentRouting/mplsForwardTable.c
#SRCS-y += PacketInspector/packetInspector.c




# Build using pkg-config variables if possible
ifeq ($(shell pkg-config --exists libdpdk && echo 0),0)

all: static # shared
.PHONY: shared static
shared: build/$(APP)-shared
	ln -sf $(APP)-shared build/$(APP)
static: build/$(APP)-static
	ln -sf $(APP)-static build/$(APP)

PKGCONF ?= pkg-config

PC_FILE := $(shell $(PKGCONF) --path libdpdk 2>/dev/null)
CFLAGS += -O3 $(shell $(PKGCONF) --cflags libdpdk) $(maodir)
# Add flag to allow experimental API as l2fwd uses rte_ethdev_set_ptype API
CFLAGS += -DALLOW_EXPERIMENTAL_API
LDFLAGS_SHARED = $(shell $(PKGCONF) --libs libdpdk)
LDFLAGS_STATIC = $(shell $(PKGCONF) --static --libs libdpdk)

build/$(APP)-shared: $(SRCS-y) Makefile $(PC_FILE) | build
	$(CC) $(CFLAGS) $(SRCS-y) -o $@ $(LDFLAGS) $(LDFLAGS_SHARED)

build/$(APP)-static: $(SRCS-y) Makefile $(PC_FILE) | build
	$(CC) $(CFLAGS) $(SRCS-y) -o $@ $(LDFLAGS) $(LDFLAGS_STATIC)

build:
	@mkdir -p $@

.PHONY: clean
clean:
	rm -f build/$(APP) build/$(APP)-static build/$(APP)-shared
	test -d build && rmdir -p build || true

maodebug:
	ls $(SRCDIR) vpathaaa $(VPATH) asdgasdg $(shell $(PKGCONF) --path libdpdk 2>/dev/null)

else # Build using legacy build system

#ifeq ($(RTE_SDK),)
#$(error "Please define RTE_SDK environment variable")
#endif
##RTE_SDK ?= /home/mao/dpdk/dpdk-stable-19.11.5/myinstall/share/dpdk
#
#
## Default target, detect a build directory, by looking for a path with a .config
#RTE_TARGET ?= $(notdir $(abspath $(dir $(firstword $(wildcard $(RTE_SDK)/*/.config)))))
## Default target, can be overridden by command line or environment
##RTE_TARGET ?= x86_64-native-linux-gcc
#
#
#include $(RTE_SDK)/mk/rte.vars.mk
#
#CFLAGS += -O3
#CFLAGS += $(WERROR_FLAGS)
## Add flag to allow experimental API as l2fwd uses rte_ethdev_set_ptype API
#CFLAGS += -DALLOW_EXPERIMENTAL_API
#
#include $(RTE_SDK)/mk/rte.extapp.mk

endif
