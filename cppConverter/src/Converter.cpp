//============================================================================
// Name        : cppConverter.cpp
// Author      : sackjo02
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;
#include <map>

string to_string(const string &str) {
	return str;
}

template<typename E1, typename E2>
void assertEquals(E1 a, E2 b) {
	string strA = to_string(a);
	string strB = to_string(b);

	if (strA != strB) {
		cout << "failed: " << a << " != " << b << endl;
	}
}

char getSymbol(long digit) {
	if (digit >= 0 && digit <= 9) {
		return (char) ('0' + digit);
	} else {
		return (char) ('A' + (digit - 10));
	}
}

long getValue(char symbol) {
	if (symbol >= '0' && symbol <= '9') {
		return symbol - '0';
	} else {
		return (symbol - 'A') + 10;
	}
}

long from10(long number, int base) {
	if (number <= 0) {
		return 0;
	}

	long ans = 0;
	long curr = 1;

	while (number > 0) {
		long rem = number % base;
		ans = ans + (rem * curr);
		curr = curr * 10;
		number = number / base;
	}

	return ans;
}

string from10(long number) {
	if (number == 0) {
		return "0";
	}

	string ans = "";

	while (number > 0) {
		long rem = number % 16;
		ans = getSymbol(rem) + ans;
		number = number / 16;
	}

	return ans;
}

long to10(long number, int base) {
	long ans = 0;
	long curr_pow = 1;

	while (number > 0) {
		long dig = number % 10;
		ans = ans + (dig * curr_pow);
		curr_pow = curr_pow * base;
		number = number / 10;
	}

	return ans;
}

long to10(string number) {
	long ans = 0;
	long cur_pow = 1;

	for (int i = number.length() - 1; i >= 0; i--) {
		char c = number[i];
		long dig = getValue(c);
		ans = ans + (dig * cur_pow);
		cur_pow = cur_pow * 16;
	}

	return ans;
}

std::map<char, int> buildRomanMap() {
	std::map<char, int> mapdata;

	mapdata['I'] = 1;
	mapdata['V'] = 5;
	mapdata['X'] = 10;
	mapdata['L'] = 50;
	mapdata['C'] = 100;
	mapdata['D'] = 500;
	mapdata['M'] = 1000;

	return mapdata;
}

int fromRoman(string number) {
	map<char, int> mapdata = buildRomanMap();

	int ans = 0;
	int n = number.length();

	for (int i = 0; i < n; i++) {
		int val = mapdata[number[i]];

		if (i + 1 < n) {
			int nxtval = mapdata[number[i + 1]];

			if (val < nxtval) {
				ans -= val;
			}

			else {
				ans += val;
			}
		}

		else {
			ans += val;
		}
	}

	return ans;
}

int main() {
	assertEquals(from10(9, 2), 1001);
	assertEquals(from10(9, 7), 12);
	assertEquals(from10(13), "D");
	assertEquals(from10(20), "14");
	assertEquals(to10(1001, 2), 9);
	assertEquals(to10(12, 7), 9);
	assertEquals(to10("D"), 13);
	assertEquals(to10("14"), 20);
	assertEquals(fromRoman("V"), 5);
	assertEquals(fromRoman("IX"), 9);
	assertEquals(fromRoman("MCCCVII"), 1307);
}

