#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {

	int debug_on, encoding_mode, key_idx, c;
	debug_on = encoding_mode = key_idx = 0;

	char *key = NULL;

	FILE *infile = stdin;
	FILE *outfile = stdout;

	for (int i = 1; i < argc; i++) {

		char *arg = argv[i];

		if (*arg == '+') {
			switch (arg[1]) {
				case 'D': debug_on = 1; break;
				case 'e':
					if (encoding_mode) goto invalid_args;
						encoding_mode = 1;
						key = arg + 2;
						break;
				default: goto invalid_args;
			}
		}
		else if (*arg == '-') {
			switch(arg[1]) {
				case 'D': debug_on = 0; break;
				case 'e':
					if (encoding_mode) goto invalid_args;
						encoding_mode = -1;
						key = arg + 2;
						break;
				case 'o':
					if (outfile != stdout) goto invalid_args;
					outfile = fopen(arg + 2, "w");
					break;
				case 'i':
					if (infile != stdin) goto invalid_args;
					infile = fopen(arg + 2, "r");
					if (!infile) {
						fprintf(stderr, "Error: Input file doesn't exist.\n");
						return 0;
					}
					break;
				default: goto invalid_args;
			}
		}
		else goto invalid_args;

		if (debug_on && arg[1] != 'D')
			fprintf(stderr, "%s\n", arg);
	}

	while ((c = fgetc(infile)) != EOF) {

		if (!encoding_mode || key[key_idx] == '\0')
			fputc(c, outfile);
		else {
			int encoded_c = c + (key[key_idx] - '0') * encoding_mode;
			int digit_or_letter = 1;

			if (c >= 'A' && c <= 'Z'){ // is uppercase
				if (encoded_c > 'Z')
					encoded_c -= 26;
				else if (encoded_c < 'A')
					encoded_c += 26;
			}
			else if (c >= 'a' && c <= 'z'){ // is lowercase
				if (encoded_c > 'z')
					encoded_c -= 26;
				else if (encoded_c < 'a')
					encoded_c += 26;
			}
			else if (c >= '0' && c <= '9'){ // is digit
				if (encoded_c > '9')
					encoded_c -= 10;
				else if (encoded_c < '0')
					encoded_c += 10;
			}
			else
				digit_or_letter = 0;

			fputc(digit_or_letter ? encoded_c : c, outfile);
			key_idx = key[++key_idx] == '\0' ? 0 : key_idx;
		}
	}

	if (infile != stdin)
		fclose(infile);
	if (outfile != stdout)
		fclose(outfile);

	return 0;

	invalid_args:
		fprintf(stderr, "Error: Invalid command line arguments.\n");
		return 0;
}
