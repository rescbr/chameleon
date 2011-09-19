#!/bin/bash

echo "==============================================="
echo "CheckWindowsDiskSignature: Is Windows installed?"
echo "************************************************"

# this script is passed the targetdisk to work from, for example /dev/disk0
# It then checks the disk sector for a 4-byte Windows disk signature
# if one is found then it exits with 1, otherwise it exits with 0

if [ "$#" -eq 1 ]; then
	targetDisk="$1"
	echo "DEBUG: passed argument for targetDisk = $targetDisk"
else
	echo "Error - wrong number of values passed - Exiting"
	exit 9
fi

disksignature=$( dd 2>/dev/null if="$targetDisk" count=1 | dd 2>/dev/null count=4 bs=1 skip=440 | perl -ne '@a=split"";for(@a){printf"%02x",ord}' )

echo "${disksignature}"

if [ "${disksignature}" = "00000000" ]; then
	echo "No Windows installation detected."
	echo "-----------------------------------------------"
	echo ""
	exit 0
else
	echo "Detected an existing Windows installation"
	echo "-----------------------------------------------"
	echo ""
	exit 1
fi

echo "-----------------------------------------------"
echo ""

exit 0