#Testing Script for CS475 Project 4
#Jesse Thoren
#Description: Compiles/runs project4.cpp

import os
cmd = "g++ project4.cpp -o runProg -lm -fopenmp"
os.system( cmd )
cmd = "./runProg"
os.system( cmd )
cmd = "rm -f runProg"
os.system( cmd )
