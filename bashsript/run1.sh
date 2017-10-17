export PATH=$PATH:/opt/llvm/Release+Asserts/bin
./configure && make
clang -emit-llvm -o test.bc -c test.c
opt -insert-edge-profiling test.bc -o test.profile.bc
llc test.profile.bc -o test.profile.s
g++ -o test.profile test.profile.s /opt/llvm/Release+Asserts/lib/libprofile_rt.so
./test.profile
opt -f -load Debug+Asserts/lib/mypass.so -profile-loader -profile-info-file=llvmprof.out -mypass test.bc
