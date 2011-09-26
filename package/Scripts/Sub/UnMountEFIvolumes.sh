#!/bin/bash

echo "==============================================="
echo "Unmount all volumes named EFI"
echo "*****************************"

# loop through and un-mount ALL mounted 'EFI' system partitions - Thanks kizwan

# Receives scriptDir: The location of the main script dir.
# Receives targetVolumeTemp: Stores original target if EFI install selected.

if [ "$#" -eq 2 ]; then
	targetVolumeTemp="$1"
	scriptDir="$2"
	echo "DEBUG: passed argument for targetVolumeTemp = $targetVolumeTemp"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi


attempts=1
while [ "$( df | grep EFI )" ] && [ "${attempts}" -lt 5 ]; do
	echo "Unmounting $( df | grep EFI | awk '{print $1}' )"
	umount -f $( df | grep EFI | awk '{print $1}' )
	attempts=$(( ${attempts} + 1 ))
done
if [ ${attempts} = 5 ]; then
	echo "failed to unmount 'EFI' System Partition."
	echo "-----------------------------------------------"
	"$scriptDir"InstallLog.sh "${targetVolumeTemp}" "Failed to unmount 'EFI' System Partition."
	echo ""
	echo ""
	echo ""
	exit 1
fi

echo "-----------------------------------------------"
echo ""
echo ""
echo ""

exit 0



