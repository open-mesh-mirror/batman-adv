#! /bin/sh
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

set -e

TARGET=${1:="compat-autoconf.h"}
TMP="${TARGET}.tmp"

echo > "${TMP}"

gen_config() {
	KEY="${1}"
	VALUE="${2}"

	echo "#undef ${KEY}"
	echo "#undef __enabled_${KEY}"
	echo "#undef __enabled_${KEY}_MODULE"
	case "${VALUE}" in
	y)
		echo "#define ${KEY} 1"
		echo "#define __enabled_${KEY} 1"
		echo "#define __enabled_${KEY}_MODULE 0"
		;;
	m)
		echo "#define ${KEY} 1"
		echo "#define __enabled_${KEY} 0"
		echo "#define __enabled_${KEY}_MODULE 1"
		;;
	n)
		echo "#define __enabled_${KEY} 0"
		echo "#define __enabled_${KEY}_MODULE 0"
		;;
	*)
		echo "#define ${KEY} \"${VALUE}\""
		;;
	esac
}

# write config variables
gen_config 'CONFIG_BATMAN_ADV_DEBUGFS' ${CONFIG_BATMAN_ADV_DEBUGFS:="n"} >> "${TMP}"
gen_config 'CONFIG_BATMAN_ADV_DEBUG' ${CONFIG_BATMAN_ADV_DEBUG:="n"} >> "${TMP}"
gen_config 'CONFIG_BATMAN_ADV_BLA' ${CONFIG_BATMAN_ADV_BLA:="y"} >> "${TMP}"
gen_config 'CONFIG_BATMAN_ADV_DAT' ${CONFIG_BATMAN_ADV_DAT:="y"} >> "${TMP}"
gen_config 'CONFIG_BATMAN_ADV_MCAST' ${CONFIG_BATMAN_ADV_MCAST:="y"} >> "${TMP}"
gen_config 'CONFIG_BATMAN_ADV_NC' ${CONFIG_BATMAN_ADV_NC:="n"} >> "${TMP}"
gen_config 'CONFIG_BATMAN_ADV_TRACING' ${CONFIG_BATMAN_ADV_TRACING:="n"} >> "${TMP}"
gen_config 'CONFIG_BATMAN_ADV_BATMAN_V' ${CONFIG_BATMAN_ADV_BATMAN_V:="y"} >> "${TMP}"

# only regenerate compat-autoconf.h when config was changed
diff "${TMP}" "${TARGET}" > /dev/null 2>&1 || cp "${TMP}" "${TARGET}"
