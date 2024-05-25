#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

void print_help(void);

void print_help(void) {
    printf("Usage: ./float-2-hex [OPTION ...]\n");
    printf(" -f   convert the input into floats for hex output (this is the default)\n");
    printf(" -d   convert the input into doubles for hex output\n");
    printf(" -H   display this help message and exit\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    int opt;
    int use_double = 0;
    char buf[128];

    while ((opt = getopt(argc, argv, "fdH")) != -1) {
        switch (opt) {
            case 'f':
                use_double = 0;
                break;
            case 'd':
                use_double = 1;
                break;
            case 'H':
                print_help();
                break;
            default:
                print_help();
                break;
        }
    }

    while (fgets(buf, sizeof(buf), stdin)) {
        buf[strlen(buf) - 1] = '\0'; // Remove the trailing newline character

        if (use_double) {
            double d;
            unsigned long ul;
            if (sscanf(buf, "%lf", &d) != 1) {
                fprintf(stderr, "Failed to scan value from input <%s>\n", buf);
                exit(EXIT_FAILURE);
            }
            memcpy(&ul, &d, sizeof(d));
            printf("%-40s\t%.16le\t%.16lf\t0x%016lx\n", buf, d, d, ul);
        } else {
            float f;
            unsigned int ui;
            if (sscanf(buf, "%f", &f) != 1) {
                fprintf(stderr, "Failed to scan value from input <%s>\n", buf);
                exit(EXIT_FAILURE);
            }
            memcpy(&ui, &f, sizeof(f));
            printf("%-40s\t%.10e\t%.10f\t0x%08x\n", buf, f, f, ui);
        }
    }

    return 0;
}

