/*
cpu_logger <https://github.com/lukastautz/cpu_logger>
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

#define _PRINT_DOUBLE(name) \
    write(write_to, buf, itoa(load.name##_percent, buf)); \
    WRITE("."); \
    write(write_to, buf, itoa_fill(load.name##_percent_after_dot, buf, 2));

void write_html(int data_fd, int write_to) {
    measured_load load;
    char buf[8];
    bool is_first = true;
#ifdef FEATURE_LOAD
    double cpu_load_percent = 0.0, cpu_load_max_percent = 0.0;
#endif
#ifdef FEATURE_STEAL
    double cpu_steal_percent = 0.0, cpu_steal_max_percent = 0.0;
#endif
#ifdef FEATURE_MEMORY
    double memory_usage_percent = 0.0, memory_usage_max_percent = 0.0;
#endif
    double tmp;
    uint64 points = 0;
    WRITE("<!DOCTYPE html><html><head><title>Load statistics</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><style>body,html{margin:0}#graph{position:absolute;inset:10px 0 2.9rem 0}#averages{position:absolute;bottom:.9rem;text-align:center;font-size:1.3rem;width:100vw}select{position:absolute;top:.3rem;left:.3rem;z-index:99}</style><link rel=\"stylesheet\" href=\"https://dygraphs.com/2.2.1/dist/dygraph.min.css\"><script src=\"https://dygraphs.com/2.2.1/dist/dygraph.min.js\"></script><script>function d(time,...data){return [new Date((time+1577836800)*1000),...data];}var graph;window.addEventListener(\"load\",function(){graph=new Dygraph(document.getElementById(\"graph\"),[");
    while (read(data_fd, &load, sizeof(load)) == sizeof(load)) {
        if (!is_first)
            WRITE(",");
        else
            is_first = false;
        WRITE("d(");
        write(write_to, buf, itoa(load.time, buf));
#ifdef FEATURE_LOAD
        WRITE(",");
        _PRINT_DOUBLE(cpu_load);
        _ADD_AVG_MAX(cpu_load);
#endif
#ifdef FEATURE_STEAL
        WRITE(",");
        _PRINT_DOUBLE(cpu_steal);
        _ADD_AVG_MAX(cpu_steal);
#endif
#ifdef FEATURE_MEMORY
        WRITE(",");
        _PRINT_DOUBLE(memory_usage);
        _ADD_AVG_MAX(memory_usage);
#endif
        WRITE(")");
        ++points;
    }
    WRITE("],{digitsAfterDecimal:3,showRoller:true,labels:[\"x\"");
#ifdef FEATURE_LOAD
    WRITE(",\"load\"");
    cpu_load_percent /= (points ? points : 1);
#endif
#ifdef FEATURE_STEAL
    WRITE(",\"steal\"");
    cpu_steal_percent /= (points ? points : 1);
#endif
#ifdef FEATURE_MEMORY
    WRITE(",\"memory\"");
    memory_usage_percent /= (points ? points : 1);
#endif
    WRITE("],legend:\"follow\",legendFollowOffsetX:0,legendFollowOffsetY:0,title:\"Load statistics\",rollPeriod:0,customBars:false,ylabel:\"Usage (%)\",valueRange:[0,101],animatedZooms:true});});function updateView(mode){for(var i=0;i<graph.visibility().length;++i){graph.setVisibility(i,mode==\"all\"||i==mode);}}</script></head><body><select onchange=\"updateView(this.value);\"><option value=\"all\">All</option>");
    char i = '0';
#ifdef FEATURE_LOAD
    WRITE("<option value=\"");
    write(write_to, &i, 1);
    ++i;
    WRITE("\">Load</option>");
#endif
#ifdef FEATURE_STEAL
    WRITE("<option value=\"");
    write(write_to, &i, 1);
    ++i;
    WRITE("\">Steal</option>");
#endif
#ifdef FEATURE_MEMORY
    WRITE("<option value=\"");
    write(write_to, &i, 1);
    ++i;
    WRITE("\">Memory</option>");
#endif

    WRITE("</select><div id=\"graph\"></div><div id=\"averages\"><b>Averages</b>: ");
    is_first = true;
#ifdef FEATURE_LOAD
    write(write_to, "load: ", strlen("load: "));
    _PRINT_PERCENT(cpu_load_percent);
    is_first = false;
#endif
#ifdef FEATURE_STEAL
    if (!is_first)
        write(write_to, "; ", strlen("; "));
    write(write_to, "steal: ", strlen("steal: "));
    _PRINT_PERCENT(cpu_steal_percent);
    is_first = false;
#endif
#ifdef FEATURE_MEMORY
    if (!is_first)
        write(write_to, "; ", strlen("; "));
    write(write_to, "memory: ", strlen("memory: "));
    _PRINT_PERCENT(memory_usage_percent);
#endif
    WRITE(" &nbsp; <b>|</b> &nbsp; <b>Maxima</b>: ");
#ifdef FEATURE_LOAD
    write(write_to, "load: ", strlen("load: "));
    _PRINT_PERCENT(cpu_load_max_percent);
    is_first = false;
#endif
#ifdef FEATURE_STEAL
    if (!is_first)
        write(write_to, "; ", strlen("; "));
    write(write_to, "steal: ", strlen("steal: "));
    _PRINT_PERCENT(cpu_steal_max_percent);
    is_first = false;
#endif
#ifdef FEATURE_MEMORY
    if (!is_first)
        write(write_to, "; ", strlen("; "));
    write(write_to, "memory: ", strlen("memory: "));
    _PRINT_PERCENT(memory_usage_max_percent);
#endif
    WRITE("</div></body></html>");
}

int main(uint8 argc, char **argv) {
    uint64 maxima = 0;
    if (argc == 1 || (argc != 2 && !(argc == 3 && (maxima = atol(argv[2])) != 0)) || !memcmp(argv[1], "-h", sizeof("-h")) || !memcmp(argv[1], "--help", sizeof("--help"))) {
        WRITE_STDERR("Usage: ");
        WRITE_STDERR(argv[0]);
        WRITE_STDERR(" FILE [MAXIMA]\nThis program converts the binary file FILE generated with cpu_logger to a viewable HTML webpage.\nWhen MAXIMA is specified and there are more than MAXIMA elements within FILE, only the last MAXIMA elements are displayed and used for average/maxima calculations.\n");
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
    write_html(fd, STDOUT);
    close(fd);
}
