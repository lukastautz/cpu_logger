/*
cpu_logger v1.0 <https://github.com/lukastautz/cpu_logger>
Copyright (C) 2024 Lukas Tautz

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

#include "include.h"

uint8 itoa_fill(uint32 n, char *dest, uint8 fill_to) {
    uint8 i = 0, z = 0, j;
    char tmp;
    while (n) {
        dest[i++] = (n % 10) + '0';
        n /= 10;
    }
    while (i < fill_to)
        dest[i++] = '0';
    j = i - 1;
    while (z < j) {
        tmp = dest[z], dest[z] = dest[j], dest[j] = tmp;
        ++z, --j;
    }
    return i;
}

uint8 itoa(uint32 n, char *s) {
    uint8 i = 0, y = 0, z;
    do
        s[i] = n % 10 + '0', ++i;
    while ((n /= 10) > 0);
    z = i - 1;
    for (char c; y < z; ++y, --z)
        c = s[y], s[y] = s[z], s[z] = c;
    return i;
}