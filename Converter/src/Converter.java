
//	Joseph Sackitey
//	This Java class handles conversion between different base

import java.util.*;

public class Converter {

	public static void main(String[] args) {
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

	public static long from10(long number, int base) {
		if (number == 0) {
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

	public static String from10(long number) {
		if (number == 0) {
			return "0";
		}

		String ans = "";

		while (number > 0) {
			long rem = number % 16;
			ans = getSymbol(rem) + ans;
			number = number / 16;
		}

		return ans;
	}

	public static long to10(long number, int base) {
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

	public static long to10(String number) {
		long ans = 0;
		long cur_pow = 1;

		for (int i = number.length() - 1; i >= 0; i--) {
			char c = number.charAt(i);
			long dig = getValue(c);
			ans = ans + (dig * cur_pow);
			cur_pow = cur_pow * 16;
		}

		return ans;
	}

	public static HashMap<Character, Integer> buildRomanMap() {
		HashMap<Character, Integer> map = new HashMap<>();
		map.put('I', 1);
		map.put('V', 5);
		map.put('X', 10);
		map.put('L', 50);
		map.put('C', 100);
		map.put('D', 500);
		map.put('M', 1000);

		return map;
	}

	public static int fromRoman(String number) {
		HashMap<Character, Integer> map = buildRomanMap();

		int ans = 0;
		int n = number.length();

		for (int i = 0; i < n; i++) {
			int val = map.get(number.charAt(i));

			if (i + 1 < n) {
				int nxtval = map.get(number.charAt(i + 1));

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

	public static char getSymbol(long digit) {
		if (digit >= 0 && digit <= 9) {
			return (char) ('0' + digit);
		} else {
			return (char) ('A' + (digit - 10));
		}
	}

	public static long getValue(char symbol) {
		if (symbol >= '0' && symbol <= '9') {
			return symbol - '0';
		} else {
			return (symbol - 'A') + 10;
		}
	}

	public static <E1, E2> void assertEquals(E1 a, E2 b) {
		String strA = Objects.toString(a);
		String strB = Objects.toString(b);

		if (!strA.equals(strB)) {
			throw new RuntimeException();
		}
	}
}
