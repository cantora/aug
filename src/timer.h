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

#include <time.h>
#include <sys/time.h>

struct aug_timer {
	struct timeval start;
};

int timer_init(struct aug_timer *tmr);
int timer_thresh(struct aug_timer *tmr, int sec, int usec);
int timer_elapsed(struct aug_timer *tmr, struct timeval *elapsed);

#endif /* AUG_TIMER_H */