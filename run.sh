filename=$1
rm llvmprof.out
clang -emit-llvm -o $filename.bc -c $filename.c
opt -loop-simplify < $filename.bc > $filename.ls.bc
opt -basicaa -licm $filename.bc -o $filename.licm.bc
llvm-dis $filename.bc
llvm-dis $filename.licm.bc

opt -insert-edge-profiling $filename.ls.bc -o $filename.profile.ls.bc
llc $filename.profile.ls.bc -o $filename.profile.ls.s
g++ -o $filename.profile $filename.profile.ls.s /opt/llvm/Release+Asserts/lib/libprofile_rt.so 
./$filename.profile 


#opt -basicaa -load Debug+Asserts/lib/slicmpass.so -lamp-inst-cnt -lamp-map-loop -lamp-load-profile -profile-loader -profile-info-file=llvmprof.out -slicmpass < $filename.ls.bc > /dev/null
opt -basicaa -load Debug+Asserts/lib/slicmpass.so -lamp-inst-cnt -lamp-map-loop -lamp-load-profile -profile-loader -profile-info-file=llvmprof.out -slicmpass < $filename.ls.bc
opt -basicaa -load Debug+Asserts/lib/slicmpass.so -lamp-inst-cnt -lamp-map-loop -lamp-load-profile -profile-loader  $filename.bc -o $filename.lamp.bc
opt -load Debug+Asserts/lib/slicmpass.so -slicmpass $filename.lamp.bc -o $filename.slicm.bc
opt -mem2reg $filename.slicm.bc -o $filename.slicmreg.bc
llvm-dis $filename.slicm.bc
llc $filename.bc -o $filename.s
llc $filename.slicm.bc -o $filename.slicm.s
g++ -o $filename $filename.s
g++ -o $filename.slicm $filename.slicm.s
opt -dot-cfg < $filename.bc >& /dev/null 
