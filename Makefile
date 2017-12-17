OBJ = bmpgen.o EasyBMP.o

all: bmpgen

bmpgen: $(OBJ)
	g++ -o $@ $^

bmpgen.o: bmpgen.cpp
	g++ -c $^ -Wall

EasyBMP.o: EasyBMP_1.06/EasyBMP.cpp
	g++ -c $^

clean:
	rm *o bmpgen
