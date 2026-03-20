pen my_pen = TimesRoman()+1+fontsize(10);
defaultpen(my_pen);

size(12cm, 0);

draw((0,0)--(0,10)--(18,10)--(18,0)--cycle, linewidth(2));

for (int i=0; i<=14; ++i) 
{
	draw((i,3)--(i,10));
}
for (int j=10; j>=3; --j)
{
	draw((0,j)--(14,j));
}

draw((0,1)--(14,1));
for (int i=0; i<=14; ++i)
{
	draw((i,0)--(i,1));
}

draw((17,3)--(17,10));
for (int j=10; j>=3; --j)
{
	draw((17,j)--(18,j));
}

draw((17,0)--(17,1));
draw((17,1)--(18,1));

label("0", (0,10)--(1,10), N);
label("1", (1,10)--(2,10), N);
label("2", (2,10)--(3,10), N);
label("3", (3,10)--(4,10), N);
label("\ldots", (4,10)--(6,10), N);

label("0", (0,10)--(0,9), W);
label("1", (0,9)--(0,8), W);
label("2", (0,8)--(0,7), W);
label("\vdots", (0,7)--(0,5), W);	

label("\ldots", (14,7)--(17,7), S);	
label("\ldots", (14,1)--(17,1), S);
label("\vdots", (3,1)--(3,3), E);
label("\vdots", (12,1)--(12,3), E);

label("319", (17,10)--(18,10), N);
label("239", (0,1)--(0,0), W);
