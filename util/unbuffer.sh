#!/usr/bin/expect --
# Description: unbuffer stdout of a program
# Author: Don Libes, NIST
set stty_init "-opost"
eval spawn -noecho $argv
set timeout -1
expect
