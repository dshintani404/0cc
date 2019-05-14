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

try "0" "main(){0;}"
try "12" "main(){10+2;}"
try "13" "main(){10 + 3;}"
try "7" "main(){10 - 3;}"
try "12" "main(){4 * 3;}"
try "2" "main(){8 / 4;}"
try "1" "main(){3 == 3;}"
try "0" "main(){3 == 2;}"
try "0" "main(){3 != 3;}"
try "1" "main(){3 != 2;}"
try "1" "main(){3 >= 1;}"
try "0" "main(){0 >= 1;}"
try "0" "main(){3 <= 2;}"
try "1" "main(){3 <= 5;}"
try "1" "main(){3 > 1;}"
try "0" "main(){0 > 0;}"
try "0" "main(){3 < 3;}"
try "1" "main(){3 < 5;}"
try "23" "main(){22 + 4 - 3;}"
try "47" "main(){5 + 6 * 7;}"
try "15" "main(){5 * (9 - 6);}"
try "10" "main(){-10+10*2;}"
try "4" "main(){(3+5) /2;}"
try "3" "main(){(3 == 3) + 2;}"
try "4" "main(){a=3;a+1;}"
try "11" "main(){b=5*6-8;return b/2;}"
try "10" "main(){a=b=5;a+b;}"
try "5" "main(){a=3;b=2;return a+b;}"
try "5" "main(){ab=3;dvd=2;return ab+dvd;}"
try "2" "main(){a=0;while (a<2) a=a+1;return a;}"
try "2" "main(){a=0; for(i=0; i<2; i=i+1) a = a + 1;return a;}"
try "5" "main(){a=3;if(a==3) return a+2;}"
try "6" "main(){a=3;if (a==2) return 2;else return a+3;}"
try "5" "main(){a=2;if (a==2) {a=a+3; return a;}}"
try "5" "main(){a=2;b=3;if (a==2) {a=a+b; return a;}}"
try "8" "bra(){return 8;} main(){return bra();}"
try "1" "bra(b){return b;} main(){a = 1;return bra(a);}"
try "2" "bra(a){return a;} main(){a = 3;return bra(a-1);}"
try "1" "bra(a){return a;} main(){a = 1;return bra(a);}"
try "5" "bra(a){return a;} main(){return bra(5);}"
try "5" "bra(a,b){return a+b;} main(){return bra(2,3);}"
try "5" "bra(a){return a;} main(){return bra(2)+bra(3);}"
try "55" "fib(a){if(a==0)return 0;if(a==1)return 1;return fib(a-1)+fib(a-2);} main(){return fib(10);}"

echo OK
