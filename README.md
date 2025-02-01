# cpu_logger
cpu_logger is a minimal program written in C logging the load, steal, memory usage and IOwait of a linux system. It can also generate html files with graphical statistics (using dygraphs) and averages.

## Internals
cpu_logger saves the data in a binary file. It appends the data in the interval specified when calling cpu_logger.

## Configuration
You can edit config.h if you don't want to save for example the used memory.

## Memory consumption
On my machines it needs around 56 kB of memory (32 kB RSS), although that does depend on the configuration and the cpu architecture. (Note that depending on the program used to measure the used amount of memory, the displayed consumption may be higher as Linux file cache influences it greatly (unfortunately caching is hard/impossible to disable as O_DIRECT has too much disadvantages to be used).)

## Disk space needed
It needs (with all features enabled) 10 bytes per interval. **If you for example log it every 10 seconds it needs approx. 30 MiB of disk space a year**. I recommend that you log to a ramdisk as otherwise frequent disk accesses will happen.

## Binary size
The binaries are linked against dietlibc 12 KiB big each.

## Building
Simply call `make`.
It is recommended to use [dietlibc](https://www.fefe.de/dietlibc), as the static binaries linked against dietlibc are smaller (and need way less memory) than the dynamically linked against glibc. However, if dietlibc is not installed, the programs are linked against glibc. That's also the cause for possible (harmless) errors and the ugly Makefile.

## Starting daemon
Simply call `cpu_logger LOG_FILE INTERVAL_IN_SECONDS` (after you built it and copied the binaries to /bin). There is also a systemd-service-file example (although it probably isn't the best possible). The daemon doesn't need to run as root, it only needs read access to /proc/meminfo and /proc/cpuinfo and write access to the log file.

## Generating the html file
Simply call `cpu_logger_html LOG_FILE`. You can see a live example at layer7 [here](https://0l7.0lt.de/cpu.php). The html files have the disadvantage that as all data is contained the file can get quite large.

## Why?
The goal of cpu_logger is that it absolutely doesn't affect the system. I would say that with 52 KiB memory usage and near to no cpu usage (as it sleeps the most time), that goal is reached. It of course doesn't have as much functions as the bigger programs, but that wasn't the goal.

## I have a suggestion/It doesn't work/I found a bug
Feel free to open an issue or pull request.

## Steal time
Please note that steal time does only work (AFAIK) with KVM, and the provider can disable it or provide arbitrary values. If the cpu speed varies, but the displayed steal is always 0.00%, steal time reporting is likely disabled.
