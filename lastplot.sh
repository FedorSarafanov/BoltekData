#!/bin/bash
file=$(ls -1t | grep .txt | head -n 2)
python3 load.py $file