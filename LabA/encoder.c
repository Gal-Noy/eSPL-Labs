#include <stdio.h>
#include <string.h>

int invalid_args() {
	fprintf(stderr, "Error: Invalid command line arguments.\n");
	return 0;
}

int main(int argc, char **argv) {

	int debug_on, encoding_mode, key_idx, c;
	debug_on = encoding_mode = key_idx = 0;
	char *key = NULL;
	FILE *input = stdin;
	FILE *output = stdout;

	for (int i = 1; i < argc; i++) {

		char *arg = argv[i];

		if (debug_on && arg[1] != 'D') // Handle debugging.
			fprintf(stderr, "%s\n", arg);
			
		if (*arg != '+' && *arg != '-') invalid_args(); // Check arg is valid.

		switch(arg[1]) {
			case 'D': // Set debug (default = OFF).
				debug_on = *arg == '+' ? 1 : 0;
				break;
			case 'e': // Set encode (default = OFF, 1 = add, -1 = subtract).
				if (encoding_mode) invalid_args();
				encoding_mode = *arg == '+' ? 1 : -1;
				key = arg + 2;
				break;
			case 'o': // Set output (default = stdout).
				if (*arg != '-' || output != stdout) invalid_args();
				output = fopen(arg + 2, "w");
				break;
			case 'i': // Set input (default = stdin).
				if (*arg != '-' || input != stdin) invalid_args();
				input = fopen(arg + 2, "r");
				if (!input) {
					fprintf(stderr, "Error: Input file doesn't exist.\n");
					return 0;
				}
				break;
			default: invalid_args(); // Handle another invalid args.
		}
	}

	while ((c = fgetc(input)) != EOF) {

		if (!encoding_mode || key[key_idx] == '\0') // True iff key is empty.
			fputc(c, output);

		else {
			int diff = (key[key_idx] - '0') * encoding_mode;
			int enc_c = c + diff; // Calculate encoded char.
			int digit_or_letter = 1;

			// Wrap if needed.
			if (c >= 'A' && c <= 'Z') // Uppercase.
				enc_c += enc_c < 'A' ? 26 : enc_c > 'Z' ? -26 : 0;
			else if (c >= 'a' && c <= 'z') // Lowercase.
				enc_c += enc_c < 'a' ? 26 : enc_c > 'z' ? -26 : 0;
			else if (c >= '0' && c <= '9') // Digit.
				enc_c += enc_c < '0' ? 10 : enc_c > '9' ? -10 : 0;
			else
				digit_or_letter = 0;

			fputc(digit_or_letter ? enc_c : c, output);
			key_idx = key[++key_idx] == '\0' ? 0 : key_idx; // Move to the next char or wrap.
		}
	}

	if (input != stdin) fclose(input);
	if (output != stdout) fclose(output);

	return 0;
}
