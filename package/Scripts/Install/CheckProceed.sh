#!/bin/bash

echo "==============================================="
echo "Check Proceed: Can the script continue?"
echo "***************************************"

# Receives targetVolume and targetDevice
# Checks the selected volume is present and the disk
# is partitioned.

if [ "$#" -eq 2 ]; then
	targetVolume="$1"
	targetDevice="$2"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for targetDevice = $targetDevice"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

if [ -z "$targetVolume" ]; then
	echo "Cannot find the volume. Exiting."
	exit 1
else
	echo "Confirming target volume exists"
fi

if [ "$targetDevice" = "$targetDevice#*disk*s" ]; then
	echo "ERROR Volume does not use slices."
	exit 1		
else
	echo "Confirming target device uses slices"
fi

echo "-----------------------------------------------"
echo ""

exit 0