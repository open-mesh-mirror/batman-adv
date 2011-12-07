#! /bin/sh

set -e

TARGET=${1:="compat-autoconf.h"}
TMP="${TARGET}.tmp"

echo -n > "${TMP}"

gen_config() {
	KEY="${1}"
	VALUE="${2}"

	echo "#undef ${KEY}"
	case "${VALUE}" in
	y)
		echo "#define ${KEY} 1"
		;;
	m)
		echo "#define ${KEY} 1"
		;;
	n)
		# leave it undefined
		;;
	*)
		echo "#define ${KEY} \"${VALUE}\""
		;;
	esac
}

# write config variables
gen_config 'CONFIG_BATMAN_ADV_DEBUG' ${CONFIG_BATMAN_ADV_DEBUG:="n"} >> "${TMP}"

# only regenerate compat-autoconf.h when config was changed
diff "${TMP}" "${TARGET}" > /dev/null 2>&1 || cp "${TMP}" "${TARGET}"
