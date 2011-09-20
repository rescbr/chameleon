#!/bin/bash

echo "==============================================="
echo "Write Chameleon Stage 0 Loader:"
echo "*******************************"

# Writes Chameleon stage 0 loader.

# Receives disksignature: 0 = Windows not found, 1 = Windows Found
# Receives stage0Loader: Name of file - boot0
# Receives stage0Loaderdualboot: Name of file - boot0hfs
# Receives targetDisk: for example, /dev/disk3
# Receives targetVolume: for example, /Volumes/USB
# Receives scriptDir: The location of the main script dir.


if [ "$#" -eq 6 ]; then
	disksignature="$1"
	stage0Loader="$2"
	stage0Loaderdualboot="$3"
	targetDisk="$4"
	targetVolume="$5"
	scriptDir="$6"
	echo "DEBUG: passed argument for disksignature = $disksignature"
	echo "DEBUG: passed argument for stage0Loader = $stage0Loader"
	echo "DEBUG: passed argument for stage0Loaderdualboot = $stage0Loaderdualboot"
	echo "DEBUG: passed argument for targetDisk = $targetDisk"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi


if [ ${disksignature} = "0" ]; then
	# There’s no Windows disk signature so we can write boot0
		
	echo "Executing command: fdisk440 -u -f /usr/standalone/i386/${stage0Loader} -y ${targetDisk}"
	"${scriptDir}"/Tools/fdisk440 -u -f "${targetVolume}"/usr/standalone/i386/${stage0Loader} -y ${targetDisk}
	"$scriptDir"InstallLog.sh "${targetVolume}" "Written boot0 to ${targetDisk}."
else
	# Windows is also installed on the HDD so we need to write boot0hfs
		
	echo "Executing command: /fdisk440 -u -f /usr/standalone/i386/${stage0Loaderdualboot} -y ${targetDisk}"
	"${scriptDir}"/Tools/fdisk440 -u -f "${targetVolume}"/usr/standalone/i386/${stage0Loaderdualboot} -y ${targetDisk}
	"$scriptDir"InstallLog.sh "${targetVolume}" "Written boot0hfs to ${targetDisk}."
fi


echo "-----------------------------------------------"
echo ""
echo ""

exit 0