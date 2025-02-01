DIET_GCC_INSTALLED=$(shell which diet > /dev/null && echo 1 || echo 0)
ifeq ($(DIET_GCC_INSTALLED), 1)
	CC=diet gcc -static
	ELFTRUNC=elftrunc
else
	CC=gcc
	ELFTRUNC=/bin/true
endif

default: clean utils.o cpu_logger cpu_logger_html cpu_logger_avg
utils.o:
	$(CC) -c -O3 -Os -fcompare-debug-second -Wno-deprecated-declarations -Wno-unused-result utils.c -o utils.o
cpu_logger:
	$(CC) -fcompare-debug-second -Wno-deprecated-declarations -Wno-unused-result -O3 -Os cpu_logger.c utils.o -o cpu_logger
	$(ELFTRUNC) cpu_logger cpu_logger
cpu_logger_html:
	$(CC) -fcompare-debug-second -Wno-deprecated-declarations -Wno-unused-result -O3 -Os html.c utils.o -o cpu_logger_html
	$(ELFTRUNC) cpu_logger_html cpu_logger_html
cpu_logger_avg:
	$(CC) -fcompare-debug-second -Wno-deprecated-declarations -Wno-unused-result -O3 -Os avg.c utils.o -o cpu_logger_avg
	$(ELFTRUNC) cpu_logger_avg cpu_logger_avg
clean:
	-rm cpu_logger cpu_logger_html cpu_logger_avg utils.o
install: default
	cp cpu_logger cpu_logger_html cpu_logger_avg /bin
