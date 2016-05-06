#!/bin/sh

g++ -std=c++11 -I /usr/include/boost client.cpp -lboost_system -lboost_thread -o client
