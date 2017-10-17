; ModuleID = '583wc/src/wc.licm.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@copyright = global [88 x i8] c"@(#) Copyright (c) 1980 Regents of the University of California.\0A All rights reserved.\0A\00", align 16
@.str = private unnamed_addr constant [4 x i8] c"lwc\00", align 1
@wd = global i8* getelementptr inbounds ([4 x i8]* @.str, i32 0, i32 0), align 8
@buffer = internal global [8192 x i8] zeroinitializer, align 16
@bufcount = internal global i32 0, align 4
@bufcurrent = internal global i32 0, align 4
@main.x = internal global i32 0, align 4
@main.y = internal global i32 0, align 4
@main.z = internal global i32 0, align 4
@stderr = external global %struct._IO_FILE*
@.str1 = private unnamed_addr constant [26 x i8] c"Usage: wc [-lwc] [files]\0A\00", align 1
@stdin = external global %struct._IO_FILE*
@.str2 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@linect = common global i64 0, align 8
@wordct = common global i64 0, align 8
@charct = common global i64 0, align 8
@last_charct = common global i64 0, align 8
@wordct_array = common global [25 x i64] zeroinitializer, align 16
@.str3 = private unnamed_addr constant [5 x i8] c" %s\0A\00", align 1
@.str4 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@tlinect = common global i64 0, align 8
@twordct = common global i64 0, align 8
@tcharct = common global i64 0, align 8
@.str5 = private unnamed_addr constant [8 x i8] c" total\0A\00", align 1
@.str6 = private unnamed_addr constant [21 x i8] c"Word size histogran\0A\00", align 1
@.str7 = private unnamed_addr constant [9 x i8] c"%d: %ld\0A\00", align 1
@.str8 = private unnamed_addr constant [6 x i8] c" %7ld\00", align 1
@pagect = common global i64 0, align 8
@tpagect = common global i64 0, align 8

; Function Attrs: nounwind uwtable
define void @my_filbuf(%struct._IO_FILE* %F) #0 {
entry:
  %F.addr = alloca %struct._IO_FILE*, align 8
  store %struct._IO_FILE* %F, %struct._IO_FILE** %F.addr, align 8
  %0 = load %struct._IO_FILE** %F.addr, align 8
  %call = call i64 @fread(i8* getelementptr inbounds ([8192 x i8]* @buffer, i32 0, i32 0), i64 1, i64 8192, %struct._IO_FILE* %0)
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* @bufcount, align 4
  store i32 0, i32* @bufcurrent, align 4
  %1 = load %struct._IO_FILE** %F.addr, align 8
  %call1 = call i32 @feof(%struct._IO_FILE* %1) #4
  %tobool = icmp ne i32 %call1, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %2 = load i32* @bufcount, align 4
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32* @bufcount, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [8192 x i8]* @buffer, i32 0, i64 %idxprom
  store i8 -1, i8* %arrayidx, align 1
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

declare i64 @fread(i8*, i64, i64, %struct._IO_FILE*) #1

; Function Attrs: nounwind
declare i32 @feof(%struct._IO_FILE*) #2

; Function Attrs: nounwind uwtable
define i32 @main(i32 %argc, i8** %argv) #0 {
entry:
  %retval = alloca i32, align 4
  %argc.addr = alloca i32, align 4
  %argv.addr = alloca i8**, align 8
  %i = alloca i32, align 4
  %token = alloca i32, align 4
  %fp = alloca %struct._IO_FILE*, align 8
  %c = alloca i32, align 4
  %p = alloca i8*, align 8
  store i32 0, i32* %retval
  store i32 %argc, i32* %argc.addr, align 4
  store i8** %argv, i8*** %argv.addr, align 8
  br label %while.cond

while.cond:                                       ; preds = %sw.epilog, %entry
  %0 = load i32* %argc.addr, align 4
  %cmp = icmp sgt i32 %0, 1
  br i1 %cmp, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %while.cond
  %1 = load i8*** %argv.addr, align 8
  %arrayidx = getelementptr inbounds i8** %1, i64 1
  %2 = load i8** %arrayidx, align 8
  %3 = load i8* %2, align 1
  %conv = sext i8 %3 to i32
  %cmp1 = icmp eq i32 %conv, 45
  br label %land.end

