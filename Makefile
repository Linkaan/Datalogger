#
# Makefile:
#   defines rules to build the fagelmatare datalogger module.
###############################################################################
#  This file is part of Fågelmataren, an embedded project created to learn
#  Linux and C. See <https://github.com/Linkaan/Fagelmatare>
#  Copyright (C) 2015-2017 Linus Styrén
#
#  Fågelmataren is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the Licence, or
#  (at your option) any later version.
#
#  Fågelmataren is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public Licence for more details.
#
#  You should have received a copy of the GNU General Public Licence
#  along with Fågelmataren.  If not, see <http://www.gnu.org/licenses/>.
##############################################################################

INCLUDE ?= -I.
LINKS ?= -L.
CFLAGS := $(INCLUDE) -std=gnu11 -g -Wall -Wextra -D _GNU_SOURCE
LDFLAGS := $(LINKS) -lpthread -lfg-events -lfg-serializer -levent\
 -levent_pthreads -lz -lcrypto
CC := gcc
SOURCES := log.c dlogger.c
HEADERS := log.h common.h
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE := fagelmatare-dlogger

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEFINES)

.PHONY: clean

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
