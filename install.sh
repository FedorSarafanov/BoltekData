#!/usr/bin/bash
currdir=`pwd`;
rm $currdir/boltek-efm-temp.ini 2>&1 >/dev/null;
rm $currdir/102-boltek.rules 2>&1 >/dev/null;

read usbtty < <(
    declare -a array=()
    if [ -d "/dev/serial/by-id" ]; then
	    for sym in /dev/serial/by-id/*;
	    do 
	        address=`readlink -f $sym 2>/dev/null`;
	        clearsym=`echo $sym | sed "s|/dev/serial/by-id/||g"`;
	        pid=`udevadm info -a -n /dev/ttyUSB0 | grep idProduct | head -n 1 | sed s/.*==//g | sed s/\"//g`;
	        vid=`udevadm info -a -n /dev/ttyUSB0 | grep idVendor | head -n 1 | sed s/.*==//g | sed s/\"//g`;
	    	array+=("serial $vid:$pid" "$clearsym")
	    done
	fi
    for sid in `lsusb -v 2>/dev/null | grep -A 15 '0403:f245' | grep iSerial | awk '{print $3}'`;
    do 
        address=`readlink -f $sym 2>/dev/null`;
        name=`lsusb -v 2>/dev/null | grep -A 15 '0403:f245' | grep -B 10 $sid | grep iProduct |  awk '{$1=$2=""; print $0}'`
        pid="f245";
        vid="0403";
        array+=("libusb $sid" "$name")
    done    
    if [ ${#array[@]} -eq 0 ]; then
    	echo ERROR;
	else
	    whiptail --title  "Install boltek-efm measure tool" --menu 'Select device' 20 100 12 "${array[@]}" 2>&1 >/dev/tty
	fi
)

if [ "$usbtty" = "ERROR" ]; then
	exit;
fi




echo "[Config Boltek EFM-100 with USB]" > $currdir/boltek-efm-temp.ini;

devtype=`echo $usbtty| awk '{print $1;}'`;
vidpid=`echo $usbtty| awk '{print $2;}'`;
vid=${vidpid%:*}; 
pid=${vidpid#*:};

case "$devtype" in
    ("libusb")
         echo "PID = 0xf245" >> $currdir/boltek-efm-temp.ini;
         echo "SID = $vidpid" >> $currdir/boltek-efm-temp.ini;
         echo "tty = none" >> $currdir/boltek-efm-temp.ini;
         echo "SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"0403\", ATTRS{idProduct}==\"f245\", MODE=\"0666\"" >> \
            $currdir/102-boltek.rules;
    ;;
    ("serial")
        symlink=$(whiptail --title  "Install boltek-efm measure tool" --inputbox  "Input dev symlink for /dev/" 20 76 "TTY_BOLTEK_0" 3>&1 1>&2 2>&3);
        exitstatus=$?
        if [ $exitstatus = 0 ];  then
            echo "SUBSYSTEM==\"tty\", ATTRS{idVendor}==\"$vid\", ATTRS{idProduct}==\"$pid\", MODE=\"0666\", SYMLINK+=\"$symlink\"" >> \
                $currdir/102-boltek.rules;
            echo "tty = /dev/$symlink" >> $currdir/boltek-efm-temp.ini
        fi
    ;;
    (*) echo "$status";;
esac

prefix=$(whiptail --title  "Install boltek-efm measure tool" --inputbox  "Input data files prefix" 20 76 "test" 3>&1 1>&2 2>&3)
echo "prefix = $prefix" >> $currdir/boltek-efm-temp.ini;

folder=$(whiptail --title  "Install boltek-efm measure tool" --inputbox  "Input folder for data files (only one word)" 20 76 "data" 3>&1 1>&2 2>&3)
mkdir -p $folder;
echo "folder = $folder" >> $currdir/boltek-efm-temp.ini;


log=$(whiptail --title  "Install boltek-efm measure tool" --inputbox  "Input log filename (without dir)" 20 76 "boltek-efm.log" 3>&1 1>&2 2>&3)
echo "log = $folder/$log" >> $currdir/boltek-efm-temp.ini;


syncdir=$(whiptail --title  "Install boltek-efm measure tool" --inputbox  "Input diogen path for rsync" 20 76 "/mnt/a/observations/fluxmeters/" 3>&1 1>&2 2>&3)


mv $currdir/boltek-efm-temp.ini  $currdir/$folder/boltek-efm.ini;
echo "cd $currdir; ps ax -o pid,cmd > temp; cat temp | grep boltek-efm | grep $folder > /dev/null && echo "" || exec $currdir/boltek-efm --ini=$folder/boltek-efm.ini" > \
    $currdir/$folder/run.sh;
chmod +x $currdir/$folder/run.sh;


crontab -l > mycron;
echo "*/1 * * * * ($currdir/$folder/run.sh)" >> mycron;
echo "*/15 * * * * (rsync -zavP $currdir$/$folder boltek@diogen.iapras.ru:$syncdir >/dev/null 2>/dev/null)" >> mycron;
crontab mycron;
rm mycron;

sudo cp $currdir/102-boltek.rules /etc/udev/rules.d/102-boltek.rules;
sudo udevadm control --reload-rules && sudo udevadm trigger