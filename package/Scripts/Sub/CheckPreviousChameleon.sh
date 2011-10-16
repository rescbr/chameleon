#!/bin/bash

echo "==============================================="
echo "Check Previous Chameleon: Will there be problems?"
echo "***********************************************"

# Checks the selected volume is present and the disk is partitioned
# Now also check for another existing Chameleon installation on the same disk.

# Receives targetDisk: for example, /dev/disk3.
# Receives targetDeviceRaw: for example, /dev/rdisk3s1.
# Receives targetDevice: Stores device number, for example /dev/disk2s1.
# Receives installerVolume: Volume to write the installer log to.
# Receives scriptDir: The location of the main script dir.

if [ "$#" -eq 5 ]; then
	targetDisk="$1"
	targetDeviceRaw="$2"
	targetDevice="$3"
	installerVolume="$4"
	scriptDir="$5"
	echo "DEBUG: passed argument for targetDisk = $targetDisk"
	echo "DEBUG: passed argument for targetDeviceRaw = $targetDeviceRaw"
	echo "DEBUG: passed argument for targetDevice = $targetDevice"
	echo "DEBUG: passed argument for installerVolume = $installerVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi


# Check for existing Chameleon installations on a different
# partition of the same target disk.
#echo "DEBUG: Checking for existing Chameleon installations on ${targetDisk#/dev/}..."

sliceNumber=$( echo ${targetDeviceRaw#*disk*s} )
# strip slice from end
targetDiskRawNoSlice=$( echo ${targetDeviceRaw%$sliceNumber} )

# Are there any other partitions on the disk?
# How many actual partitions are there?
numSlices=$(( $( diskutil list | grep $( echo ${targetDisk#/dev/} ) | sed -n '$=' ) -2 ))

# Only check the disk for Chameleon installations if there is more than one partition.
if [ $numSlices -gt 1 ]; then 
		
	#Scan all partitions for Chameleon code
	for (( i=1; i <= $numSlices; i++ ));
	do
		stageExistence=0
		stage0FirstBootable=0
		previousExistence="NONE"
		targetDiskRaw=$targetDiskRawNoSlice$i

		# Check for existing stage 0 boot file (same code as CheckDiskMicrocode.sh script)
		stage0type=$( dd 2>/dev/null if="$targetDisk" count=3 bs=1 skip=105 | xxd | awk '{print $2$3}' )
		if [ "${stage0type}" == "0b807c" ] || [ "${stage0type}" == "0a803c" ] || [ "${stage0type}" == "ee7505" ] || [ "${stage0type}" == "742b80" ]; then
			#echo "DEBUG: boot0 found on $targetDisk"
			(( stageExistence++ ))
			
			# While here, check just for either existing boot0hfs, boot0md or boot0md (dmazar's boot0workV2)
			if [ "${stage0type}" == "0a803c" ] || [ "${stage0type}" == "ee7505" ] || [ "${stage0type}" == "742b80" ]; then
				stage0FirstBootable=1
			fi
		fi
			
		# Check for existence of a bootable partition boot sector containing either boot1h or boot1f32
		boot1Search=$( dd 2>/dev/null if="$targetDiskRaw" count=1 | perl -ne '@a=split"";for(@a){printf"%02x",ord}' )
		if [ "${boot1Search:0:16}" == "fa31c08ed0bcf0ff" ] && [ "${boot1Search:1020:2}" == "55" ]; then
			#echo "DEBUG: boot1h found on "$targetDiskRaw
			(( stageExistence++ ))
			previousExistence="boot1"
		elif [ "${boot1Search:0:4}" == "e962" ] && [ "${boot1Search:180:12}" == "424f4f542020" ] && [ "${boot1Search:1020:2}" == "55" ]; then
			#echo "DEBUG: boot1f32 found on "$targetDiskRaw
			(( stageExistence++ ))
			previousExistence="boot1f32"
		fi
		
		# Check for existing stage 2 boot file also
		# NOTE: This will fail to find /boot on /Volumes/EFI as it won't be mounted!
		if [ -e "$( df | grep ${targetDisk}s${i} | awk '{ print $6 }' )"/boot ]; then
			#echo "DEBUG: boot found on $targetDiskRaw"
			(( stageExistence++ ))
		fi
		
		#if [ $stageExistence == 3 ] && [ $i -ne $sliceNumber ]; then
			#echo "DEBUG: STOP: There is already an existing Chameleon installation on $targetDiskRaw"
			#"$scriptDir"InstallLog.sh "${installerVolume}" "STOP: There is already an existing Chameleon installation on $targetDiskRaw."
			#"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: This is allowed and does work as long as you aren't dual booting Windows"
			#"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: from the same disk and are happy to control which partition is used by"
			#"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: flagging the required partition active. General use doesn't require two"
			#"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: Chameleon installs on the same disk, though might be done by advanced users."
			#"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: If you still want to do this then proceed by installing Chameleon manually."
			#exit 1
		#fi
			
		if [ $stageExistence -ge 2 ] && [ "$previousExistence" != "NONE" ] && [ $i -ne $sliceNumber ]; then
			# There is previous Chameleon stage 1 code on a partition boot sector,
			# and either a complete or incomplete installation (ie. boot0 or boot are missing).
			if [ $stageExistence == 3 ]; then
				"$scriptDir"InstallLog.sh "${installerVolume}" "WARN: There is already an existing Chameleon installation on $targetDiskRaw."
			else
				"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: $previousExistence already exists at ${targetDisk}s${i}"
			fi
			
			# This could prove problematic and result in either a b1f:error or boot0:error 
			# if the following conditions are true:
			# A) Boot0hfs, Boot0md or Boot0md (dmazar's Boot0workV2) is being used.
			# B) The previous stage 1 code is on a lower partiton than the one being installed to now.
						
			# stage0FirstBootable=1 is used to know if 'A' is true.
			if [ $stage0FirstBootable == 1 ]; then
				# i = current slice we're checking, slicenumber = slice trying to install to.
				if [ $i -lt $sliceNumber ]; then
					"$scriptDir"InstallLog.sh "${installerVolume}" "WARN: Conditions point to you receiving a boot failure"
					# Fix by making previous paritionboot sector un-bootable
					# Change Byte 01FExh to 00 (510 decimal)
					"$scriptDir"InstallLog.sh "${installerVolume}" "---"
					"$scriptDir"InstallLog.sh "${installerVolume}" "FIX: Make ${targetDisk}s${i} boot sector un-bootable by changing byte 1FEh to 00."
					"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: Any Extra folder you had there will still be there. If you want to use"
					"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: ${targetDisk}s${i} again as your boot partition then re-run this installer"
					"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: selecting it as the target, ONLY choosing the 'Chameleon Bootloader' option"
					"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: and NONE of the other options."
					"$scriptDir"InstallLog.sh "${installerVolume}" "---"
					dd if=${targetDisk}s${i} count=1 bs=512 of=/tmp/originalBootSector
					cp /tmp/originalBootSector /tmp/newBootSector
					dd if="$scriptDir/patch" of=/tmp/newBootSector bs=1 count=1 seek=510 conv=notrunc
					dd if=/tmp/newBootSector of=${targetDisk}s${i} count=1 bs=512
				else
					"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: but won't interfere as you're installing to an earlier partition."
				fi
			else
				"$scriptDir"InstallLog.sh "${installerVolume}" "NOTE: so select to boot that partition (if used) with active flag."
			fi
		fi
	done

#else
	#echo "DEBUG: Just one slice"
fi

exit 0