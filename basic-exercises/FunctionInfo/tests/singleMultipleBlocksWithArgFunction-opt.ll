; ModuleID = 'tests/singleMultipleBlocksWithArgFunction-opt.bc'
source_filename = "tests/singleMultipleBlocksWithArgFunction.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define i32 @singleMultipleBlockWithArgFunction(i32, i32) #0 {
  %3 = icmp sgt i32 %0, 5
  br i1 %3, label %4, label %5

; <label>:4:                                      ; preds = %2
  br label %6

; <label>:5:                                      ; preds = %2
  br label %6

; <label>:6:                                      ; preds = %5, %4
  %.0 = phi i32 [ 1, %4 ], [ 0, %5 ]
  ret i32 %.0
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0-1ubuntu2~16.04.1 (tags/RELEASE_600/final)"}
