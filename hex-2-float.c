#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// Function prototypes
void print_help(void);
unsigned long create_mask(int bits);
void print_binary(unsigned long value, int bits);

void print_help(void) {
    printf("Usage: ./hex-2-float [OPTION ...]\n");
    printf(" -i filename    specify the name of an input file\n");
    printf(" -d             use settings for double precision (double, 64-bits, 1-11-52)\n");
    printf(" -h             use settings for half precision (binary16, 16-bits, 1-5-10)\n");
    printf(" -b             use settings for bfloat16 (16-bits, 1-8-7)\n");
    printf(" -m             use settings for minifloat (8-bits, 1-4-3, bias -2)\n");
    printf(" -e #           set the number of bits to use for the exponent\n");
    printf(" -E #           set the value used for the exponent bias\n");
    printf(" -f #           set the number of bits to use for the fraction\n");
    printf(" -F #           set the value to add to the fraction (unstored fraction bits)\n");
    printf(" -v             enable verbose mode\n");
    printf(" -H             display this help message and exit\n");
    exit(EXIT_SUCCESS);
}

unsigned long create_mask(int bits) {
    unsigned long mask = 1UL;
    for (int i = 1; i < bits; ++i) {
        mask = (mask << 1) | 1;
    }
    return mask;
}

void print_binary(unsigned long value, int bits) {
    for (int i = bits - 1; i >= 0; --i) {
        printf("%lu", (value >> i) & 1);
    }
}

int main(int argc, char *argv[]) {
    int opt;
    int exp_bits = 8, frac_bits = 23, exp_bias = 127; // default for float
    int verbose = 0;
    int add_to_frac = 0;
    char buf[41];
    char hex_str[11]; // To hold the hexadecimal part
    char input_str[31]; // To hold the remaining input string
    unsigned long sign_mask, exp_mask, frac_mask, uv;
    int sign;
    unsigned long raw_exp, raw_frac;
    double frac, value;

    while ((opt = getopt(argc, argv, "i:dhebmE:e:f:F:vH")) != -1) {
        switch (opt) {
            case 'i':
                freopen(optarg, "r", stdin);
                break;
            case 'd':
                exp_bits = 11;
                frac_bits = 52;
                exp_bias = 1023;
                break;
            case 'h':
                exp_bits = 5;
                frac_bits = 10;
                exp_bias = 15;
                break;
            case 'b':
                exp_bits = 8;
                frac_bits = 7;
                exp_bias = 127;
                break;
            case 'm':
                exp_bits = 4;
                frac_bits = 3;
                exp_bias = -2;
                break;
            case 'e':
                exp_bits = atoi(optarg);
                break;
            case 'E':
                exp_bias = atoi(optarg);
                break;
            case 'f':
                frac_bits = atoi(optarg);
                break;
            case 'F':
                add_to_frac = atoi(optarg);
                break;
            case 'v':
                verbose = 1;
                break;
            case 'H':
                print_help();
                break;
            default:
                print_help();
                break;
        }
    }

    sign_mask = 1UL << (exp_bits + frac_bits);
    exp_mask = create_mask(exp_bits) << frac_bits;
    frac_mask = create_mask(frac_bits);

    while (fgets(buf, sizeof(buf), stdin)) {
        buf[strcspn(buf, "\n")] = '\0'; // Safely remove the trailing newline character

        // Split input into hexadecimal part and the remaining input string
        sscanf(buf, "%10s %30[^\n]", hex_str, input_str);

        if (sscanf(hex_str, "%lx", &uv) != 1) {
            fprintf(stderr, "Failed to scan value from input <%s>\n", buf);
            exit(EXIT_FAILURE);
        }

        // Apply addition to the fraction
        uv += add_to_frac;

        // Extracting sign, exponent, and fraction
        sign = (uv & sign_mask) ? -1 : 1;
        raw_exp = (uv & exp_mask) >> frac_bits;
        raw_frac = uv & frac_mask;

        frac = raw_frac / (double)(1UL << frac_bits);

        // Print input value and binary representation
        printf("0x%08lx %s\n", uv, input_str); // Print the hexadecimal part and remaining string
        printf("\t");
        printf("%d ", (sign == -1) ? 1 : 0); // Print sign bit correctly
        print_binary(raw_exp, exp_bits);
        printf(" ");
        print_binary(raw_frac, frac_bits);
        printf("\n\ts eeeeeeee fffffffffffffffffffffff\n");

        // Handle special values
        if (raw_exp == create_mask(exp_bits)) {
            if (raw_frac == 0) {
                value = sign == 1 ? INFINITY : -INFINITY;
                printf("\tspecial value\n\t%s infinity\n", sign == 1 ? "positive" : "negative");
            } else {
                value = NAN;
                printf("\tspecial value\n\tNaN\n");
            }
        } else if (raw_exp == 0) {
            if (raw_frac == 0) {
                value = 0.0;
                printf("\tdenormalized value\n");
            } else {
                value = sign * frac * pow(2, 1 - exp_bias);
                printf("\tdenormalized value\n");
            }
        } else {
            value = sign * (1 + frac) * pow(2, raw_exp - exp_bias);
            printf("\tnormalized value\n");
        }

        // Print detailed information
        printf("\tsign:\t\t%s\n", sign == 1 ? "positive" : "negative");
        printf("\tbias:\t\t%d\n", exp_bias);
        printf("\tunbiased exp:\t%lu\n", raw_exp);
        printf("\tE:\t\t%ld\n", raw_exp == 0 ? (long)(1 - exp_bias) : (long)(raw_exp - exp_bias));
        printf("\tfrac:\t\t%-.20lf\n", frac);
        printf("\tM:\t\t%-.20lf\n", 1 + frac);
        printf("\tvalue:\t\t%-.20lf\n", value);
        printf("\tvalue:\t\t%-.20le\n\n", value);

        // Verbose output
        if (verbose) {
            fprintf(stderr, "Verbose mode: hex=%lx, value=%lf\n", uv, value);
        }
    }

    return 0;
}

