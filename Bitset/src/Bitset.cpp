//============================================================================
// Name        : Bitset.cpp
// Author      : sackjo02
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cassert>
using namespace std;

template<typename T>
class Bitset {
private:
	T data;
	const T ZERO = 0;
	const T ONE = 1;
	const int NUM_BITS = 8 * sizeof(T);

public:

	Bitset() :
			Bitset(ZERO) {
	}

	explicit Bitset(T theBits) :
			data(theBits) {
	}

	T getValue() const {
		return data;
	}

	bool none() const {
		return data == ZERO;
	}

	bool any() {
		return data != ZERO;
	}

	bool all(){
		return data == (~ZERO);
	}

	void flip() {
		data = data ^ (~ZERO);
	}

	bool get(int index) const{
		T mask = ONE;
		mask <<= index;
		return (data & mask) != ZERO;
	}

	void set() {
		data = ~ZERO;
	}

	void set(int index) {
		T mask = ONE;
		mask <<= index;
		data |= mask;
	}

	void clear() {
		data ^= data;
	}

	void clear(int index) {
		T mask = ONE;
		mask <<= index;
		data &= ~mask;
	}

	void swap() {
		T left = data >> (NUM_BITS / 2);
		T right = data << (NUM_BITS / 2);
		data = left | right;
	}

	void swapHi() {
		int half = NUM_BITS / 2;
		int quart = half / 2;

		T rembyte = data & (~ZERO >> half);
		T qmask = ~ZERO >> (half + quart);

		T left = (data >> quart) & (quart << half);

		T right = ((data << quart) & (qmask << (half + quart)));
		data = rembyte | left | right;

	}

	void swapLo() {
		int half = NUM_BITS / 2;
		int quart = half / 2;

		T rembyte = data & (~ZERO << half);
		T qmask = ~ZERO >> (half + quart);

		T left = (data >> quart) & (qmask);

		T right = (data << quart) & (qmask << quart);
		data = rembyte | left | right;
	}

	bool isPow2() const {
		return (data != ZERO) && ((data & (data - 1)) == ZERO);
	}

	void clearLast1() {
		data &= (data - 1);
	}

	int count() const {
		int ans = 0;
		T mask = ONE;

		for (int i = 0; i < NUM_BITS; i++) {
			if ((data & mask) != ZERO) {
				ans += 1;
			}

			mask = mask << 1;
		}

		return ans;
	}

	void printBinary() const{
		cout << "0b";
		T mask = ONE;
		mask = mask << (mask - 1);

		for (int i = 0; i < NUM_BITS; i++) {
			char c;

			if ((data & mask) == ZERO) {
				c = '0';
			} else {
				c = '1';
			}

			cout << c;
			mask = mask >> 1;
		}
	}

	void print() {
		cout << "[" << dec << data << ", 0x" << hex << data << ", 0" << oct
				<< data << ", ";
		printBinary();
		cout << "]" << endl;
		cout << dec;
	}

	bool operator==(const Bitset<T> &a) const {
		return (this->getValue() == a.getValue());
	}

	Bitset<T> operator&(const Bitset<T> &a) const {
		return Bitset<T>(this->getValue() & a.getValue());
	}

	Bitset<T> operator~() const {
		return Bitset<T>(~this->getValue());
	}

	Bitset<T> operator<<(int factor) const {
		T bit = this->getValue() << factor;
		return Bitset<T>(bit);
	}

	bool operator<(const Bitset<T> &b) const {
		return (this->getValue() < b.getValue());
	}

	Bitset<T>& operator^=(const Bitset<T> &a) {
		this->data ^= a.getValue();
		return *this;
	}

	Bitset<T>& operator>>=(int a) {
		this->data >>= a;
		return *this;
	}

	Bitset<T>& operator++() {
		++this->data;
		return *this;
	}
};

template<typename T>
bool operator!=(const Bitset<T> &a, const Bitset<T> &b) {
	return !(a == b);
}

//template<typename T>
//Bitset<T> operator~(const Bitset<T>& a) {
//	return Bitset<T>(~a.getValue());
//}

template<typename T>
Bitset<T> operator^(const Bitset<T> &a, const Bitset<T> &b) {
	return Bitset<T>(a.getValue() ^ b.getValue());
}

//template<typename T>
//Bitset operator|(const Bitset& a , const Bitset& b){
//	return Bitset(a.getValue() | b.getValue());
//}

template<typename T>
Bitset<T> operator|(const Bitset<T> &a, const Bitset<T> &b) {
	return ~(~a & ~b);
}

template<typename T>
Bitset<T> operator>>(const Bitset<T> &a, int factor) {
	return Bitset<T>(a.getValue() >> factor);
}

template<typename T>
bool operator>=(Bitset<T> &a, Bitset<T> &b) {
	return !(a < b);
}

template<typename T>
ostream& operator<<(ostream &os, const Bitset<T> &a) {
	os << "[" << dec << a.getValue() << ", 0x" << hex << a.getValue() << ", 0"
			<< oct << a.getValue() << ", ";
	os << "]";
	os << dec;
	return os;
}

template<typename T>
T id(T value) {
	return value;
}

int main() {
	Bitset<unsigned short> bitset(0xABCD);
	assert(bitset.getValue() == id(0xABCD));
	assert(bitset.count() == 10);
	assert(bitset.any());

	Bitset<unsigned short> single(0x0004);
	assert(single.isPow2());
	single.clearLast1();
	assert(single.none());

	Bitset<unsigned short> bitset1(0xABCD);
	Bitset<unsigned short> bitset2(0xABCD);
	assert((bitset1 & bitset2) == Bitset<unsigned short>(0xABCD));
	assert((bitset1 & bitset2) != Bitset<unsigned short>(0xABD));

	Bitset<unsigned short> inc(10);
	++inc;
	assert(inc == Bitset<unsigned short>(11));

	return 0;
}

