#include <ncurses.h>

int main() {
	int ch;

	for(ch = 0; ch < 256; ch++) {
		printf("0x%02x: %s\n", ch, unctrl(ch) );
	}

	return 0;
}
