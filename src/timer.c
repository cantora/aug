/* 
 * Copyright 2012 anthony cantor
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
#include "timer.h"

int timer_init(struct aug_timer *tmr) {
	if(gettimeofday(&tmr->start, NULL) != 0)
		return -1;

	return 0;
}

/* return 1 if the elapsed time is greater
 * than or equal to sec*1,000,000 + usec 
 * microseconds and return 0 otherwise.
 * returns -1 on error with errno set.
 */
int timer_thresh(struct aug_timer *tmr, int sec, int usec) {
	struct timeval now, diff, amt;
	amt.tv_sec = sec;
	amt.tv_usec = usec;

	if(gettimeofday(&now, NULL) != 0)
		return -1;

	timersub(&now, &tmr->start, &diff);
	if(!timercmp(&diff, &amt, <) )
		return 1;
	
	return 0;
}

