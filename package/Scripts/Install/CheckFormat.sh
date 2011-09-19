#!/bin/bash

echo "==============================================="
echo "CheckFormat: Is target HFS or MSDOS?"
echo "**********************************************"

# Receives passed value for the Target Volume Device
# for example: /dev/disk0s2
# if the selected partition is formatted as HFS then exit with 1
# if the selected partition is formatted as MSDOS then exit with 2
# if fstyp doesn't return a value then exit with 0

if [ "$#" -eq 1 ]; then
	targetDevice="$1"
	echo "DEBUG: passed argument for targetDevice = $targetDevice"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

if [ "$( fstyp "$targetDevice" | grep hfs )" ]; then
	echo "${targetDevice} is currently formatted as HFS"
	echo "-----------------------------------------------"
	echo ""
	exit 1

fi
if [ "$( fstyp "$targetDevice" | grep msdos )" ]; then
	echo "${targetDevice} is currently formatted as msdos"
	echo "-----------------------------------------------"
	echo ""
	exit 2
fi 

echo "WARNING: ${targetDevice} is currently not formatted as either HFS or msdos"
echo "-----------------------------------------------"
echo ""

exit 0