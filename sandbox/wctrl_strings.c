#include <stdint.h>

#include "ncurses.h"

int main() {
	uint32_t ch;
	cchar_t cch;

	for(ch = 0; ch < 256; ch++) {
		setcchar(&cch, &ch, 0, 0, NULL);
		printf("0x%02x: wunctrl => %ls, key_name => %s", ch, wunctrl(&cch), key_name(ch) );
		printf("\n");
	}

	return 0;
}
