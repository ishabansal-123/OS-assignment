#!/bin/sh
dd if=/dev/zero of=/swapfile bs=1024 count=1048576
chmod 600 /swapfile
mkswap /swapfile
swapon /swapfile
