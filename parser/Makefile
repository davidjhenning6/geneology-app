all: list parser shared

list:
	gcc -std=c11 -Wall -Iinclude -g  -fpic src/LinkedListAPI.c -o ../LLAPI.o -c ;\
	ar cr ../liblist.a ../LLAPI.o

parser: utilities
	gcc -std=c11 -Wall -Iinclude -g  -fpic src/GEDCOMparser.c -o ../GP.o -c ;\
	ar cr ../libparser.a ../GP.o ../util.o

shared: 
	gcc -shared -o ../sharedLib.so ../LLAPI.o ../GP.o ../util.o

utilities:
	gcc -std=c11 -Wall -Iinclude -g  -fpic src/GEDCOMutilities.c -o ../util.o -c

clean:
	rm -f ../*.o ;\
	rm -f ../*.a
