# mini C compiler

Under development...

## Goals

* Being able to compile the *tests/test.txt* **C** code into assembly code. The target will be RISC-V or ARM64.

* Implement few target independent optimizations. 

## Build
```
mkdir build
cd build
cmake ../
make
```

## Usage

##### AST dumping
```
miniCC ../tests/test.txt -dump-ast
```
Where test.txt is
```c
int g;

int foo(int a, int b) {
	return g = a - b;
}

int bar () {
	int i;
	double real;
	int arr[10];


	bar();

	real = 0.0;
	if (g == 0)
		real = foo(1,2.0);
	else
		g = 0;
	
	i = 10;
	while (i > 0 && real != 0.0) {
		arr[i] = real = 3.0 * (i % 20);
		i = i + 1;
	}
	
	return arr[5] & 3;
}
```

Output:
```
TranslationUnit
  VariableDeclaration 'int' 'g'
  FunctionDeclaration 'int (int,int)' 'foo'
    FunctionParameterDeclaration 'int' 'a'
    FunctionParameterDeclaration 'int' 'b'
    CompoundStatement
      ReturnStatement
        BinaryExpression 'int' '='
          ReferenceExpression 'int' 'g'
          BinaryExpression 'int' '-'
            ReferenceExpression 'int' 'a'
            ReferenceExpression 'int' 'b'
  FunctionDeclaration 'int (void)' 'bar'
    CompoundStatement
      VariableDeclaration 'int' 'i'
      VariableDeclaration 'double' 'real'
      VariableDeclaration 'int[10]' 'arr'
      ExpressionStatement
        CallExpression 'int (void)' 'bar'
      ExpressionStatement
        BinaryExpression 'double' '='
          ReferenceExpression 'double' 'real'
          FloatLiteralExpression 'double' '0.000000'
      IfStatement
        BinaryExpression 'int' '=='
          ReferenceExpression 'int' 'g'
          IntegerLiteralExpression 'int' '0'
        ExpressionStatement
          BinaryExpression 'double' '='
            ReferenceExpression 'double' 'real'
            ImplicitCastExpression 'double'
              CallExpression 'int (int,int)' 'foo'
                IntegerLiteralExpression 'int' '1'
                ImplicitCastExpression 'int'
                  FloatLiteralExpression 'double' '2.000000'
        ExpressionStatement
          BinaryExpression 'int' '='
            ReferenceExpression 'int' 'g'
            IntegerLiteralExpression 'int' '0'
      ExpressionStatement
        BinaryExpression 'int' '='
          ReferenceExpression 'int' 'i'
          IntegerLiteralExpression 'int' '10'
      WhileStatement
        BinaryExpression 'int' '&&'
          BinaryExpression 'int' '>'
            ReferenceExpression 'int' 'i'
            IntegerLiteralExpression 'int' '0'
          BinaryExpression 'int' '!='
            ReferenceExpression 'double' 'real'
            FloatLiteralExpression 'double' '0.000000'
        CompoundStatement
          ExpressionStatement
            BinaryExpression 'int' '='
              ArrayExpression 'int' 'arr'
                ReferenceExpression 'int' 'i'
              ImplicitCastExpression 'int'
                BinaryExpression 'double' '='
                  ReferenceExpression 'double' 'real'
                  BinaryExpression 'double' '*'
                    FloatLiteralExpression 'double' '3.000000'
                    ImplicitCastExpression 'double'
                      BinaryExpression 'int' '%'
                        ReferenceExpression 'int' 'i'
                        IntegerLiteralExpression 'int' '20'
          ExpressionStatement
            BinaryExpression 'int' '='
              ReferenceExpression 'int' 'i'
              BinaryExpression 'int' '+'
                ReferenceExpression 'int' 'i'
                IntegerLiteralExpression 'int' '1'
      ReturnStatement
        BinaryExpression 'int' '&'
          ArrayExpression 'int' 'arr'
            IntegerLiteralExpression 'int' '5'
          IntegerLiteralExpression 'int' '3'
```
