#!/bin/bash

try(){
	expected="$1"
	input="$2"

	./0cc "$input" > tmp.s
	gcc -o tmp tmp.s
	./tmp
	actual="$?"
	if [ "$expected" = "$actual" ]; then
		echo "$input => $actual"
	else
		echo "$expected expected, but got $actual"
		exit 1
	fi
}

try "0" "0;"
try "21" "22 - 4 + 3;"
try "47" "5 + 6 * 7;"
try "15" "5 * (9 - 6);"
try "10" "-10+10*2;"
try "4" "(3+5) /2;"
try "4" "a=3;a+1;"

echo OK

