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
#ifndef AUG_TIMER_H
#define AUG_TIMER_H

#if defined(__APPLE__)
/* for timercmp, timersub */
#	define _DARWIN_C_SOURCE
#endif
#include <time.h>
#include <sys/time.h>

struct aug_timer {
	struct timeval start;
};

#define AUG_TIMER_ALLOC() \
	struct aug_timer aug_tmr; \
	struct timeval aug_tmr_elapsed

#define AUG_TIMER_START() \
	AUG_STATUS_EQUAL(timer_init(&aug_tmr), 0);

#define AUG_TIMER_IF_EXCEEDED(secs, usecs) \
	if( \
		(timer_elapsed(&aug_tmr, &aug_tmr_elapsed) == 0) \
		&& aug_tmr_elapsed.tv_sec >= secs \
		&& aug_tmr_elapsed.tv_usec >= usecs \
	)

#define AUG_TIMER_DISPLAY(fp, fmt) \
	fprintf(fp, fmt, (int) aug_tmr_elapsed.tv_sec, (int) aug_tmr_elapsed.tv_usec)

int timer_init(struct aug_timer *tmr);
int timer_thresh(struct aug_timer *tmr, int sec, int usec);
int timer_elapsed(struct aug_timer *tmr, struct timeval *elapsed);

#endif /* AUG_TIMER_H */