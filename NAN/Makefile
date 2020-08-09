server:  obj/main.o obj/misc.o obj/request.o obj/response.o obj/setup.o
	gcc -Wall -static -o server obj/main.o obj/misc.o obj/request.o obj/response.o obj/setup.o

obj/misc.o: src/misc.c src/project.h src/misc.h
	gcc -Wall -c src/misc.c -o $@

obj/setup.o: src/setup.c src/project.h src/setup.h src/request.h
	gcc -Wall -c src/setup.c -o $@

obj/request.o: src/request.c src/project.h src/request.h src/misc.h
	gcc -Wall -c src/request.c -o $@

obj/response.o: src/response.c src/project.h src/response.h src/request.h src/misc.h
	gcc -Wall -c src/response.c -o $@

obj/main.o: src/main.c src/project.h src/setup.h src/request.h src/response.h
	gcc -Wall -c src/main.c -o $@

clean:
	rm -f obj/*.o src/*~ src/*.o