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
	echo "DEBUG: passed argument = $logLocation"
	echo "DEBUG: passed argument = $verboseText"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

logName="Chameleon_Installer_Log.txt"
logFile="${logLocation}"/$logName

if [ -f "${logFile}" ]; then
	echo "${verboseText}" >> "${logFile}"
else
	echo "${verboseText}" > "${logFile}"
fi

exit 0