#!/bin/bash
cd ~/BoltekData;
ps -C get_data >/dev/null && echo "" || exec ~/BoltekData/get_data &