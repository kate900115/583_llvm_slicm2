export PATH=$PATH:/opt/llvm/Release+Asserts/bin
./configure && make
clang -emit-llvm -o test.bc -c test.c
opt -loop-simplify < test.bc > test.ls.bc
opt -load Debug+Asserts/lib/mypass.so -lamp-insts -insert-lamp-profiling -insert-lamp-loop-profiling -insert-lamp-init < test.ls.bc > test.lamp.bc
llc <test.lamp.bc>test.lamp.s
g++ -o test.lamp.exe test.lamp.s tools/lamp-profiler/lamp_hooks.o
./test.lamp.exe
opt -load Debug+Asserts/lib/mypass.so -lamp-inst-cnt -lamp-map-loop -lamp-load-profile -mypass < test.ls.bc
