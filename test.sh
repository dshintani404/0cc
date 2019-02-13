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

try 0 0
try 24 24

echo OK
