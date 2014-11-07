#!/usr/bin/env bash

for (( i = 0; i < 30; i++ )); do
	./server_tester.py 127.0.0.1 &
done