#
# Copyright (C) 2007-2011 B.A.T.M.A.N. contributors:
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
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA
#

# changing the CONFIG_* line to 'y' enables the related feature
# B.A.T.M.A.N. debugging:
export CONFIG_BATMAN_ADV_DEBUG=n

PWD:=$(shell pwd)
KERNELPATH ?= /lib/modules/$(shell uname -r)/build
# sanity check: does KERNELPATH exist?
ifeq ($(shell cd $(KERNELPATH) && pwd),)
$(warning $(KERNELPATH) is missing, please set KERNELPATH)
endif

export KERNELPATH
RM ?= rm -f

REVISION= $(shell	if [ -d .git ]; then \
				echo $$(git describe --always --dirty --match "v*" |sed 's/^v//' 2> /dev/null || echo "[unknown]"); \
			fi)

CONFIG_BATMAN_ADV=m
batman-adv-y += compat.o
ifneq ($(REVISION),)
ccflags-y += -DSOURCE_VERSION=\"$(REVISION)\"
endif
include $(PWD)/Makefile.kbuild

all: config
	$(MAKE) -C $(KERNELPATH) M=$(PWD) PWD=$(PWD) modules

clean:
	$(RM) compat-autoconf.h*
	$(MAKE) -C $(KERNELPATH) M=$(PWD) PWD=$(PWD) clean

install: config
	$(MAKE) -C $(KERNELPATH) M=$(PWD) PWD=$(PWD) INSTALL_MOD_DIR=kernel/net/batman-adv/ modules_install

config:
	$(PWD)/gen-compat-autoconf.sh $(PWD)/compat-autoconf.h

.PHONY: all clean install config
