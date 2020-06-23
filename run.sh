list=$(pidof -o %PPID get_data)

i=0
for PID in $list
do
	echo "Killing $PID...";
	p=`ls -ald --color=never /proc/$PID/exe | awk '{ print $11 }'`
	if [[ $p == *"BoltekData"* ]]; then
		((i=i+1))
	fi
done

if [ "$i" -eq "0" ]; then
	cd ~/BoltekData;
   exec ~/BoltekData/get_data;
fi