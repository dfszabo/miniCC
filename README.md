# subset of C compiler

Under development...

## Goals

* Being able to compile simple **C** functions like *tests/frontend/algorithm-gcd.c* into assembly code. The target will be ARM64, but further targets planed like RISC-V. (DONE)

* Being able to compile my tetris applications main parts. (DONE)

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
        sa      $0<*i32>
        sa      $2<*i32>
        sa      $4<*i32>
        str     [$0<*i32>], $a
        str     [$2<*i32>], $b
        j       <loop_header0>
.loop_header0:
        ld      $5<i32>, [$0<*i32>]
        ld      $6<i32>, [$2<*i32>]
        mod     $7<i32>, $5<i32>, $6<i32>
        cmp.le  $8<i1>, $7<i32>, 0
        br      $8<i1>, <loop_end0>
.loop_body0:
        ld      $9<i32>, [$0<*i32>]
        ld      $10<i32>, [$2<*i32>]
        mod     $11<i32>, $9<i32>, $10<i32>
        str     [$4<*i32>], $11<i32>
        ld      $12<i32>, [$2<*i32>]
        str     [$0<*i32>], $12<i32>
        ld      $13<i32>, [$4<*i32>]
        str     [$2<*i32>], $13<i32>
        j       <loop_header0>
.loop_end0:
        ld      $14<i32>, [$2<*i32>]
        ret     $14<i32>
```
Generating assembly

The default architecture is AArch64. It can be changed using `arch` option like `-arch=riscv`. **NOTE**: RISC-V code was not checked, only AArch64 with qemu. Also for now RISCV backend is broken and needs updating.
```
miniCC ../tests/fronted/algorithm-gcd.c
```
Output:
```
.globl  gcd
gcd:
        sub     sp, sp, #16
        str     w0, [sp, #0]
        str     w1, [sp, #4]
        b       .L0_loop_header0
.L0_loop_header0:
        ldr     w2, [sp, #0]
        ldr     w3, [sp, #4]
        sdiv    w4, w2, w3
        mul     w5, w4, w3
        sub     w3, w2, w5
        cmp     w3, #0
        b.le    .L0_loop_end0
.L0_loop_body0:
        ldr     w2, [sp, #0]
        ldr     w3, [sp, #4]
        sdiv    w4, w2, w3
        mul     w5, w4, w3
        sub     w3, w2, w5
        str     w3, [sp, #8]
        ldr     w2, [sp, #4]
        str     w2, [sp, #0]
        ldr     w2, [sp, #8]
        str     w2, [sp, #4]
        b       .L0_loop_header0
.L0_loop_end0:
        ldr     w0, [sp, #4]
        add     sp, sp, #16
        ret
```
