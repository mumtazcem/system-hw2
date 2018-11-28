#!/bin/bash

echo "---------- dmesg -c ---------- "
dmesg -c
echo "---------- module is made ---------- "
make -C /lib/modules/3.13.0-24-generic/build M=`pwd` modules
echo "---------- rm /dev/queue0---------- "
rm /dev/queue0
echo "---------- rm /dev/queue1---------- "
rm /dev/queue1
echo "---------- rm /dev/queue2---------- "
rm /dev/queue2
echo "---------- rm /dev/queue3---------- "
rm /dev/queue3
echo "---------- rm /dev/queue4---------- "
rm /dev/queue4
echo "---------- rmmod hw2---------- "
rmmod hw2

echo "---------- insmod ./hw2.ko ---------- "
insmod ./hw2.ko hw2_nr_devs=4
echo "---------- dmesg ---------- "
dmesg
echo "---------- mknod /dev/queue0 c 250 0 ---------- "
mknod /dev/queue0 c 250 0
echo "---------- mknod /dev/queue1 c 250 1 ---------- "
mknod /dev/queue1 c 250 1
echo "---------- mknod /dev/queue2 c 250 2 ---------- "
mknod /dev/queue2 c 250 2
echo "---------- mknod /dev/queue3 c 250 3 ---------- "
mknod /dev/queue3 c 250 3
echo "---------- echo "queue0" > /dev/queue0 ---------- "
echo "queue0" > /dev/queue0
echo "---------- echo "queue1" > /dev/queue ---------- "
echo "queue1" > /dev/queue1
echo "---------- cat /dev/q0  ---------- "
cat /dev/queue0 
echo "---------- cat /dev/q1  ---------- "
cat /dev/queue1 
echo "---------- end ---------- "
