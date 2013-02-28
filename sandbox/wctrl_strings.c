#include <stdint.h>

#include "ncurses.h"

int main() {
	uint32_t ch;
	cchar_t cch;

	for(ch = 0; ch < 256; ch++) {
		setcchar(&cch, &ch, 0, 0, NULL);
		printf("0x%02x: %ls", ch, wunctrl(&cch) );
		printf("\n");
	}

	return 0;
}
