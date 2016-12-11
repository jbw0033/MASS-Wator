#!/bin/sh

export MASS_DIR=../..
g++ -Wall Nomad.cpp -I$MASS_DIR/source -shared -fPIC -o Nomad
g++ -Wall Land.cpp -I$MASS_DIR/source -shared -fPIC -o Land
g++ -Wall main.cpp Timer.cpp -I$MASS_DIR/source -L$MASS_DIR/ubuntu -lmass -I$MASS_DIR/ubuntu/ssh2/include -L$MASS_DIR/ubuntu/ssh2/lib -lssh2 -o main
