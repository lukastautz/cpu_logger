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

#if defined(FEATURE_LOAD) || defined(FEATURE_STEAL) // cpu

typedef struct jiffies_spent_s {
    uint64 user;
    uint64 nice;
    uint64 system;
    uint64 idle;
    uint64 iowait;
    uint64 irq;
    uint64 softirq;
    uint64 steal;
#ifdef MEASURE_GUEST_TIME
    uint64 guest;
    uint64 guest_nice;
#endif
    uint64 work; /* = user+nice+system */
    uint64 total;
} jiffies_spent; // 72 or 88 bytes

jiffies_spent get_current_jiffies() {
    uint64 tmp;
    char buf[256], *ptr = buf;
    jiffies_spent data = { .total = 0 };
    int fd = open("/proc/stat", O_RDONLY);
    if (fd == -1 || (tmp = read(fd, buf, 256)) == -1 || tmp < 128)
        return data;
    close(fd);
    ptr += 3; // skip "cpu"
    while (isspace(*++ptr) && PTR_IS_IN_BUF(ptr, buf, 256));
    if (!PTR_IS_IN_BUF(ptr, buf, 256))
        return data;
#ifndef MEASURE_GUEST_TIME
    for (uint8 i = 0; i < 8; ++i) {
#else
    for (uint8 i = 0; i < 10; ++i) {
#endif
        tmp = atol(ptr);
        data.total += tmp;
        memcpy((uint64 *)((uint64)&data + (uint64)(8 * i)), &tmp, 8);
        while (!isspace(*++ptr) && PTR_IS_IN_BUF(ptr, buf, 256));
        if (!PTR_IS_IN_BUF(ptr, buf, 256)) {
            data.total = 0;
            return data;
        }
        while (isspace(*++ptr) && PTR_IS_IN_BUF(ptr, buf, 256));
        if (!PTR_IS_IN_BUF(ptr, buf, 256)) {
            data.total = 0;
            return data;
        }
    }
#ifndef MEASURE_GUEST_TIME
    data.work = data.user + data.nice + data.system;
#else
    data.work = data.user + data.nice + data.system + data.guest + data.guest_nice;
#endif
    return data;
}

jiffies_spent get_jiffies_diff(jiffies_spent d1, jiffies_spent d2) {
    jiffies_spent diff;
    diff.work = d2.work - d1.work;
    diff.steal = d2.steal - d1.steal;
    diff.total = d2.total - d1.total;
    return diff;
}

#endif  // cpu

measured_load measure_load(uint16 sleep_seconds) {
    measured_load load;
    double integer;
#if defined(FEATURE_LOAD) || defined(FEATURE_STEAL)
    jiffies_spent d1, d2, diff;
    d1 = get_current_jiffies();
    if (!d1.total)
        exit(9);
    load.time = (uint32)(((uint32)((uint64)time(NULL) - 1577836800 /* 1.1.2020 */)) + (uint32)(sleep_seconds / 2));
#endif
    sleep(sleep_seconds);
#if defined(FEATURE_LOAD) || defined(FEATURE_STEAL)
    d2 = get_current_jiffies();
    if (!d2.total)
        exit(9);
    diff = get_jiffies_diff(d1, d2);
#ifdef FEATURE_LOAD
    double cpu_load_percent = ((double)diff.work / (double)diff.total) * 100.0;
    load.cpu_load_percent = (uint8)cpu_load_percent;
    load.cpu_load_percent_after_dot = (uint16)((cpu_load_percent - (double)load.cpu_load_percent) * 100.0);
#endif
#ifdef FEATURE_STEAL
    double cpu_steal_percent = ((double)diff.steal / (double)diff.total) * 100.0;
    load.cpu_steal_percent = (uint8)cpu_steal_percent;
    load.cpu_steal_percent_after_dot = (uint16)((cpu_steal_percent - (double)load.cpu_steal_percent) * 100.0);
#endif
#endif // cpu logging
#ifdef FEATURE_MEMORY
    char buf[256], *ptr = buf + 10;
    int fd = open("/proc/meminfo", O_RDONLY);
    if (fd == -1 || read(fd, buf, 256) < 128)
        exit(9);
    close(fd);
    ptr = buf + 10;
    while (isspace(*++ptr) && PTR_IS_IN_BUF(ptr, buf, 256));
    uint32 total_memory = atol(ptr); // will overflow if more than 4 TiB of memory is installed (as it is in KiB)
    while (*++ptr != 'F' && PTR_IS_IN_BUF(ptr, buf, 256)); // Free
    while (*++ptr != 'A' && PTR_IS_IN_BUF(ptr, buf, 256)); // Available
    ptr += strlen("vailable ");
    while (isspace(*++ptr) && PTR_IS_IN_BUF(ptr, buf, 256));
    double memory_usage_percent = ((double)(total_memory - atol(ptr) /* total memory - available memory = used memory */) / total_memory) * 100.0;
    load.memory_usage_percent = (uint8)memory_usage_percent;
    load.memory_usage_percent_after_dot = (uint16)((memory_usage_percent - (double)load.memory_usage_percent) * 100.0);
#endif
    return load;
}

bool fd_is_valid(int fd) {
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

bool always_true_sleep(int time) {
    sleep(time);
    return true;
}

// return values:
    // 1: first open() failed
    // 2: reopening failed
    // 3: writing failed
    // 4: did not write all at first, and the second attempt did not succeed
    // 9: reading /proc/stat or /proc/meminfo failed
    // 99: can't happen

int main(int argc, char **argv) {
    char *save_to;
    uint16 sleep_seconds;
    if (argc != 3 || !memcmp(argv[1], "-h", sizeof("-h")) || !memcmp(argv[1], "--help", sizeof("--help")) || !(sleep_seconds = atoi(argv[2]))) {
        WRITE_STDERR("Usage: ");
        WRITE_STDERR(argv[0]);
        WRITE_STDERR(" FILE INTERVAL\nThis program saves the measured load (as configured) every INTERVAL seconds to FILE in a binary format.\n");
        exit(1);
    }
    int fd = open(save_to = argv[1], O_APPEND | O_CREAT | O_WRONLY, 0644), write_return;
    if (fd == -1)
        return 1;
    measured_load load;
    for (;;) {
        load = measure_load(sleep_seconds);
        if (!fd_is_valid(fd) && (fd = open(save_to, O_APPEND | O_CREAT | O_WRONLY, 0644)) == -1)
            return 2;
        if ((write_return = write(fd, &load, sizeof(load))) == -1 && errno != EINTR)
            return 3;
        else if (write_return < sizeof(load) && always_true_sleep(5) && write(fd, &load + write_return, sizeof(load) - write_return) != (sizeof(load) - write_return))
            return 4;
        fsync(fd);
    }
    return 99;
}