land.end:                                         ; preds = %land.rhs, %while.cond
  %4 = phi i1 [ false, %while.cond ], [ %cmp1, %land.rhs ]
  br i1 %4, label %while.body, label %while.end

while.body:                                       ; preds = %land.end
  %5 = load i8*** %argv.addr, align 8
  %arrayidx3 = getelementptr inbounds i8** %5, i64 1
  %6 = load i8** %arrayidx3, align 8
  %arrayidx4 = getelementptr inbounds i8* %6, i64 1
  %7 = load i8* %arrayidx4, align 1
  %conv5 = sext i8 %7 to i32
  switch i32 %conv5, label %sw.default [
    i32 108, label %sw.bb
    i32 119, label %sw.bb
    i32 99, label %sw.bb
  ]

sw.bb:                                            ; preds = %while.body, %while.body, %while.body
  %8 = load i8*** %argv.addr, align 8
  %arrayidx6 = getelementptr inbounds i8** %8, i64 1
  %9 = load i8** %arrayidx6, align 8
  %add.ptr = getelementptr inbounds i8* %9, i64 1
  store i8* %add.ptr, i8** @wd, align 8
  br label %sw.epilog

sw.default:                                       ; preds = %while.body
  br label %usage

usage:                                            ; preds = %sw.default
  %10 = load %struct._IO_FILE** @stderr, align 8
  %call = call i32 (%struct._IO_FILE*, i8*, ...)* @fprintf(%struct._IO_FILE* %10, i8* getelementptr inbounds ([26 x i8]* @.str1, i32 0, i32 0))
  call void @exit(i32 1) #5
  unreachable

sw.epilog:                                        ; preds = %sw.bb
  %11 = load i32* %argc.addr, align 4
  %dec = add nsw i32 %11, -1
  store i32 %dec, i32* %argc.addr, align 4
  %12 = load i8*** %argv.addr, align 8
  %incdec.ptr = getelementptr inbounds i8** %12, i32 1
  store i8** %incdec.ptr, i8*** %argv.addr, align 8
  br label %while.cond

while.end:                                        ; preds = %land.end
  store i32 1, i32* %i, align 4
  %13 = load %struct._IO_FILE** @stdin, align 8
  store %struct._IO_FILE* %13, %struct._IO_FILE** %fp, align 8
  %14 = load i32* %argc.addr, align 4
  %cmp7 = icmp sgt i32 %14, 1
  %15 = load i8*** %argv.addr, align 8
  %16 = load i8*** %argv.addr, align 8
  %17 = load i32* %argc.addr, align 4
  %18 = load i32* %argc.addr, align 4
  %cmp71 = icmp sgt i32 %18, 1
  %19 = load i8*** %argv.addr, align 8
  br label %do.body

do.body:                                          ; preds = %do.cond, %while.end
  br i1 %cmp7, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %do.body
  %20 = load i32* %i, align 4
  %idxprom = sext i32 %20 to i64
  %arrayidx9 = getelementptr inbounds i8** %15, i64 %idxprom
  %21 = load i8** %arrayidx9, align 8
  %call10 = call %struct._IO_FILE* @fopen(i8* %21, i8* getelementptr inbounds ([2 x i8]* @.str2, i32 0, i32 0))
  store %struct._IO_FILE* %call10, %struct._IO_FILE** %fp, align 8
  %cmp11 = icmp eq %struct._IO_FILE* %call10, null
  br i1 %cmp11, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  %22 = load i32* %i, align 4
  %idxprom13 = sext i32 %22 to i64
  %arrayidx14 = getelementptr inbounds i8** %16, i64 %idxprom13
  %23 = load i8** %arrayidx14, align 8
  call void @perror(i8* %23)
  br label %do.cond

if.end:                                           ; preds = %land.lhs.true, %do.body
  store i64 0, i64* @linect, align 8
  store i64 0, i64* @wordct, align 8
  store i64 0, i64* @charct, align 8
  store i32 0, i32* %token, align 4
  store i64 0, i64* @last_charct, align 8
  %24 = load %struct._IO_FILE** %fp, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.cond.backedge, %if.end
  %25 = load i32* @bufcurrent, align 4
  %26 = load i32* @bufcount, align 4
  %cmp15 = icmp sge i32 %25, %26
  br i1 %cmp15, label %cond.true, label %cond.false

