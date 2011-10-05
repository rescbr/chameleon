#!/bin/bash

echo "==============================================="
echo "Check Proceed: Can the installation continue?"
echo "***********************************************"

# Checks the selected volume is present and the disk is partitioned
# Now also check for another existing Chameleon installation on the same disk.

# Receives targetDisk: for example, /dev/disk3.
# Receives targetDeviceRaw: for example, /dev/rdisk3s1.
# Receives targetVolume: Volume to install to (will be '/Volumes/EFI' if EFI install)
# Receives targetDevice: Stores device number, for example /dev/disk2s1.
# Receives installerVolume: Volume to write the installer log to.
# Receives scriptDir: The location of the main script dir.

if [ "$#" -eq 6 ]; then
	targetDisk="$1"
	targetDeviceRaw="$2"
	targetVolume="$3"
	targetDevice="$4"
	installerVolume="$5"
	scriptDir="$6"
	echo "DEBUG: passed argument for targetDisk = $targetDisk"
	echo "DEBUG: passed argument for targetDeviceRaw = $targetDeviceRaw"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for targetDevice = $targetDevice"
	echo "DEBUG: passed argument for installerVolume = $installerVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi


# Does target volume exist?
if [ -z "$targetVolume" ]; then
	echo "*** Cannot find the volume. Exiting."
	"$scriptDir"InstallLog.sh "${installerVolume}" "FAIL: Cannot file the volume: $targetVolume."
	exit 1
else
	echo "Confirming target volume exists"
fi


# Does target volume use slices?
if [ "$targetDevice" = "$targetDevice#*disk*s" ]; then
	echo "*** ERROR Volume does not use slices. Exiting."
	"$scriptDir"InstallLog.sh "${installerVolume}" "FAIL: $targetVolume doesn't use slices."
	exit 1		
else
	echo "Confirming target device uses slices"
fi


# Add check for installing to a 'small' HFS device like a
# 1GB USB flash drive which won't have an EFI System Partition.
if [ "$targetVolume" = "/Volumes/EFI" ]; then
	# Take target device and check slice 1 matches partition named "EFI"
	stripped=$( echo ${targetDevice#/dev/} )
	if [ ! $(echo ${stripped#*disk*s}) = 1 ]; then
		stripped=$( echo ${stripped%s*})"s1"
	fi
	if [ ! $( diskutil list | grep ${stripped} | awk {'print $2'} ) = "EFI" ]; then
		echo "*** The selected volume doesn't have an EFI System Partition. Exiting."
		"$scriptDir"InstallLog.sh "${installerVolume}" "FAIL: Selected disk does not have an EFI System Partition."
		exit 1
	fi
fi


# Check for existing Chameleon installations on a different
# partition of the same target disk.
echo "Checking for existing Chameleon installations on ${targetDisk#/dev/}..."

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
		targetDiskRaw=$targetDiskRawNoSlice$i

		# Check for existing stage 0 boot file from CheckDiskMicrocode.sh script
		stage0type=$( dd 2>/dev/null if="$targetDisk" count=3 bs=1 skip=105 | xxd | awk '{print $2$3}' )
		if [ "${stage0type}" == "0b807c" ] || [ "${stage0type}" == "0a803c" ] || [ "${stage0type}" == "ee7505" ]; then
			echo "boot0 found on $targetDisk"
			(( stageExistence++ ))
		fi
			
		# Check for boot1h and boot1f32
		boot1hSearch=$( dd 2>/dev/null if="$targetDiskRaw" count=1 | perl -ne '@a=split"";for(@a){printf"%02x",ord}' )
		if [ "${boot1hSearch:0:16}" == "fa31c08ed0bcf0ff" ]; then
			echo "boot1h found on "$targetDiskRaw
			(( stageExistence++ ))
		elif [ "${boot1hSearch:0:4}" == "e962" ] && [ "${boot1hSearch:180:12}" == "424f4f542020" ]; then
			echo "boot1f32 found on "$targetDiskRaw
			(( stageExistence++ ))
		fi
		
		# Check for existing stage 2 boot file also
		if [ -e "$( df | grep $targetDisk"s"$i | awk '{ print $6 }' )"/boot ]; then
			echo "boot found on $targetDiskRaw"
			(( stageExistence++ ))
		fi
		
		if [ $stageExistence == 3 ] && [ $i -ne $sliceNumber ]; then
			echo "STOP: Not recommended you install to $targetDeviceRaw as there is already an existing Chameleon installation on $targetDiskRaw"
			"$scriptDir"InstallLog.sh "${installerVolume}" "STOP: Not recommended you install to $targetDeviceRaw as there is already an existing Chameleon installation on $targetDiskRaw."
			echo "================================"
			echo "End"
			echo "--------------------------------"
			exit 1
		fi
	done

else
	echo "Just one slice"
fi


echo "-----------------------------------------------"
echo ""

#"$scriptDir"InstallLog.sh "${installerVolume}" "CheckProceed: PASS"

exit 0