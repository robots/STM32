#!/bin/sh

module="canf_ktest"
device="canf_ktest"
mode="664"

# insert neccessary modules
/sbin/insmod ../../drivers/can_virtual_kernel/can_virtual.ko
/sbin/insmod ../../src/canfestival.ko

# insert module with all arguments we got
# and use a pathname, as newer modutils don't look in . by default
/sbin/insmod ./$module.ko $* || exit 1

# remove stale nodes
rm -f /dev/${device}

# create device file
major=$(awk "\$2==\"$device\" {print \$1; exit}" /proc/devices)
mknod /dev/${device} c $major 0

# give appropriate group/permissions, and change the group.
# Not all distributions have staff, some have "wheel" instead.
group="staff"
grep -q '^staff:' /etc/group || group="wheel"
chgrp $group /dev/${device}
chmod $mode /dev/${device}
