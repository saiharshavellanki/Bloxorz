all: sample2D

sample2D: Sample_GL3_2D.cpp glad.c
	g++ -std=c++11 -o sample2D Sample_GL3_2D.cpp glad.c -ldl -lGL -lglfw -lftgl -I/usr/include -I/usr/local/include -I/usr/local/include/freetype2 -L/usr/local/lib 
clean:
	rm sample2D
