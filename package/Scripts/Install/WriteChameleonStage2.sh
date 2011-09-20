#!/bin/bash

echo "==============================================="
echo "Write Chameleon Stage 2 Loader:"
echo "*******************************"

# Writes Chameleon stage 2 loader.

# Receives espformat: 1 for HFS, 2 for MSDOS, 0 for unknown
# Receives stage2Loader: Name of file - boot
# Receives selectedDestination: for example, /Volumes/USB
# Receives targetDevice: for example, /dev/disk3s1
# Receives targetVolume: for example, /Volumes/USB
# Receives scriptDir: The location of the main script dir.


if [ "$#" -eq 6 ]; then
	espformat="$1"
	stage2Loader="$2"
	selectedDestination="$3"
	targetDevice="$4"
	targetVolume="$5"
	scriptDir="$6"
	echo "DEBUG: passed argument for espformat = $espformat"
	echo "DEBUG: passed argument for stage2Loader = $stage2Loader"
	echo "DEBUG: passed argument for selectedDestination = $selectedDestination"
	echo "DEBUG: passed argument for targetDevice = $targetDevice"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

# check to see if install to EFI system partition was selected
# if chosen, the package installer will add a file named 'nullESP'
# in to the temporary directory /.Chameleon

if [ "${targetVolume}" = "/Volumes/EFI" ]; then
	echo "DEBUG: EFI install chosen"

	if [ ! -d "${targetVolume}" ]; then
		echo "Executing Command: mkdir -p ${targetVolume}"
		mkdir -p "${targetVolume}"
	else
		echo "DEBUG: folder /Volumes/EFI already exists"
	fi

	#if the EFI system partition was selected then
	# mount '/Volumes/EFI' with the correct format type

	if [ ${espformat} = 1 ]; then

		echo "Executing command: mount_hfs ${targetDevice} ${targetVolume}"
		mount_hfs "${targetDevice}" "${targetVolume}"
	fi
	if [ ${espformat} = 2 ]; then
		[ -d "${targetVolume}" ] || mkdir -p "${targetVolume}"
		echo "Executing command: mount_msdos -u 0 -g 0 ${targetDevice} ${targetVolume}"
		mount_msdos -u 0 -g 0 "${targetDevice}" "${targetVolume}"
	fi

	echo "Executing command: cp "${selectedDestination}"/usr/standalone/i386/${stage2Loader} ${targetVolume}"
	cp "${selectedDestination}"/usr/standalone/i386/"${stage2Loader}" "${targetVolume}"
else
	echo "Executing command: cp "${targetVolume}"/usr/standalone/i386/${stage2Loader} ${targetVolume}"
	cp "${targetVolume}"/usr/standalone/i386/"${stage2Loader}" "${targetVolume}"
fi

"$scriptDir"InstallLog.sh "${targetVolume}" "Written boot to ${targetVolume}."

#ÊCheck to see if the user wants to hide the boot file

#if [ -f "${selectedDestination}"/.Chameleon/nullhideboot ]; then
#
#	echo "Executing command: SetFile -a V ${targetVolume}/${stage2Loader}"
#	"${selectedDestination}"/.Chameleon//Tools/SetFile -a V "${targetVolume}"/"${stage2Loader}"
#fi

echo "-----------------------------------------------"
echo ""
echo ""

exit 0