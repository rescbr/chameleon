#!/bin/bash

echo "==============================================="
echo "Write Chameleon Stage 1 Loader:"
echo "*******************************"

# espformat code is 1 for HFS, 2 for MSDOS, 0 for unknown

if [ "$#" -eq 6 ]; then
	espformat="$1"
	stage1LoaderHFS="$2"
	stage1LoaderFAT="$3"
	selectedDestination="$4"
	targetDeviceRaw="$5"
	targetVolume="$6"
	echo "DEBUG: passed argument for espformat = $espformat"
	echo "DEBUG: passed argument for stage1LoaderHFS = $stage1LoaderHFS"
	echo "DEBUG: passed argument for stage1LoaderFAT = $stage1LoaderFAT"
	echo "DEBUG: passed argument for selectedDestination = $selectedDestination"
	echo "DEBUG: passed argument for targetDeviceRaw = $targetDeviceRaw"
	echo "DEBUG: passed argument for targetVolume = $targetVolume"
else
	echo "Error - wrong number of values passed"
	exit 9
fi

if [ ${espformat} = "1" ]; then
	# the selected partition is HFS formatted

	echo "Executing command: dd if=${selectedDestination}/usr/standalone/i386/${stage1LoaderHFS} of=${targetDeviceRaw}"
	dd if="${selectedDestination}"/usr/standalone/i386/${stage1LoaderHFS} of=${targetDeviceRaw}
fi

if [ ${espformat} = "2" ]; then
	# the selected partition FAT formatted

	echo "Executing command: dd if=${targetDeviceRaw} count=1 bs=512 of=/tmp/origbs"
	dd if=${targetDeviceRaw} count=1 bs=512 of=/tmp/origbs

	echo "Executing command: cp "${selectedDestination}"/usr/standalone/i386/${stage1LoaderFAT} /tmp/newbs"
	cp "${selectedDestination}"/usr/standalone/i386/${stage1LoaderFAT} /tmp/newbs

	echo "Executing command: dd if=/tmp/origbs of=/tmp/newbs skip=3 seek=3 bs=1 count=87 conv=notrunc"
	dd if=/tmp/origbs of=/tmp/newbs skip=3 seek=3 bs=1 count=87 conv=notrunc

	echo "Executing command: dd of=${targetDeviceRaw} count=1 bs=512 if=/tmp/newbs"
	dd if=/tmp/newbs of="${targetDeviceRaw}" count=1 bs=512
fi

echo "-----------------------------------------------"
echo ""

exit 0