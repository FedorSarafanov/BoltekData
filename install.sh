#!/usr/bin/bash

read usbtty < <(
    declare -a array=()
    for sym in /dev/serial/by-id/*;
    do 
        address=`readlink -f $sym`;
        clearsym=`echo $sym | sed "s|/dev/serial/by-id/||g"`;
        pid=`udevadm info -a -n /dev/ttyUSB0 | grep idProduct | head -n 1 | sed s/.*==//g | sed s/\"//g`;
        vid=`udevadm info -a -n /dev/ttyUSB0 | grep idVendor | head -n 1 | sed s/.*==//g | sed s/\"//g`;
    	array+=("serial $vid:$pid" "$clearsym")
    done
    for sid in `lsusb -v 2>/dev/null | grep -A 15 '0403:f245' | grep iSerial | awk '{print $3}'`;
    do 
        address=`readlink -f $sym`;
        name=`lsusb -v 2>/dev/null | grep -A 15 '0403:f245' | grep -B 10 $sid | grep iProduct |  awk '{$1=$2=""; print $0}'`
        pid="f245";
        vid="0403";
        array+=("libusb $sid" "$name")
    done    
    whiptail --title  "Install boltek-efm measure tool" --menu 'Select device' 20 100 12 "${array[@]}" 2>&1 >/dev/tty
)

devtype=`echo $usbtty| awk '{print $1;}'`;
vidpid=`echo $usbtty| awk '{print $2;}'`;
vid=${vidpid%:*}; 
pid=${vidpid#*:};

case "$devtype" in
    ("libusb")
         echo "PID = 0xf245" >> test-boltek-efm.ini;
         echo "SID = $vidpid" >> test-boltek-efm.ini;
    ;;
    ("serial")
        symlink=$(whiptail --title  "Install boltek-efm measure tool" --inputbox  "Input dev symlink for /dev/" 20 76 "TTY_BOLTEK_0" 3>&1 1>&2 2>&3);
        exitstatus=$?
        if [ $exitstatus = 0 ];  then
            echo "SUBSYSTEM==\"tty\", ATTRS{idVendor}==\"$vid\", ATTRS{idProduct}==\"$pid\", MODE=\"0666\", SYMLINK+=\"$symlink\"" >> \
                102-boltek.rules;
            echo "tty = /dev/$symlink" >> test-boltek-efm.ini
        fi
    ;;
    (*) echo "$status";;
esac


find_by_id(){
    v=${1%:*}; p=${1#*:}  # split vid:pid into 2 vars
    v=${v#${v%%[!0]*}}; p=${p#${p%%[!0]*}}  # strip leading zeros
    grep -il "^PRODUCT=$v/$p" /sys/bus/usb/devices/*:*/uevent |
    sed s,uevent,, |
    xargs -r grep -r '^DEVNAME=' --include uevent
}


add_usbtty_to_udev(){
    v=${1%:*}; p=${1#*:}  # split vid:pid into 2 vars
	symlink=$(whiptail --title  "Install boltek-efm measure tool" --inputbox  "Input dev symlink for /dev/" 20 76 "TTY_BOLTEK_0" 3>&1 1>&2 2>&3)
 	exitstatus=$?
	if [ $exitstatus = 0 ];  then
    	echo "SUBSYSTEM==\"tty\", ATTRS{idVendor}==\"$v\", ATTRS{idProduct}==\"$p\", MODE=\"0666\", SYMLINK+=\"$symlink\"" >> \
    		102-boltek.rules;
    	echo "tty = /dev/$symlink" >> test-boltek-efm.ini
	else
    	temp=1;
	fi
}


add_usbtty_to_udev $usbtty;
prefix=$(whiptail --title  "Install boltek-efm measure tool" --inputbox  "Input data files prefix" 20 76 "test" 3>&1 1>&2 2>&3)
echo "prefix = $prefix" >> test-boltek-efm.ini;

folder=$(whiptail --title  "Install boltek-efm measure tool" --inputbox  "Input folder for data files" 20 76 "data" 3>&1 1>&2 2>&3)
mkdir -p $folder;
echo "folder = $folder" >> test-boltek-efm.ini;


log=$(whiptail --title  "Install boltek-efm measure tool" --inputbox  "Input log filename" 20 76 "boltek-efm.log" 3>&1 1>&2 2>&3)
mkdir -p $log;
echo "log = $log" >> test-boltek-efm.ini;
