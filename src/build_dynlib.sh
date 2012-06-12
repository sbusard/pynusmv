#! /usr/bin/env sh

ar rc lib/libnusmv.a nusmv/*.o nusmv/src/*/*.o nusmv/src/*/*/*.o nusmv/src/*/*/*/*.o nusmv/src/*/*/*/*/*.o
ar d lib/libnusmv.a ltl2smvMain.o
ar d lib/libnusmv.a SatMinisat.o
gcc -dynamiclib -flat_namespace -all_load -undefined suppress -lexpat -lreadline -o lib/libnusmv.dylib cudd-2.4.1.1/lib/*.a lib/libnusmv.a 
ln -s lib/libnusmv.dylib .

