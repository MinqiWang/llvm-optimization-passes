; ModuleID = 'tests/multipleFunctions-opt.bc'
source_filename = "tests/multipleFunctions.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define i32 @notBeingCalled(i32) #0 {
  %2 = add nsw i32 %0, 1
  ret i32 %2
}

; Function Attrs: noinline nounwind uwtable
define i32 @beingCalledOnce(i32) #0 {
  %2 = mul nsw i32 2, %0
  %3 = sub nsw i32 %2, 1
  ret i32 %3
}

; Function Attrs: noinline nounwind uwtable
define i32 @beingCalledMultiTimes(i32, i32) #0 {
  %3 = add nsw i32 %0, %1
  ret i32 %3
}

; Function Attrs: noinline nounwind uwtable
define i32 @caller(i32, i32) #0 {
  %3 = call i32 @beingCalledOnce(i32 %0)
  %4 = call i32 @beingCalledMultiTimes(i32 %0, i32 %1)
  %5 = call i32 @beingCalledMultiTimes(i32 %0, i32 %1)
  %6 = call i32 @beingCalledMultiTimes(i32 %0, i32 %1)
  ret i32 0
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0-1ubuntu2~16.04.1 (tags/RELEASE_600/final)"}
