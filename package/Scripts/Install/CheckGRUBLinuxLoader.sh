#!/bin/bash

echo "==============================================="
echo "CheckGRUBLinuxLoader: Does GRUB or LILO exist?"
echo "**********************************************"

# This reads the MBR of the disk in the attempt to find the
# signature for either the GRUB or Linux bootloaders.
# The script returns 1 if either is found, or 0 if none found.

# Receives targetdisk: Stores device number: for example /dev/disk2
# Receives targetVolumeTemp: Stores original target if EFI install selected.
# Receives scriptDir: The location of the main script dir.

if [ "$#" -eq 3 ]; then
	targetDisk="$1"
	targetVolumeTemp="$2"
	scriptDir="$3"
	echo "DEBUG: passed argument for targetDisk = $targetDisk"
	echo "DEBUG: passed argument for targetVolumeTemp = $targetVolumeTemp"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

diskmicrocodetype[1]="GRUB,47525542"
diskmicrocodetype[2]="LILO,4c494c4f"


diskmicrocode=$( dd 2>/dev/null if="$targetDisk" count=1 | dd 2>/dev/null count=1 bs=437 | perl -ne '@a=split"";for(@a){printf"%02x",ord}' )
#echo "${diskmicrocode}"
diskmicrocodetypecounter=0
while [ ${diskmicrocodetypecounter} -lt ${#diskmicrocodetype[@]} ]; do
        diskmicrocodetypecounter=$(( ${diskmicrocodetypecounter} + 1 ))
        diskmicrocodetypeid=${diskmicrocodetype[${diskmicrocodetypecounter}]#*,}
        if [ ! "${diskmicrocode}" = "${diskmicrocode/${diskmicrocodetypeid}/}" ]; then
                echo "${diskmicrocodetype[${diskmicrocodetypecounter}]%,*} found."
		"$scriptDir"InstallLog.sh "${targetVolumeTemp}" "Found an exisitng GRUB/LILO bootloader."
		exit 1
	else
		echo "Didn't find a match for ${diskmicrocodetype[${diskmicrocodetypecounter}]%,*}"
        fi
done
echo "-----------------------------------------------"
echo ""

exit 0