cond.true:                                        ; preds = %for.cond
  call void @my_filbuf(%struct._IO_FILE* %24)
  %27 = load i32* @bufcurrent, align 4
  %inc = add nsw i32 %27, 1
  store i32 %inc, i32* @bufcurrent, align 4
  %idxprom17 = sext i32 %27 to i64
  %arrayidx18 = getelementptr inbounds [8192 x i8]* @buffer, i32 0, i64 %idxprom17
  %28 = load i8* %arrayidx18, align 1
  %conv19 = sext i8 %28 to i32
  br label %cond.end

cond.false:                                       ; preds = %for.cond
  %29 = load i32* @bufcurrent, align 4
  %inc20 = add nsw i32 %29, 1
  store i32 %inc20, i32* @bufcurrent, align 4
  %idxprom21 = sext i32 %29 to i64
  %arrayidx22 = getelementptr inbounds [8192 x i8]* @buffer, i32 0, i64 %idxprom21
  %30 = load i8* %arrayidx22, align 1
  %conv23 = sext i8 %30 to i32
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %conv19, %cond.true ], [ %conv23, %cond.false ]
  %cmp24 = icmp eq i32 %cond, -1
  br i1 %cmp24, label %if.then26, label %if.end27

if.then26:                                        ; preds = %cond.end
  store i32 %cond, i32* %c, align 4
  br label %for.end

if.end27:                                         ; preds = %cond.end
  %31 = load i64* @charct, align 8
  %inc28 = add nsw i64 %31, 1
  store i64 %inc28, i64* @charct, align 8
  %32 = load i64* @last_charct, align 8
  %inc29 = add nsw i64 %32, 1
  store i64 %inc29, i64* @last_charct, align 8
  %cmp30 = icmp slt i32 32, %cond
  br i1 %cmp30, label %land.lhs.true32, label %if.end47

land.lhs.true32:                                  ; preds = %if.end27
  %cmp33 = icmp slt i32 %cond, 127
  br i1 %cmp33, label %if.then35, label %if.end47

if.then35:                                        ; preds = %land.lhs.true32
  %33 = load i32* %token, align 4
  %tobool = icmp ne i32 %33, 0
  br i1 %tobool, label %if.end46, label %if.then36

if.then36:                                        ; preds = %if.then35
  %34 = load i64* @wordct, align 8
  %inc37 = add nsw i64 %34, 1
  store i64 %inc37, i64* @wordct, align 8
  %35 = load i32* %token, align 4
  %inc38 = add nsw i32 %35, 1
  store i32 %inc38, i32* %token, align 4
  %36 = load i64* @last_charct, align 8
  %cmp39 = icmp sge i64 %36, 25
  br i1 %cmp39, label %if.then41, label %if.else

if.then41:                                        ; preds = %if.then36
  %37 = load i64* getelementptr inbounds ([25 x i64]* @wordct_array, i32 0, i64 24), align 8
  %inc42 = add nsw i64 %37, 1
  store i64 %inc42, i64* getelementptr inbounds ([25 x i64]* @wordct_array, i32 0, i64 24), align 8
  br label %if.end45

if.else:                                          ; preds = %if.then36
  %38 = load i64* @last_charct, align 8
  %arrayidx43 = getelementptr inbounds [25 x i64]* @wordct_array, i32 0, i64 %38
  %39 = load i64* %arrayidx43, align 8
  %inc44 = add nsw i64 %39, 1
  store i64 %inc44, i64* %arrayidx43, align 8
  br label %if.end45

if.end45:                                         ; preds = %if.else, %if.then41
  store i64 0, i64* @last_charct, align 8
  br label %if.end46

if.end46:                                         ; preds = %if.end45, %if.then35
  br label %for.cond.backedge

for.cond.backedge:                                ; preds = %if.end70, %if.then68, %if.end46
  br label %for.cond

if.end47:                                         ; preds = %land.lhs.true32, %if.end27
  %cmp48 = icmp eq i32 %cond, 10
  br i1 %cmp48, label %if.then50, label %if.else62

