pen my_pen = TimesRoman()+2+fontsize(12);
defaultpen(my_pen);

size(12cm, 0);

draw((0,0)--(0,1)--(18,1)--(18,0)--cycle);
draw((12,0)--(12,1), linewidth(1));
draw((6,0)--(6,1), linewidth(1));

label("7", (0,1), NE);
label("5", (6,1), NW);
label("4", (6,1), NE);
label("2", (12,1), NW);
label("1", (12,1), NE);
label("0", (18,1), NW);

label("red", (0,0)--(6,0), 2N);
label("green", (6,0)--(12,0), 2N);
label("blue", (12,0)--(18,0), 2N);
