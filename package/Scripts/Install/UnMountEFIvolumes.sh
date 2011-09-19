#!/bin/bash

echo "==============================================="
echo "Unmount all volumes named EFI"
echo "*****************************"

# loop through and un-mount ALL mounted EFI system partitions - Thanks kizwan

attempts=1
while [ "$( df | grep EFI )" ] && [ "${attempts}" -lt 5 ]; do
	echo "Unmounting $( df | grep EFI | awk '{print $1}' )"
	umount -f $( df | grep EFI | awk '{print $1}' )
	attempts=$(( ${attempts} + 1 ))
done
if [ ${attempts} = 5 ]; then
	echo "failed to unmount EFI System Partition."
	echo "-----------------------------------------------"
	echo ""
	echo ""
	echo ""
	exit 1
fi

echo "-----------------------------------------------"
echo ""
echo ""
echo ""

exit 0



