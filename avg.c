/*
Copyright 2024-2025 Lukas Tautz

This file is part of cpu_logger.

cpu_logger is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "include.h"

void calculate_and_print_averages(int data_fd, int write_to) {
    measured_load load;
    char buf[8];
    double tmp;
#ifdef FEATURE_LOAD
    double cpu_load_percent = 0.0, cpu_load_max_percent = 0.0;
#endif
#ifdef FEATURE_STEAL
    double cpu_steal_percent = 0.0, cpu_steal_max_percent = 0.0;
#endif
#ifdef FEATURE_IOWAIT
    double iowait_percent = 0.0, iowait_max_percent = 0.0;
#endif
#ifdef FEATURE_MEMORY
    double memory_usage_percent = 0.0, memory_usage_max_percent = 0.0;
#endif
    uint64 points = 0;
    while (read(data_fd, &load, sizeof(load)) == sizeof(load)) {
#ifdef FEATURE_LOAD
        _ADD_AVG_MAX(cpu_load);
#endif
#ifdef FEATURE_STEAL
        _ADD_AVG_MAX(cpu_steal);
#endif
#ifdef FEATURE_IOWAIT
        _ADD_AVG_MAX(iowait);
#endif
#ifdef FEATURE_MEMORY
        _ADD_AVG_MAX(memory_usage);
#endif
        ++points;
    }
    WRITE("\t\t");
#ifdef FEATURE_LOAD
    cpu_load_percent /= (points ? points : 1);
    WRITE("Load\t\t");
#endif
#ifdef FEATURE_STEAL
    cpu_steal_percent /= (points ? points : 1);
    WRITE("Steal\t\t");
#endif
#ifdef FEATURE_IOWAIT
    iowait_percent /= (points ? points : 1);
    WRITE("IOWait\t\t");
#endif
#ifdef FEATURE_MEMORY
    memory_usage_percent /= (points ? points : 1);
    WRITE("Memory\t\t");
#endif
    WRITE("\nAverages:\t");
    bool is_first = true;
#ifdef FEATURE_LOAD
    _PRINT_PERCENT(cpu_load_percent);
    is_first = false;
#endif
#ifdef FEATURE_STEAL
    if (!is_first)
        WRITE("\t");
    _PRINT_PERCENT(cpu_steal_percent);
    is_first = false;
#endif
#ifdef FEATURE_IOWAIT
    if (!is_first)
        WRITE("\t");
    _PRINT_PERCENT(iowait_percent);
    is_first = false;
#endif
#ifdef FEATURE_MEMORY
    if (!is_first)
        WRITE("\t");
    _PRINT_PERCENT(memory_usage_percent);
#endif
    WRITE("\nMaxima:\t\t");
    is_first = true;
#ifdef FEATURE_LOAD
    _PRINT_PERCENT(cpu_load_max_percent);
    is_first = false;
#endif
#ifdef FEATURE_STEAL
    if (!is_first)
        WRITE("\t");
    _PRINT_PERCENT(cpu_steal_max_percent);
    is_first = false;
#endif
#ifdef FEATURE_IOWAIT
    if (!is_first)
        WRITE("\t");
    _PRINT_PERCENT(iowait_max_percent);
    is_first = false;
#endif
#ifdef FEATURE_MEMORY
    if (!is_first)
        WRITE("\t");
    _PRINT_PERCENT(memory_usage_max_percent);
#endif
    WRITE("\n");
}

int main(uint8 argc, char **argv) {
    uint64 maxima = 0;
    if (argc == 1 || (argc != 2 && !(argc == 3 && (maxima = atol(argv[2])) != 0)) || !memcmp(argv[1], "-h", sizeof("-h")) || !memcmp(argv[1], "--help", sizeof("--help"))) {
        WRITE_STDERR("Usage: ");
        WRITE_STDERR(argv[0]);
        WRITE_STDERR(" FILE [MAXIMA]\nThis program prints the average and max load (as configured) from the binary file FILE generated with cpu_logger.\nWhen MAXIMA is specified and there are more than MAXIMA elements within FILE, only the last MAXIMA elements are displayed and used for average/maxima calculations.\n");
        exit(1);
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        return 1;
    if (maxima) {
        int64 pos = lseek(fd, 0, SEEK_END) - (maxima * sizeof(measured_load));
        if (pos < 0)
            pos = 0; // there are less than MAXIMA elements in the file
        lseek(fd, pos, SEEK_SET);
    }
    calculate_and_print_averages(fd, STDOUT);
    close(fd);
}
