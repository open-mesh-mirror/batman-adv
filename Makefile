#
# Copyright (C) 2007-2016  B.A.T.M.A.N. contributors:
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
export CONFIG_BATMAN_ADV_DEBUGFS=y
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
# B.A.T.M.A.N. V routing algorithm (experimental):
export CONFIG_BATMAN_ADV_BATMAN_V=n

PWD:=$(shell pwd)
BUILD_DIR=$(PWD)/build
KERNELPATH ?= /lib/modules/$(shell uname -r)/build
# sanity check: does KERNELPATH exist?
ifeq ($(shell cd $(KERNELPATH) && pwd),)
$(warning $(KERNELPATH) is missing, please set KERNELPATH)
endif

ifeq ($(origin SPATCH), undefined)
  SPATCH = spatch
  ifeq ($(shell which $(SPATCH) 2>/dev/null),)
    $(error $(SPATCH) (coccinelle) not found)
  endif
endif

export KERNELPATH
RM ?= rm -f
MKDIR := mkdir -p
PATCH_FLAGS = --batch --fuzz=0 --forward --strip=1 --unified --version-control=never -g0 --remove-empty-files --no-backup-if-mismatch --reject-file=-
PATCH := patch $(PATCH_FLAGS) -i
SPATCH_FLAGS := --in-place --relax-include-path --use-coccigrep --very-quiet
CP := cp -fpR
LN := ln -sf

SOURCE = $(wildcard net/batman-adv/*.[ch]) net/batman-adv/Makefile
SOURCE_BUILD = $(wildcard $(BUILD_DIR)/net/batman-adv/*.[ch]) $(BUILD_DIR)/net/batman-adv/Makefile
SOURCE_STAMP = $(BUILD_DIR)/net/batman-adv/.compat-prepared

REVISION= $(shell	if [ -d "$(PWD)/.git" ]; then \
				echo $$(git --git-dir="$(PWD)/.git" describe --always --dirty --match "v*" |sed 's/^v//' 2> /dev/null || echo "[unknown]"); \
			fi)
NOSTDINC_FLAGS += \
	-I$(PWD)/../compat-include/ \
	-I$(PWD)/../include/ \
	-include $(PWD)/../compat.h \
	$(CFLAGS)

ifneq ($(REVISION),)
NOSTDINC_FLAGS += -DBATADV_SOURCE_VERSION=\"$(REVISION)\"
endif

-include $(PWD)/../compat-sources/Makefile

obj-y += net/batman-adv/

export batman-adv-y


BUILD_FLAGS := \
	M=$(BUILD_DIR) \
	PWD=$(BUILD_DIR) \
	REVISION=$(REVISION) \
	CONFIG_BATMAN_ADV=m \
	CONFIG_BATMAN_ADV_DEBUG=$(CONFIG_BATMAN_ADV_DEBUG) \
	CONFIG_BATMAN_ADV_DEBUGFS=$(CONFIG_BATMAN_ADV_DEBUGFS) \
	CONFIG_BATMAN_ADV_BLA=$(CONFIG_BATMAN_ADV_BLA) \
	CONFIG_BATMAN_ADV_DAT=$(CONFIG_BATMAN_ADV_DAT) \
	CONFIG_BATMAN_ADV_NC=$(CONFIG_BATMAN_ADV_NC) \
	CONFIG_BATMAN_ADV_MCAST=$(CONFIG_BATMAN_ADV_MCAST) \
	CONFIG_BATMAN_ADV_BATMAN_V=$(CONFIG_BATMAN_ADV_BATMAN_V) \
	INSTALL_MOD_DIR=updates/

all: config $(SOURCE_STAMP)
	$(MAKE) -C $(KERNELPATH) $(BUILD_FLAGS)	modules

clean:
	$(RM) compat-autoconf.h*
	$(RM) -r $(BUILD_DIR)

install: config $(SOURCE_STAMP)
	$(MAKE) -C $(KERNELPATH) $(BUILD_FLAGS) modules_install
	depmod -a

config:
	$(PWD)/gen-compat-autoconf.sh $(PWD)/compat-autoconf.h

$(SOURCE_STAMP): $(SOURCE) compat-patches/*
	$(MKDIR) $(BUILD_DIR)/net/batman-adv/
	@$(LN) ../Makefile $(BUILD_DIR)/Makefile
	@$(RM) $(SOURCE_BUILD)
	@$(CP) $(SOURCE) $(BUILD_DIR)/net/batman-adv/
	@set -e; \
	patches="$$(ls -1 compat-patches/|grep -e '.patch$$' -e '.cocci$$'|sort)"; \
	for i in $${patches}; do \
		echo '  COMPAT_PATCH '$${i}; \
		if echo $${i}|grep '.patch$$'; then \
			cd $(BUILD_DIR); \
			$(PATCH) ../compat-patches/$${i}; \
			cd - > /dev/null; \
		fi; \
		if echo $${i}|grep '.cocci$$'; then echo $$(pwd); \
			$(SPATCH) $(SPATCH_FLAGS) --dir $(BUILD_DIR) --sp-file compat-patches/$${i} > /dev/null; \
		fi; \
	done
	touch $(SOURCE_STAMP)

.PHONY: all clean install config
