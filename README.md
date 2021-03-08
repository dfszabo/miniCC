# mini C compiler

Under development...

## Goals

* Being able to compile simple **C** functions like *tests/frontend/algorithm-gcd.c* into assembly code. The target will be ARM64, but further targets planed like RISC-V. (DONE)

* Being able to compile my tetris applications main parts.

* Implement few target independent optimizations. 

## Build
```
mkdir build
cd build
cmake ../
make
```

## Usage

AST dumping
```
miniCC ../tests/fronted/algorithm-gcd.c -dump-ast
```
Where **algorithm-gcd.c** is
```c
int gcd(int a, int b) {
  int R;

  while ((a % b) > 0) {
    R = a % b;
    a = b;
    b = R;
  }

  return b;
}

```

Output:
```
TranslationUnit
  FunctionDeclaration 'int (int,int)' 'gcd'
    FunctionParameterDeclaration 'int' 'a'
    FunctionParameterDeclaration 'int' 'b'
    CompoundStatement
      VariableDeclaration 'int' 'R'
      WhileStatement
        BinaryExpression 'int' '>'
          BinaryExpression 'int' '%'
            ReferenceExpression 'int' 'a'
            ReferenceExpression 'int' 'b'
          IntegerLiteralExpression 'int' '0'
        CompoundStatement
          ExpressionStatement
            BinaryExpression 'int' '='
              ReferenceExpression 'int' 'R'
              BinaryExpression 'int' '%'
                ReferenceExpression 'int' 'a'
                ReferenceExpression 'int' 'b'
          ExpressionStatement
            BinaryExpression 'int' '='
              ReferenceExpression 'int' 'a'
              ReferenceExpression 'int' 'b'
          ExpressionStatement
            BinaryExpression 'int' '='
              ReferenceExpression 'int' 'b'
              ReferenceExpression 'int' 'R'
      ReturnStatement
        ReferenceExpression 'int' 'b'
```
IR dumping
```
miniCC ../tests/fronted/algorithm-gcd.c -dump-ir
```
Output:
```
func gcd ($a :i32, $b :i32) -> i32:
.entry_gcd:
        sa      $0 :i32
        sa      $1 :i32
        sa      $2 :i32
        str     [$0], $a
        str     [$1], $b
        j       <loop_header0>
.loop_header0:
        ld      $3, [$0]
        ld      $4, [$1]
        mod     $5, $3, $4
        cmp.le  $6, $5, 0
        br      $6, <loop_end0>
.loop_body0:
        ld      $7, [$0]
        ld      $8, [$1]
        mod     $9, $7, $8
        str     [$2], $9
        ld      $10, [$1]
        str     [$0], $10
        ld      $11, [$2]
        str     [$1], $11
        j       <loop_header0>
.loop_end0:
        ld      $12, [$1]
        ret     $12
```
Generating assembly

The default architecture is AArch64. It can be changed using `arch` option like `-arch=riscv`. **NOTE**: RISC-V code was not checked, only AArch64 with qemu.
```
miniCC ../tests/fronted/algorithm-gcd.c
```
Output:
```
.globl  gcd
gcd:
        sub     sp, sp, #16
        str     w0, [sp, #12]
        str     w1, [sp, #8]
        b       .Lloop_header0
.Lloop_header0:
        ldr     w2, [sp, #12]
        ldr     w3, [sp, #8]
        sdiv    w4, w2, w3
        mul     w4, w4, w3
        sub     w5, w2, w4
        cmp     w5, #0
        b.le    .Lloop_end0
.Lloop_body0:
        ldr     w6, [sp, #12]
        ldr     w7, [sp, #8]
        sdiv    w9, w6, w7
        mul     w10, w9, w7
        sub     w11, w6, w10
        str     w11, [sp, #4]
        ldr     w12, [sp, #8]
        str     w12, [sp, #12]
        ldr     w13, [sp, #4]
        str     w13, [sp, #8]
        b       .Lloop_header0
.Lloop_end0:
        ldr     w0, [sp, #8]
        add     sp, sp, #16
        ret
```
```
miniCC ../tests/fronted/algorithm-gcd.c -arch=riscv
```
Output:
```
.globl  gcd
gcd:
        addi    sp, sp, -16
        sw      a0, 12(sp)
        sw      a1, 8(sp)
        j       .Lloop_header0
.Lloop_header0:
        lw      a2, 12(sp)
        lw      a3, 8(sp)
        rem     a4, a2, a3
        slti    a5, a4, 0
        bnez    a5, .Lloop_end0
.Lloop_body0:
        lw      a6, 12(sp)
        lw      a7, 8(sp)
        rem     t0, a6, a7
        sw      t0, 4(sp)
        lw      t1, 8(sp)
        sw      t1, 12(sp)
        lw      t2, 4(sp)
        sw      t2, 8(sp)
        j       .Lloop_header0
.Lloop_end0:
        lw      a0, 8(sp)
        addi    sp, sp, 16
        ret
```
