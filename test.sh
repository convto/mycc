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

try 0 '0;'
try 42 '42;'
try 21 '5+20-4;'
try 41 ' 12 + 34 - 5 ;'
try 47 '5+6*7;'
try 15 '5*(9-6);'
try 4 '(3+5)/2;'
try 10 '-10+20;'
try 1 '1==1;'
try 1 '1!=10;'
try 1 '1 < 2;'
try 1 '1 <= 1;'
try 1 '2 > 1;'
try 1 '1 >= 1;'
try 0 '((3 + 6) * 2) == 8 / (2 + 2);'
try 12 'a = 3; a * 4;'
try 12 '(40 - 2 * 2) / (1 + 2);'
try 1 '12 == (40 - 2 * 2) / (1 + 2);'
try 1 'a = 3; b = 4; a * b == (40 - 2 * 2) / (1 + 2);'
try 6 'foo = 3; bar = 3; foo + bar;'
echo OK
