#!/bin/bash

echo "==============================================="
echo "Unmount all volumes named EFI"
echo "*****************************"

# loop through and un-mount ALL mounted 'EFI' system partitions - Thanks kizwan

# Receives targetVolumeChosenByUser: Stores original target if EFI install selected.
# Receives scriptDir: The location of the main script dir.

if [ "$#" -eq 2 ]; then
	targetVolumeChosenByUser="$1"
	scriptDir="$2"
	echo "DEBUG: passed argument for targetVolumeChosenByUser = $targetVolumeChosenByUser"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

# Count of 5 exists incase for some reason /Volumes/EFI fails
# be unmounted in which case the loop would run forever.
attempts=1
while [ "$( df | grep EFI )" ] && [ $attempts -lt 5 ]; do
	#echo "DEBUG: Unmounting $( df | grep EFI | awk '{print $1}' )"
	"$scriptDir"InstallLog.sh "${targetVolumeChosenByUser}" "Find and unmount any volume named 'EFI':"
	"$scriptDir"InstallLog.sh "${targetVolumeChosenByUser}" "$( df | grep EFI | awk '{print $1}' )"
	umount -f $( df | grep EFI | awk '{print $1}' )
	(( attempts++ ))
done
if [ $attempts = 5 ]; then
	#echo "DEBUG: failed to unmount 'EFI' System Partition."
	"$scriptDir"InstallLog.sh "${targetVolumeChosenByUser}" "Failed to unmount 'EFI' System Partition."
	exit 1
fi

exit 0