if.then50:                                        ; preds = %if.end47
  %40 = load i64* @linect, align 8
  %inc51 = add nsw i64 %40, 1
  store i64 %inc51, i64* @linect, align 8
  %41 = load i64* @linect, align 8
  %cmp52 = icmp sgt i64 %41, 20
  br i1 %cmp52, label %land.lhs.true54, label %if.else59

land.lhs.true54:                                  ; preds = %if.then50
  %42 = load i64* @last_charct, align 8
  %cmp55 = icmp sgt i64 %42, 10
  br i1 %cmp55, label %if.then57, label %if.else59

if.then57:                                        ; preds = %land.lhs.true54
  %43 = load i64* @linect, align 8
  %div = sdiv i64 %43, 5
  %conv58 = trunc i64 %div to i32
  store i32 %conv58, i32* @main.x, align 4
  %44 = load i32* @main.x, align 4
  %mul = mul nsw i32 %44, 98
  store i32 %mul, i32* @main.y, align 4
  %45 = load i32* @main.y, align 4
  %rem = srem i32 %45, 23
  store i32 %rem, i32* @main.z, align 4
  br label %if.end61

if.else59:                                        ; preds = %land.lhs.true54, %if.then50
  %46 = load i32* @main.z, align 4
  %inc60 = add nsw i32 %46, 1
  store i32 %inc60, i32* @main.z, align 4
  br label %if.end61

if.end61:                                         ; preds = %if.else59, %if.then57
  br label %if.end70

if.else62:                                        ; preds = %if.end47
  %cmp63 = icmp ne i32 %cond, 32
  br i1 %cmp63, label %land.lhs.true65, label %if.end69

land.lhs.true65:                                  ; preds = %if.else62
  %cmp66 = icmp ne i32 %cond, 9
  br i1 %cmp66, label %if.then68, label %if.end69

if.then68:                                        ; preds = %land.lhs.true65
  br label %for.cond.backedge

if.end69:                                         ; preds = %land.lhs.true65, %if.else62
  br label %if.end70

if.end70:                                         ; preds = %if.end69, %if.end61
  store i32 0, i32* %token, align 4
  br label %for.cond.backedge

for.end:                                          ; preds = %if.then26
  %47 = load i8** @wd, align 8
  %48 = load i64* @charct, align 8
  %49 = load i64* @wordct, align 8
  %50 = load i64* @linect, align 8
  call void @wcp(i8* %47, i64 %48, i64 %49, i64 %50, i64* getelementptr inbounds ([25 x i64]* @wordct_array, i32 0, i32 0))
  br i1 %cmp71, label %if.then73, label %if.else77

if.then73:                                        ; preds = %for.end
  %51 = load i32* %i, align 4
  %idxprom74 = sext i32 %51 to i64
  %arrayidx75 = getelementptr inbounds i8** %19, i64 %idxprom74
  %52 = load i8** %arrayidx75, align 8
  %call76 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([5 x i8]* @.str3, i32 0, i32 0), i8* %52)
  br label %if.end79

if.else77:                                        ; preds = %for.end
  %call78 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([2 x i8]* @.str4, i32 0, i32 0))
  br label %if.end79

if.end79:                                         ; preds = %if.else77, %if.then73
  %53 = load %struct._IO_FILE** %fp, align 8
  %call80 = call i32 @fclose(%struct._IO_FILE* %53)
  %54 = load i64* @linect, align 8
  %55 = load i64* @tlinect, align 8
  %add = add nsw i64 %55, %54
  store i64 %add, i64* @tlinect, align 8
  %56 = load i64* @wordct, align 8
  %57 = load i64* @twordct, align 8
  %add81 = add nsw i64 %57, %56
  store i64 %add81, i64* @twordct, align 8
  %58 = load i64* @charct, align 8
  %59 = load i64* @tcharct, align 8
  %add82 = add nsw i64 %59, %58
  store i64 %add82, i64* @tcharct, align 8
  br label %do.cond

do.cond:                                          ; preds = %if.end79, %if.then
  %60 = load i32* %i, align 4
  %inc83 = add nsw i32 %60, 1
  store i32 %inc83, i32* %i, align 4
  %cmp84 = icmp slt i32 %inc83, %17
  br i1 %cmp84, label %do.body, label %do.end

