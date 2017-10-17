filename=$1
clang -emit-llvm -o $filename.bc -c $filename.c
opt -basicaa -licm $filename.bc -o $filename.licm.bc
llvm-dis $filename.bc
llvm-dis $filename.licm.bc
