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
try "12" "10+2;"
try "13" "10 + 3;"
try "7" "10 - 3;"
try "12" "4 * 3;"
try "2" "8 / 4;"
try "1" "3 == 3;"
try "0" "3 == 2;"
try "0" "3 != 3;"
try "1" "3 != 2;"
try "1" "3 >= 1;"
try "0" "0 >= 1;"
try "0" "3 <= 2;"
try "1" "3 <= 5;"
try "1" "3 > 1;"
try "0" "0 > 0;"
try "0" "3 < 3;"
try "1" "3 < 5;"
try "23" "22 + 4 - 3;"
try "47" "5 + 6 * 7;"
try "15" "5 * (9 - 6);"
try "10" "-10+10*2;"
try "4" "(3+5) /2;"
try "3" "(3 == 3) + 2;"
try "4" "a=3;a+1;"
try "11" "b=5*6-8;return b/2;"
try "10" "a=b=5;a+b;"
try "5" "a=3;b=2;return a+b;"
try "5" "ab=3;dvd=2;return ab+dvd;"
try "2" "a=0;while (a<2) a=a+1;return a;"
try "2" "a=0; for(i=0; i<2; i=i+1) a = a + 1;return a;"
try "5" "a=3;if(a==3) return a+2;"
try "6" "a=3;if (a==2) return 2;else return a+3;"
try "5" "a=2;if (a==2) {a=a+3; return a;}"
try "5" "a=2;b=3;if (a==2) {a=a+b; return a;}"

echo OK
