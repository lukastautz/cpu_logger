default: utils.o cpu_logger cpu_logger_html cpu_logger_avg
utils.o:
	gcc -c -O3 -Os -fcompare-debug-second -Wno-deprecated-declarations -Wno-unused-result utils.c -o utils.o
	-diet -Os gcc -c -O3 -Os -fcompare-debug-second -Wno-deprecated-declarations -Wno-unused-result utils.c -o utils.o
cpu_logger:
	-gcc -fcompare-debug-second -Wno-deprecated-declarations -Wno-unused-result -O3 -Os cpu_logger.c utils.o -o cpu_logger
	-diet -Os gcc -static -fcompare-debug-second -Wno-deprecated-declarations -Wno-unused-result -O3 -Os utils.o cpu_logger.c -o cpu_logger
	-elftrunc cpu_logger cpu_logger
cpu_logger_html:
	-gcc -fcompare-debug-second -Wno-deprecated-declarations -Wno-unused-result -O3 -Os html.c utils.o -o cpu_logger_html
	-diet -Os gcc -static -fcompare-debug-second -Wno-deprecated-declarations -Wno-unused-result -O3 -Os utils.o html.c -o cpu_logger_html
	-elftrunc cpu_logger_html cpu_logger_html
cpu_logger_avg:
	-gcc -fcompare-debug-second -Wno-deprecated-declarations -Wno-unused-result -O3 -Os avg.c utils.o -o cpu_logger_avg
	-diet -Os gcc -static -fcompare-debug-second -Wno-deprecated-declarations -Wno-unused-result -O3 -Os utils.o avg.c -o cpu_logger_avg
	-elftrunc cpu_logger_avg cpu_logger_avg
clean:
	-rm cpu_logger cpu_logger_html cpu_logger_avg utils.o
