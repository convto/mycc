#!/bin/bash
try() {
	expected="$1"
	input="$2"

	./mycc "$input" > tmp.s
	gcc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" == "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

try 0 'main(){0;}'
try 42 'main(){42;}'
try 21 'main(){5+20-4;}'
try 41 'main(){ 12 + 34 - 5 ;}'
try 47 'main(){ 5+6*7;}'
try 15 'main(){5*(9-6);}'
try 4 'main(){(3+5)/2;}'
try 10 'main(){-10+20;}'
try 1 'main(){1==1;}'
try 1 'main(){1!=10;}'
try 1 'main(){1 < 2;}'
try 1 'main(){1 <= 1;}'
try 1 'main(){2 > 1;}'
try 1 'main(){1 >= 1;}'
try 0 'main(){((3 + 6) * 2) == 8 / (2 + 2);}'
try 12 'main(){a = 3; a * 4;}'
try 12 'main(){(40 - 2 * 2) / (1 + 2);}'
try 1 'main(){12 == (40 - 2 * 2) / (1 + 2);}'
try 1 'main(){a = 3; b = 4; a * b == (40 - 2 * 2) / (1 + 2);}'
try 6 'main(){foo = 3; bar = 3; foo + bar;}'
try 12 'main(){a = 3; b = 4; return a * b; return a + b;}'
try 10 'main(){if (1 < 2) return 10;}'
try 10 'main(){if (1 > 2) return 20; else return 10;}'
try 10 'main(){i = 0; while (i <= 9) i = i + 1; return i;}'
try 0 'main(){for (i = 0; i <= 9; i = i + 1) return i;}'
try 10 'main(){ for (i = 0; i <= 9; i = i + 1) if (i == 10) return i;}'
try 10 'main(){ return 1 + 9; }'
try 20 'main(){for (i = 0; i < 10; i = i + 1) { i = i + 9; i = i + 10; } return i;}'
try 10 'main(){ return foo(); } foo(){ f = 6; oo = 4; return f + oo; }'
try 10 'main(){ foo = 3; bar = 7; foobar(foo, bar); } foobar(foo, bar) { foo + bar; }'
try 55 'main() { return fib(10); } fib(n) { if (n == 0) { return 0; } if (n == 1) { return 1; } return fib(n - 1) + fib(n - 2); }'
try 55 'sum(m, n) { acc = 0; for (i = m; i <= n; i = i + 1) acc = acc + i; return acc; } main() { return sum(1, 10); }'
echo OK
