if false || false; then echo not good; else echo good; fi > res1.txt; cat -e res1.txt;
if true || echo 1; then echo good; else echo not good; fi > res2.txt; cat -e res2.txt;
if echo 1 || echo 2; then echo good; else echo not good; fi > res3.txt; cat -e res3.txt;
if true || true; then echo good; else echo not good; fi > res4.txt; cat -e res4.txt;
if false && false; then echo good; else echo not good; fi > res5.txt; cat -e res5.txt;
if false && true; then echo good; else echo not good; fi > res6.txt; cat -e res6.txt;
if true && false; then echo good; else echo not good; fi > res7.txt; cat -e res7.txt;
if true && true; then echo good; else echo not good; fi > res8.txt; cat -e res8.txt;
if echo 1 && echo 2; then true; else false; fi > res9.txt; cat -e res9.txt;
