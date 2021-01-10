# mini C compiler

Under development...

## Goals

* Being able to compile the *tests/test.c* **C** code into assembly code. The target will be ARM64, but further targets planed like RISC-V

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
