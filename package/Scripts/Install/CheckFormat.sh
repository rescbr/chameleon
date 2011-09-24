#!/bin/bash

echo "==============================================="
echo "CheckFormat: Is target HFS or MSDOS?"
echo "**********************************************"

# if the selected partition is formatted as HFS then exit with 1
# if the selected partition is formatted as MSDOS then exit with 2
# if fstyp doesn't return a value then exit with 0

# Receives targetDevice: for example, /dev/disk0s2
# Receives targetVolume: Volume to install to.
# Receives scriptDir: The location of the main script dir.


if [ "$#" -eq 3 ]; then
	targetDevice="$1"
	targetVolume="$2"
	scriptDir="$3"
	echo "DEBUG: passed argument for targetDevice = $targetDevice"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

if [ "$( fstyp "$targetDevice" | grep hfs )" ]; then
	echo "${targetDevice} is currently formatted as HFS"
	echo "-----------------------------------------------"
	echo ""
	#"$scriptDir"InstallLog.sh "${targetVolume}" "${targetDevice} is currently formatted as HFS"
	exit 1

fi
if [ "$( fstyp "$targetDevice" | grep msdos )" ]; then
	echo "${targetDevice} is currently formatted as msdos"
	echo "-----------------------------------------------"
	echo ""
	#"$scriptDir"InstallLog.sh "${targetVolume}" "${targetDevice} is currently formatted as msdos"
	exit 2
fi 

echo "WARNING: ${targetDevice} is currently not formatted as either HFS or msdos"
echo "-----------------------------------------------"
echo ""

"$scriptDir"InstallLog.sh "${targetVolume}" "WARNING: ${targetDevice} is currently not formatted as either HFS or msdos"

exit 0