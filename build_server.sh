#!/bin/sh

g++ -std=c++11 -I /usr/include/boost server.cpp -lboost_system -lboost_thread -o server
