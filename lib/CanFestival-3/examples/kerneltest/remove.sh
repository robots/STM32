#!/bin/sh

module="canf_ktest"
device="canf_ktest"

/sbin/rmmod $module || exit 1
/sbin/rmmod canfestival
/sbin/rmmod can_virtual

rm -f /dev/${device}

