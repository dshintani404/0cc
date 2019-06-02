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

try "0" "int main(){0;}"
try "12" "int main(){10+2;}"
try "13" "int main(){10 + 3;}"
try "7" "int main(){10 - 3;}"
try "12" "int main(){4 * 3;}"
try "2" "int main(){8 / 4;}"
try "1" "int main(){3 == 3;}"
try "0" "int main(){3 == 2;}"
try "0" "int main(){3 != 3;}"
try "1" "int main(){3 != 2;}"
try "1" "int main(){3 >= 1;}"
try "0" "int main(){0 >= 1;}"
try "0" "int main(){3 <= 2;}"
try "1" "int main(){3 <= 5;}"
try "1" "int main(){3 > 1;}"
try "0" "int main(){0 > 0;}"
try "0" "int main(){3 < 3;}"
try "1" "int main(){3 < 5;}"
try "23" "int main(){22 + 4 - 3;}"
try "47" "int main(){5 + 6 * 7;}"
try "15" "int main(){5 * (9 - 6);}"
try "10" "int main(){-10+10*2;}"
try "4" "int main(){(3+5) /2;}"
try "3" "int main(){(3 == 3) + 2;}"
try "4" "int main(){int a; a=3;a+1;}"
try "11" "int main(){int b; b=5*6-8;return b/2;}"
try "10" "int main(){int a;int b;a=b=5;a+b;}"
try "5" "int main(){int a;int b;a=3;b=2;return a+b;}"
try "5" "int main(){int ab; int dvd; ab=3;dvd=2;return ab+dvd;}"
try "2" "int main(){int a; a=0;while (a<2) a=a+1;return a;}"
try "2" "int main(){int a; int i; a=0; for(i=0; i<2; i=i+1) a = a + 1;return a;}"
try "5" "int main(){int a; a=3;if(a==3) return a+2;}"
try "6" "int main(){int a; a=3;if (a==2) return 2;else return a+3;}"
try "5" "int main(){int a; a=2;if (a==2) {a=a+3; return a;}}"
try "5" "int main(){int a; int b; a=2;b=3;if (a==2) {a=a+b; return a;}}"
try "8" "int bra(){return 8;} int main(){return bra();}"
try "1" "int bra(int b){return b;} int main(){int a; a = 1;return bra(a);}"
try "5" "int bra(int b){int a; a=8; return b;} int main(){int a; a = 3;return bra(a-1)+a;}"
try "5" "int bra(int a){return a;} int main(){return bra(5);}"
try "5" "int bra(int a,int b){return a+b;} int main(){return bra(2,3);}"
try "5" "int bra(int a){return a;} int main(){return bra(2)+ bra(3);}"
try "55" "int sum(int m, int n) {int acc = 0;int i; for (i = m; i <= n; i = i + 1)acc = acc + i; return acc;} int main() { return sum(1, 10);}"
try "55" "int fib(int a){if(a==0)return 0;if(a==1)return 1;return fib(a-1)+fib(a-2);} int main(){return fib(10);}"
try "7" "int main() {int x; x= 7; int* y; y = &x; return *y;}"
try "7" "int main() {int x; int* y; y = &x; x = 7; return *y;}"
try "8" "int main() {int x; x = 7; int* y; y = &x; *y = 8; return x;}"

echo OK
