CC=gcc
FLEX=flex
BISON=bison


.lex: lex.l
	$(FLEX) lex.l
.syntax: syntax.y
	$(BISON) -t -d -v syntax.y
splc: .lex .syntax
	$(CC) syntax.tab.c -lfl -ly -D CALC_MAIN -o ./splc
	# $(CC) syntax.tab.c -lfl -ly -D CALC_MAIN -o ./bin/splc
libcalc: .lex .syntax
	$(CC) syntax.tab.c -lfl -ly -fPIC --shared -o libcalc.so
clean:
	@rm -f lex.yy.c syntax.tab.* *.out *.so
