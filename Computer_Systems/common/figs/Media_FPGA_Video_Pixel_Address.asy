pen my_pen = TimesRoman()+2+fontsize(12);
defaultpen(my_pen);

size(12cm, 0);

draw((0,0)--(0,1)--(21,1)--(21,0)--cycle);
draw((20,0)--(20,1), linewidth(1));
draw((14,0)--(14,1), linewidth(1));
draw((8,0)--(8,1), linewidth(1));

label("31", (0,1), NE);
label("\ldots", (1,1)--(7,1), N);
label("18", (8,1), NW);
label("17", (8,1), NE);
label("\ldots", (9,1)--(13,1), N);
label("10", (14,1), NW);
label("9", (14,1), NE);
label("\ldots", (15,1)--(19,1), N);
label("1", (20,1), NW);
label("0", (21,1), NW);

label("0", (20,0)--(21, 0), N);
label("x", (14,0)--(20,0), N);
label("y", (8,0)--(14,0), N);
label("00001000000000", (0,0)--(8,0), N);