do.end:                                           ; preds = %do.cond
  %61 = load i32* %argc.addr, align 4
  %cmp86 = icmp sgt i32 %61, 2
  br i1 %cmp86, label %if.then88, label %if.end90

if.then88:                                        ; preds = %do.end
  %62 = load i8** @wd, align 8
  %63 = load i64* @tcharct, align 8
  %64 = load i64* @twordct, align 8
  %65 = load i64* @tlinect, align 8
  call void @wcp(i8* %62, i64 %63, i64 %64, i64 %65, i64* getelementptr inbounds ([25 x i64]* @wordct_array, i32 0, i32 0))
  %call89 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([8 x i8]* @.str5, i32 0, i32 0))
  br label %if.end90

if.end90:                                         ; preds = %if.then88, %do.end
  call void @exit(i32 0) #5
  unreachable

return:                                           ; No predecessors!
  %66 = load i32* %retval
  ret i32 %66
}

declare i32 @fprintf(%struct._IO_FILE*, i8*, ...) #1

; Function Attrs: noreturn nounwind
declare void @exit(i32) #3

declare %struct._IO_FILE* @fopen(i8*, i8*) #1

declare void @perror(i8*) #1

; Function Attrs: nounwind uwtable
define void @wcp(i8* %wd, i64 %charct, i64 %wordct, i64 %linect, i64* %wordct_array) #0 {
entry:
  %wd.addr = alloca i8*, align 8
  %charct.addr = alloca i64, align 8
  %wordct.addr = alloca i64, align 8
  %linect.addr = alloca i64, align 8
  %wordct_array.addr = alloca i64*, align 8
  %i = alloca i32, align 4
  store i8* %wd, i8** %wd.addr, align 8
  store i64 %charct, i64* %charct.addr, align 8
  store i64 %wordct, i64* %wordct.addr, align 8
  store i64 %linect, i64* %linect.addr, align 8
  store i64* %wordct_array, i64** %wordct_array.addr, align 8
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([21 x i8]* @.str6, i32 0, i32 0))
  store i32 0, i32* %i, align 4
  %0 = load i64** %wordct_array.addr, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32* %i, align 4
  %cmp = icmp slt i32 %1, 25
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32* %i, align 4
  %3 = load i32* %i, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds i64* %0, i64 %idxprom
  %4 = load i64* %arrayidx, align 8
  %call1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([9 x i8]* @.str7, i32 0, i32 0), i32 %2, i64 %4)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %5 = load i32* %i, align 4
  %inc = add nsw i32 %5, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %6 = load i64* %linect.addr, align 8
  %7 = load i64* %wordct.addr, align 8
  %8 = load i64* %charct.addr, align 8
  br label %while.cond

while.cond:                                       ; preds = %sw.epilog, %for.end
  %9 = load i8** %wd.addr, align 8
  %10 = load i8* %9, align 1
  %tobool = icmp ne i8 %10, 0
  br i1 %tobool, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %11 = load i8** %wd.addr, align 8
  %incdec.ptr = getelementptr inbounds i8* %11, i32 1
  store i8* %incdec.ptr, i8** %wd.addr, align 8
  %12 = load i8* %11, align 1
  %conv = sext i8 %12 to i32
  switch i32 %conv, label %sw.epilog [
    i32 108, label %sw.bb
    i32 119, label %sw.bb2
    i32 99, label %sw.bb3
  ]

sw.bb:                                            ; preds = %while.body
  call void @ipr(i64 %6)
  br label %sw.epilog

sw.bb2:                                           ; preds = %while.body
  call void @ipr(i64 %7)
  br label %sw.epilog

sw.bb3:                                           ; preds = %while.body
  call void @ipr(i64 %8)
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.bb3, %sw.bb2, %sw.bb, %while.body
  br label %while.cond

while.end:                                        ; preds = %while.cond
  ret void
}

declare i32 @printf(i8*, ...) #1

declare i32 @fclose(%struct._IO_FILE*) #1

; Function Attrs: nounwind uwtable
define void @ipr(i64 %num) #0 {
entry:
  %num.addr = alloca i64, align 8
  store i64 %num, i64* %num.addr, align 8
  %0 = load i64* %num.addr, align 8
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([6 x i8]* @.str8, i32 0, i32 0), i64 %0)
  ret void
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { noreturn nounwind }
