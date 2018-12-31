# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2007-2019  B.A.T.M.A.N. contributors:
#
# Marek Lindner, Simon Wunderlich
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of version 2 of the GNU General Public
# License as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#

# read README.external for more information about the configuration
# batman-adv DebugFS entries:
export CONFIG_BATMAN_ADV_DEBUGFS=n
# B.A.T.M.A.N. debugging:
export CONFIG_BATMAN_ADV_DEBUG=n
# B.A.T.M.A.N. bridge loop avoidance:
export CONFIG_BATMAN_ADV_BLA=y
# B.A.T.M.A.N. distributed ARP table:
export CONFIG_BATMAN_ADV_DAT=y
# B.A.T.M.A.N network coding (catwoman):
export CONFIG_BATMAN_ADV_NC=n
# B.A.T.M.A.N. multicast optimizations:
export CONFIG_BATMAN_ADV_MCAST=y
# B.A.T.M.A.N. tracing support:
export CONFIG_BATMAN_ADV_TRACING=n
# B.A.T.M.A.N. V routing algorithm (experimental):
export CONFIG_BATMAN_ADV_BATMAN_V=y

PWD:=$(shell pwd)
KERNELPATH ?= /lib/modules/$(shell uname -r)/build
# sanity check: does KERNELPATH exist?
ifeq ($(shell cd $(KERNELPATH) && pwd),)
$(warning $(KERNELPATH) is missing, please set KERNELPATH)
endif

export KERNELPATH
RM ?= rm -f
CP := cp -fpR
LN := ln -sf
DEPMOD := depmod -a

REVISION= $(shell	if [ -d "$(PWD)/.git" ]; then \
				echo $$(git --git-dir="$(PWD)/.git" describe --always --dirty --match "v*" |sed 's/^v//' 2> /dev/null || echo "[unknown]"); \
			fi)
NOSTDINC_FLAGS += \
	-I$(PWD)/compat-include/ \
	-I$(PWD)/include/ \
	-include $(PWD)/compat.h \
	$(CFLAGS)

ifneq ($(REVISION),)
NOSTDINC_FLAGS += -DBATADV_SOURCE_VERSION=\"$(REVISION)\"
endif

include $(PWD)/compat-sources/Makefile

obj-y += net/batman-adv/

export batman-adv-y


BUILD_FLAGS := \
	M=$(PWD) \
	PWD=$(PWD) \
	REVISION=$(REVISION) \
	CONFIG_BATMAN_ADV=m \
	CONFIG_BATMAN_ADV_DEBUG=$(CONFIG_BATMAN_ADV_DEBUG) \
	CONFIG_BATMAN_ADV_DEBUGFS=$(CONFIG_BATMAN_ADV_DEBUGFS) \
	CONFIG_BATMAN_ADV_BLA=$(CONFIG_BATMAN_ADV_BLA) \
	CONFIG_BATMAN_ADV_DAT=$(CONFIG_BATMAN_ADV_DAT) \
	CONFIG_BATMAN_ADV_NC=$(CONFIG_BATMAN_ADV_NC) \
	CONFIG_BATMAN_ADV_MCAST=$(CONFIG_BATMAN_ADV_MCAST) \
	CONFIG_BATMAN_ADV_TRACING=$(CONFIG_BATMAN_ADV_TRACING) \
	CONFIG_BATMAN_ADV_BATMAN_V=$(CONFIG_BATMAN_ADV_BATMAN_V) \
	INSTALL_MOD_DIR=updates/

all: config
	$(MAKE) -C $(KERNELPATH) $(BUILD_FLAGS)	modules

clean:
	$(RM) compat-autoconf.h*
	$(MAKE) -C $(KERNELPATH) $(BUILD_FLAGS) clean

install: config
	$(MAKE) -C $(KERNELPATH) $(BUILD_FLAGS) modules_install
	$(DEPMOD)

config:
	$(PWD)/gen-compat-autoconf.sh $(PWD)/compat-autoconf.h

.PHONY: all clean install config
