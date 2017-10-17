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
./$filename.profile $2


#opt -basicaa -load Debug+Asserts/lib/slicmpass.so -lamp-inst-cnt -lamp-map-loop -lamp-load-profile -profile-loader -profile-info-file=llvmprof.out -slicmpass < $filename.ls.bc > /dev/null
opt -basicaa -load Debug+Asserts/lib/slicmpass.so -lamp-inst-cnt -lamp-map-loop -lamp-load-profile -profile-loader -profile-info-file=llvmprof.out -slicmpass < $filename.ls.bc
opt -dot-cfg < $filename.bc >& /dev/null 
