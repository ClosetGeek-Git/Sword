/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SWOOLE_CHORD_H_
#define SWOOLE_CHORD_H_

#include "ext/swoole/config.h"
#include "ext/swoole/php_swoole_cxx.h"
#include "config.h"

typedef struct _php_chord_object {
  zval *object;
  int row;
  int fd;
  bool connected;
  double timeout;
  swTimer_node *timer;
} chdht_object;

#define CHORD_ASSOC           1<<0
#define CHORD_NUM             1<<1
#define CHORD_BOTH            (CHORD_ASSOC|CHORD_NUM)

#endif
