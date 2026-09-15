import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.assertEquals;

public class ConverterTest {

	@Test
	public void testBase2Conversion() {
		assertEquals(1001L, Converter.from10(9, 2));
		assertEquals(9L, Converter.to10(1001, 2));
		assertEquals(11001L, Converter.from10(25, 2));
		assertEquals(25L, Converter.to10(11001, 2));
	}

	@Test
	public void testBase3Conversion() {
		assertEquals(2L, Converter.from10(2, 3));
		assertEquals(2L, Converter.to10(2, 3));
		assertEquals(222L, Converter.from10(26, 3));
		assertEquals(26L, Converter.to10(222, 3));
		assertEquals(10201L, Converter.from10(100, 3));
		assertEquals(100L, Converter.to10(10201, 3));
	}

	@Test
	public void testBase7And8Conversion() {
		assertEquals(12L, Converter.from10(9, 7));
		assertEquals(9L, Converter.to10(12, 7));
		assertEquals(666L, Converter.from10(342, 7));
		assertEquals(342L, Converter.to10(666, 7));

		assertEquals(10L, Converter.from10(8, 8));
		assertEquals(8L, Converter.to10(10, 8));
		assertEquals(777L, Converter.from10(511, 8));
		assertEquals(511L, Converter.to10(777, 8));
	}

	@Test
	public void testBase10Conversion() {
		assertEquals(12345L, Converter.from10(12345, 10));
		assertEquals(12345L, Converter.to10(12345, 10));
	}

	@Test
	public void testHexadecimalConversion() {
		assertEquals("D", Converter.from10(13));
		assertEquals(13L, Converter.to10("D"));
		assertEquals("14", Converter.from10(20));
		assertEquals(20L, Converter.to10("14"));
		assertEquals("ABCD", Converter.from10(43981));
		assertEquals(43981L, Converter.to10("ABCD"));
		assertEquals("ABCDEF", Converter.from10(11259375));
		assertEquals(11259375L, Converter.to10("ABCDEF"));
	}

	@Test
    public void testRomanNumerals() {
        assertEquals(5, Converter.fromRoman("V"));
        assertEquals(4, Converter.fromRoman("IV"));
        assertEquals(9, Converter.fromRoman("IX"));
        assertEquals(40, Converter.fromRoman("XL"));
        assertEquals(90, Converter.fromRoman("XC"));
        assertEquals(400, Converter.fromRoman("CD"));
        assertEquals(900, Converter.fromRoman("CM"));
        assertEquals(8, Converter.fromRoman("VIII"));
        assertEquals(29, Converter.fromRoman("XXIX"));
        assertEquals(74, Converter.fromRoman("LXXIV"));
        assertEquals(789, Converter.fromRoman("DCCLXXXIX"));
        assertEquals(2421, Converter.fromRoman("MMCDXXI"));
        assertEquals(160, Converter.fromRoman("CLX"));
        assertEquals(207, Converter.fromRoman("CCVII"));
        assertEquals(1066, Converter.fromRoman("MLXVI"));
        assertEquals(1666, Converter.fromRoman("MDCLXVI"));
        assertEquals(1904, Converter.fromRoman("MCMIV"));
        assertEquals(1954, Converter.fromRoman("MCMLIV"));
        assertEquals(1990, Converter.fromRoman("MCMXC"));
        assertEquals(2014, Converter.fromRoman("MMXIV"));
        assertEquals(3999, Converter.fromRoman("MMMCMXCIX"));
    }
}