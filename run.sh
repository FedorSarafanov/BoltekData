ps ax -o pid,cmd > temp; cat temp | grep boltek-efm | grep  > /dev/null && echo  || exec /home/osabio/projects/boltek_measure/BoltekData/boltek-efm --ini=/boltek-efm.ini
