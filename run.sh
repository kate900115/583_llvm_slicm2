#export PATH=#PATH:/opt/llvm/Release+Asserts/bin
fname=$1
rm llvmprof.out
clang -emit-llvm -o $fname.bc -c $fname.c 
opt -loop-simplify < $fname.bc > $fname.ls.bc 
opt -insert-edge-profiling $fname.ls.bc -o $fname.profile.ls.bc
llc $fname.profile.ls.bc -o $fname.profile.ls.s
g++ -o $fname.profile $fname.profile.ls.s /opt/llvm/Release+Asserts/lib/libprofile_rt.so 
./$fname.profile $2

opt -load Debug+Asserts/lib/ldstats.so -lamp-insts -insert-lamp-profiling -insert-lamp-loop-profiling -insert-lamp-init < $fname.ls.bc > $fname.lamp.bc 
llc < $fname.lamp.bc > $fname.lamp.s 
g++ -o $fname.lamp.exe $fname.lamp.s tools/lamp-profiler/lamp_hooks.o 
./$fname.lamp.exe $2

echo "Done generating lamp profile"
 
opt -load Debug+Asserts/lib/ldstats.so -lamp-inst-cnt -lamp-map-loop -lamp-load-profile -profile-loader -profile-info-file=llvmprof.out -ldstats < $fname.ls.bc 
