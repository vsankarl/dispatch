Summary
Framework that demonstrates dispatch table mechanism. The dispatch table has a custom key with the ability to add function pointers. The function pointers are templatized to have the ability to arbitrarily add any number or type of arguments and any return type.


Environment:
Ubuntu 16.04 - 64 bit
GCC version - gcc (Ubuntu 6.5.0-2ubuntu1~16.04) 6.5.0 20181026

Application output - evebus.
Design and assumptions are in the respective headers.

Steps to build
1. extract to directory.
2. cd to that top level directory "adobe"
3. $make
4. $./evebus
5  "log.txt" gets generated in the same directory along with console output

