; ModuleID = '583simple/src/simple.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@stdout = external global %struct._IO_FILE*
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %in = alloca [1000 x i32], align 16
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %myfile = alloca %struct._IO_FILE*, align 8
  store i32 0, i32* %retval
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4
  %cmp = icmp slt i32 %0, 1000
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32* %i, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [1000 x i32]* %in, i32 0, i64 %idxprom
  store i32 0, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %2 = load i32* %i, align 4
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 100, i32* %j, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc6, %for.end
  %3 = load i32* %j, align 4
  %cmp2 = icmp slt i32 %3, 1000
  br i1 %cmp2, label %for.body3, label %for.end8

for.body3:                                        ; preds = %for.cond1
  %4 = load i32* %j, align 4
  %idxprom4 = sext i32 %4 to i64
  %arrayidx5 = getelementptr inbounds [1000 x i32]* %in, i32 0, i64 %idxprom4
  %5 = load i32* %arrayidx5, align 4
  %add = add nsw i32 %5, 10
  store i32 %add, i32* %arrayidx5, align 4
  br label %for.inc6

for.inc6:                                         ; preds = %for.body3
  %6 = load i32* %j, align 4
  %inc7 = add nsw i32 %6, 1
  store i32 %inc7, i32* %j, align 4
  br label %for.cond1

for.end8:                                         ; preds = %for.cond1
  store i32 0, i32* %i, align 4
  br label %for.cond9

for.cond9:                                        ; preds = %for.inc14, %for.end8
  %7 = load i32* %i, align 4
  %cmp10 = icmp slt i32 %7, 1000
  br i1 %cmp10, label %for.body11, label %for.end16

for.body11:                                       ; preds = %for.cond9
  %8 = load %struct._IO_FILE** @stdout, align 8
  %9 = load i32* %i, align 4
  %idxprom12 = sext i32 %9 to i64
  %arrayidx13 = getelementptr inbounds [1000 x i32]* %in, i32 0, i64 %idxprom12
  %10 = load i32* %arrayidx13, align 4
  %call = call i32 (%struct._IO_FILE*, i8*, ...)* @fprintf(%struct._IO_FILE* %8, i8* getelementptr inbounds ([4 x i8]* @.str, i32 0, i32 0), i32 %10)
  br label %for.inc14

for.inc14:                                        ; preds = %for.body11
  %11 = load i32* %i, align 4
  %inc15 = add nsw i32 %11, 1
  store i32 %inc15, i32* %i, align 4
  br label %for.cond9

for.end16:                                        ; preds = %for.cond9
  ret i32 1
}

declare i32 @fprintf(%struct._IO_FILE*, i8*, ...) #1

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
