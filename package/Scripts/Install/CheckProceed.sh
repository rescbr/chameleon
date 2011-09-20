#!/bin/bash

echo "==============================================="
echo "Check Proceed: Can the script continue?"
echo "***************************************"

# Checks the selected volume is present and the disk
# is partitioned.

# Receives targetVolumeTemp: Stores original target if EFI install selected.
# Receives targetVolume: Stores '/Volumes/EFI' if EFI install, or blank if not. 
# Receives targetDevice: Stores device number: for example /dev/disk2s1.
# Receives scriptDir: The location of the main script dir.

if [ "$#" -eq 4 ]; then
	targetVolumeTemp="$1"
	targetVolume="$2"
	targetDevice="$3"
	scriptDir="$4"
	echo "DEBUG: passed argument for targetVolumeTemp = $targetVolumeTemp"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for targetDevice = $targetDevice"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

if [ -z "$targetVolume" ]; then
	echo "*** Cannot find the volume. Exiting."
	"$scriptDir"InstallLog.sh "${targetVolumeTemp}" "Cannot file the volume."
	exit 1
else
	echo "Confirming target volume exists"
fi

if [ "$targetDevice" = "$targetDevice#*disk*s" ]; then
	echo "*** ERROR Volume does not use slices. Exiting."
	"$scriptDir"InstallLog.sh "${targetVolumeTemp}" "The selected volume doesn't use slices."
	exit 1		
else
	echo "Confirming target device uses slices"
fi

# Add check for installing to a 'small' HFS device like a
# 1GB USB flash drive which won't have an EFI System Partition.

if [ "$targetVolume" = "/Volumes/EFI" ]; then
	existESP=$( df | grep /dev/disk2s1 | awk {'print $6'} )
	if [ ! "$existESP" = "/Volumes/EFI" ]; then
		echo "*** The selected volume doesn't have an EFI System Partition. Exiting."
		"$scriptDir"InstallLog.sh "${targetVolumeTemp}" "The selected volume doesn't have an EFI System Partition."
		exit 1
	fi
fi

echo "-----------------------------------------------"
echo ""

exit 0