# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2010-2014 Intel Corporation

# binary name
APP = iai_mbus_client

# all source are stored in SRCS-y
SRCS-y+=src/interface.c
SRCS-y+=src/setup.c
SRCS-y+=src/backend.c
SRCS-y+=src/main.c

all : ${SRCS-y}

ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

# Default target, can be overridden by command line or environment
RTE_TARGET ?=  x86_64-default-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

CFLAGS += $(WERROR_FLAGS) -I../MBus-Shared/src

# workaround for a gcc bug with noreturn attribute
# http://gcc.gnu.org/bugzilla/show_bug.cgi?id=12603
ifeq ($(CONFIG_RTE_TOOLCHAIN_GCC),y)
CFLAGS_main.o += -Wno-return-type
endif

EXTRA_CFLAGS += -O3 -g # -Wfatal-errors

include $(RTE_SDK)/mk/rte.extapp.mk

