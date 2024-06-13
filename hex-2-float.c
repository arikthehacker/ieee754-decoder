// Ariella Marchuk   |||   amarchuk@pdx.edu
// ========================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define BUFFER_LENGTH 1000

int main(int argc, char **argv) 
{
    // initialize variables and set default values
    short option;
    short precisionMode = 0;
    short isVerbose = 0;

    char inputFile[BUFFER_LENGTH] = {'\0'};
    char buffer[BUFFER_LENGTH] = {'\0'};

    long exponentBits = 8;
    long fractionBits = 23;

    double exponentBias = 0.0;
    double fractionAddition = 1.0;

    FILE *inputStream = NULL;

    uint8_t isNaN = 0;
    int signMultiplier;

    double exponentValue;
    double unbiasedExponent;
    double rawFraction;
    double fraction;
    double power;
    double finalResult;

    unsigned long hexValue;
    unsigned long bitMask;
    unsigned long signBit;
    unsigned long signMask;
    unsigned long exponentMask;

    // parse command-line options
    while ((option = getopt(argc, argv, "Hhbmvde:E:f:F:i:")) != -1) 
    {
        switch (option) 
        {
            case 'i':
                // copy the input file name
                strcpy(inputFile, optarg);
                break;
            case 'd':
                // set precision mode to double
                precisionMode = 1;
                break;
            case 'h':
                // set precision mode to half
                precisionMode = 2;
                break;
            case 'b':
                // set precision mode to bfloat
                precisionMode = 3;
                break;
            case 'm':
                // set precision mode to minifloat
                precisionMode = 4;
                break;
            case 'e':
                // set the number of exponent bits
                exponentBits = strtol(optarg, NULL, 10);
                break;
            case 'E':
                // set the exponent bias value
                exponentBias = strtod(optarg, NULL);
                break;
            case 'f':
                // set the number of fraction bits
                fractionBits = strtol(optarg, NULL, 10);
                break;
            case 'F':
                // set the fraction addition value
                fractionAddition = strtod(optarg, NULL);
                break;
            case 'v':
                // enable verbose mode
                isVerbose = 1;
                break;
            case 'H':
                // display the help message and exit
                printf("Usage: ./hex-to-float [OPTIONS ...]\n"
                       "\t-i [inputFile] | specifies the name of an input file| DEFAULT stdin\n"
                       "\t-d use settings for double precision (double, 64-bits)\n"
                       "\t-h use settings for half precision (binary16, 16-bits)\n"
                       "\t-b use settings for half precision (bfloat16, 16-bits)\n"
                       "\t-m use settings for quarter precision (minifloat, 8-bits)\n"
                       "\t-e # set the number of bits to use for the exponent\n"
                       "\t-E # set the value used for the exponent bias\n"
                       "\t-f # set the number of bits to use for the fraction\n"
                       "\t-F # set the value to add to the fraction (unstored fraction bits)\n"
                       "\t-v verbose mode\n"
                       "\t-H display this help message and exit.\n");
                return 0;
        }
    }

    // check for unrecognized flags
    if (optind < argc) 
    {
        fprintf(stderr, "\nFlags not recognized:\n");
        for (int i = optind; i < argc; ++i)
        {
            printf("\t%s\n", argv[i]);
        }
    }

    // handle verbose mode and file opening
    if (isVerbose)
    {
        fprintf(stderr, "\n\nread file...\nfile: %s\n", inputFile);
    }

    if (fractionBits && !fractionAddition) 
    {
        fractionAddition = 1;
    }

    if (inputFile[0] == '\0') 
    {
        inputStream = stdin;
    } 
    else 
    {
        inputStream = fopen(inputFile, "r");
    }

    // read each line from the input
    while (fgets(buffer, BUFFER_LENGTH, inputStream)) 
    {
        buffer[strlen(buffer) - 1] = '\0';

        unbiasedExponent = 0;
        exponentValue = 0;
        rawFraction = 0.0;
        fraction = 0.0;
        signBit = 1.0;
        isNaN = 0;
        power = 1;
        finalResult = 0.0;
        hexValue = 0x0;
        exponentMask = 1;
        signMask = 1L << (exponentBits + fractionBits);

        // convert the hex string to a number
        if (sscanf(buffer, "%lx", &hexValue) != 1) 
        {
            fprintf(stderr, "Failed to scan value from input <%s>\n", buffer);
            exit(EXIT_FAILURE);
        } 
        else 
        {
            // handle precision-specific settings
            if (precisionMode) 
            {
                if (precisionMode == 1) 
                {
                    exponentBits = 11;
                    fractionBits = 52;
                } 
                else if (precisionMode == 2) 
                {
                    exponentBits = 5;
                    fractionBits = 10;
                } 
                else if (precisionMode == 3) 
                {
                    exponentBits = 8;
                    fractionBits = 7;
                } 
                else if (precisionMode == 4) 
                {
                    exponentBits = 4;
                    fractionBits = 3;
                    exponentBias = -2.0;
                }
            }

            // calculate the exponent bias if not set
            if (!exponentBias) 
            {
                exponentBias = pow(2, exponentBits - 1) - 1;
            }
            bitMask = 1L << (exponentBits + fractionBits);
            signBit = hexValue & signMask;
            signMultiplier = (hexValue & bitMask) ? -1 : 1;
            rawFraction = 0.0;
            exponentMask = 1;
            // create the mask for exponent bits
            for (int i = 0; i < exponentBits - 1; ++i) 
            {
                exponentMask <<= 1;
                exponentMask |= 1;
            }
            exponentMask <<= fractionBits;
            unbiasedExponent = hexValue & exponentMask;
            power = 1;

            // print the input value
            printf("%s\n", buffer);

            // print binary representation
            printf("\t%d ", (signBit) ? 1 : 0);
            for (int i = exponentBits - 1; i >= 0; --i) 
            {
                bitMask >>= 1;
                printf("%d", (hexValue & bitMask) ? 1 : 0);
            }
            printf(" ");
            for (int i = fractionBits - 1; i >= 0; --i) 
            {
                bitMask >>= 1;
                printf("%d", (hexValue & bitMask) ? 1 : 0);
                if (hexValue & bitMask) 
                {
                    rawFraction += pow(2, (-1 * power));
                    isNaN = 1;
                }
                ++power;
            }
            printf("\n\ts ");
            for (int i = exponentBits; i > 0; --i) 
            {
                bitMask >>= 1;
                printf("e");
            }
            printf(" ");
            for (int i = fractionBits; i > 0; --i) 
            {
                bitMask >>= 1;
                printf("f");
            }

            // handle special values
            if (unbiasedExponent == exponentMask) 
            {
                if (!isNaN) 
                {
                    if (!signBit)
                        printf("\n\tspecial value\n\tpositive infinity\n\n");
                    else
                        printf("\n\tspecial value\n\tnegative infinity\n\n");
                } 
                else
                    printf("\n\tspecial value\n\tNaN\n\n");
            } 
            else 
            {
                unbiasedExponent = hexValue & exponentMask;
                unbiasedExponent = (unsigned long)unbiasedExponent >> fractionBits;
                fraction = fractionAddition + rawFraction;

                // calculate normalized and denormalized values
                if (unbiasedExponent == 0) 
                {
                    exponentValue = (1 - exponentBias);
                    fraction = rawFraction;
                } 
                else 
                {
                    exponentValue = unbiasedExponent - exponentBias;
                    fraction = fractionAddition + rawFraction;
                }

                finalResult = (signMultiplier * fraction * pow(2, exponentValue));
                printf("\n");
                printf("\t%s\n", (unbiasedExponent == 0) ? "denormalized value" : "normalized value");
                printf("\tsign:\t\t%s\n", (!signBit) ? "positive" : "negative");
                printf("\tbias:\t\t%-10.0lf\n", exponentBias);
                printf("\tunbiased exp:\t%-10.0lf\n", unbiasedExponent);
                printf("\tE:\t\t%-10.0f\n", exponentValue);
                printf("\tfrac:\t\t%-.20lf\n", rawFraction);
                printf("\tM:\t\t%-.20lf\n", fraction);
                printf("\tvalue:\t\t%-.20lf\n", finalResult);
                printf("\tvalue:\t\t%-.20le\n\n", finalResult);
            }
        }
    }

    return EXIT_SUCCESS;
}

