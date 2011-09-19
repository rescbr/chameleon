#!/bin/bash

echo "==============================================="
echo "CheckFatType: Do we have FAT16 or FAT32?"
echo "****************************************"

# Receives passed value for the raw Target Device
# for example: /dev/rdisk0s2
# Then looks for the following in the partition boot sector
# Byte number 19 to see if it's either 00 or 02
# Byte number 22 to see if it's either F8 or F0
# Byte number 25 to see if it's either 3F or 20
#
# Exit with value 1 for FAT16, 2 for FAT32 
# Exit with value 0 if nothing is found - this shouldn't happen.?

if [ "$#" -eq 1 ]; then
	targetDeviceRaw="$1"
	echo "DEBUG: passed argument = $targetDeviceRaw"
else
	echo "Error - wrong number of values passed"
	exit 9
fi


partitionBootSector=$( dd 2>/dev/null if="$targetDeviceRaw" count=1 | perl -ne '@a=split"";for(@a){printf"%02x",ord}' )
if [ "${partitionBootSector:36:2}" == "00" ] && [ "${partitionBootSector:42:2}" == "f8" ] && [ "${partitionBootSector:48:2}" == "3f" ]; then
	echo "Found a FAT32 device formatted by Windows Explorer"
	echo "-----------------------------------------------"
	echo ""
	exit 2
fi
if [ "${partitionBootSector:36:2}" == "02" ] && [ "${partitionBootSector:42:2}" == "f8" ] && [ "${partitionBootSector:48:2}" == "3f" ]; then
	echo "Found a FAT16 device formatted by Windows Explorer"
	echo "-----------------------------------------------"
	echo ""
	exit 1
fi
if [ "${partitionBootSector:36:2}" == "00" ] && [ "${partitionBootSector:42:2}" == "f0" ] && [ "${partitionBootSector:48:2}" == "20" ]; then
	echo "Found a FAT32 device formatted by OSX Disk Utility"
	echo "-----------------------------------------------"
	echo ""
	exit 2
fi
if [ "${partitionBootSector:36:2}" == "02" ] && [ "${partitionBootSector:42:2}" == "f0" ] && [ "${partitionBootSector:48:2}" == "20" ]; then
	echo "Found a FAT16 device formatted by OSX Disk Utility"
	echo "-----------------------------------------------"
	echo ""
	exit 1
fi

echo "-----------------------------------------------"
echo ""
exit 0