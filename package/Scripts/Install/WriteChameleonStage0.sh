#!/bin/bash

echo "==============================================="
echo "Write Chameleon Stage 0 Loader:"
echo "*******************************"

# this script is passed….

if [ "$#" -eq 7 ]; then
	diskupdate="$1"
	disksignature="$2"
	stage0Loader="$3"
	stage0Loaderdualboot="$4"
	targetDisk="$5"
	targetVolume="$6"
	scriptDir="$7"
	echo "DEBUG: passed argument for diskupdate = $diskupdate"
	echo "DEBUG: passed argument for disksignature = $disksignature"
	echo "DEBUG: passed argument for stage0Loader = $stage0Loader"
	echo "DEBUG: passed argument for stage0Loaderdualboot = $stage0Loaderdualboot"
	echo "DEBUG: passed argument for targetDisk = $targetDisk"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
	echo "DEBUG: passed argument for scriptDir = $scriptDir"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

if [ ${diskupdate} = "0" ]; then
	echo "Diskupdate = true, so yes"
	
	if [ ${disksignature} = "0" ]; then
		# There’s no Windows disk signature then we can write boot0
		
		echo "Executing command: fdisk440 -u -f /usr/standalone/i386/${stage0Loader} -y ${targetDisk}"
		"${scriptDir}"/Tools/fdisk440 -u -f "${targetVolume}"/usr/standalone/i386/${stage0Loader} -y ${targetDisk}
	else
		# Windows is also installed on the HDD and we need to write boot0hfs
		
		echo "Executing command: /fdisk440 -u -f /usr/standalone/i386/${stage0Loaderdualboot} -y ${targetDisk}"
		"${scriptDir}"/Tools/fdisk440 -u -f "${targetVolume}"/usr/standalone/i386/${stage0Loaderdualboot} -y ${targetDisk}
	fi
fi

echo "-----------------------------------------------"
echo ""
echo ""

exit 0