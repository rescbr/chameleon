#!/bin/bash

echo "==============================================="
echo "InstallLog: Create/Append installation log"
echo "**********************************************"

# Creates text file named 'Chameleon_Installer_Log.txt'
# at the root of the target volume. This is to help show the
# user why the installation process failed (even though the
# package installer ends reading 'Installation Successful'. 

# Receives two parameters
# $1 = selected volume for location of the install log
# $2 = text to write to the installer log

if [ "$#" -eq 2 ]; then
	logLocation="$1"
	verboseText="$2"
	echo "DEBUG: passed argument = ${logLocation}"
	echo "DEBUG: passed argument = ${verboseText}"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

logName="Chameleon_Installer_Log.txt"
logFile="${logLocation}"/$logName

# On first run, create a file named .ChameleonLogFlag at
# the root of the target volume. Then check for this file
# on subsequent runs to know the initialisation sequence
# has been done.

if [ ! -f "${logLocation}"/.ChameleonLogFlag ]; then
	# This is the first run, so setup 
	# Chameleon_Installer_Log.txt file
	# by writing header.

	# Also include the first message that this script
	# would be called with which will be version/revision
	# of Chameleon package.

	echo "Chameleon installer log - $( date )
${verboseText}
======================================================" >"${logFile}"
	
	# Create /.ChameleonLogFlag file.
	echo "Log" >"${logLocation}"/.ChameleonLogFlag
else
	# Append messages to the log as passed by other scripts.
	if [ "${verboseText}" = "Diskutil" ]; then
		diskutil list >>"${logFile}"
	echo "======================================================" >>"${logFile}"
	fi

	if [ "${verboseText}" = "LineBreak" ]; then
		echo "======================================================" >>"${logFile}"
	fi

	if [[ "${verboseText}" == *fdisk* ]]; then
		targetDiskRaw="${verboseText#fdisk *}"
		fdisk $targetDiskRaw >>"${logFile}"
		echo " " >>"${logFile}"
	fi

	if [ "${verboseText}" != "LineBreak" ] && [[ "${verboseText}" != *fdisk* ]] && [[ "${verboseText}" != "Diskutil" ]]; then
		echo "${verboseText}" >> "${logFile}"
	fi
fi

exit 0