<?php

system('make clean');
system('cp lib/Makefile lib/libhydrogen/Makefile');
system('cd lib/libhydrogen && make shared_lib && cd ../..');
system('phpize');
system('./configure');
system('make');
