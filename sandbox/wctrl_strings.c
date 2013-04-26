/* 
 * Copyright 2013 anthony cantor
 * This file is part of aug.
 *
 * aug is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * aug is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with aug.  If not, see <http://www.gnu.org/licenses/>.
 */
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
