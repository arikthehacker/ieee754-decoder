# ieee754-decoder

![language](https://img.shields.io/badge/language-C-blue) ![platform](https://img.shields.io/badge/platform-Linux-lightgrey)
<!-- ![CI](https://github.com/arikthehacker/ieee754-decoder/actions/workflows/ci.yml/badge.svg) -->

Decodes a hex value into its IEEE 754 floating-point fields (sign, exponent, fraction)
and reconstructs the number. It handles single, double, half (binary16), bfloat16, and
minifloat, plus any custom width and bias from the command line.

## quickstart

```
git clone https://github.com/arikthehacker/ieee754-decoder.git
cd ieee754-decoder
sudo apt-get install -y build-essential
make run
```

expected output:

```
0x40490FDB
	0 10000000 10010010000111111011011
	s eeeeeeee fffffffffffffffffffffff
	normalized value
	sign:		positive
	bias:		127
	unbiased exp:	128
	E:		1
	frac:		0.57079637050628662109
	M:		1.57079637050628662109
	value:		3.14159274101257324219
	value:		3.14159274101257324219e+00
```

## how it works

The program builds a bit mask for the exponent field, then walks the bits from the sign
down through the exponent and fraction, printing each bit and accumulating the fraction.
The field widths come from the preset or the `-e`/`-f` flags, so the same loop handles any
format:

```c
// create the mask for exponent bits
for (int i = 0; i < exponentBits - 1; ++i)
{
    exponentMask <<= 1;
    exponentMask |= 1;
}
exponentMask <<= fractionBits;

// print binary representation
printf("\t%d ", (signBit) ? 1 : 0);
for (int i = exponentBits - 1; i >= 0; --i)
{
    bitMask >>= 1;
    printf("%d", (hexValue & bitMask) ? 1 : 0);
}
```

It then applies the bias to the exponent, adds the implicit leading bit (or the `-F`
value for a custom format), and checks the all-ones exponent for infinity and NaN. The
companion `float-2-hex` goes the other way.

## options

Running `./hex-2-float -H`:

```
Usage: ./hex-to-float [OPTIONS ...]
	-i [inputFile] | specifies the name of an input file| DEFAULT stdin
	-d use settings for double precision (double, 64-bits)
	-h use settings for half precision (binary16, 16-bits)
	-b use settings for half precision (bfloat16, 16-bits)
	-m use settings for quarter precision (minifloat, 8-bits)
	-e # set the number of bits to use for the exponent
	-E # set the value used for the exponent bias
	-f # set the number of bits to use for the fraction
	-F # set the value to add to the fraction (unstored fraction bits)
	-v verbose mode
	-H display this help message and exit.
```
