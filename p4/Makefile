CC = /usr/bin/g++

#OPENGL LIBS FOR LINUX
GLLIB :=  -lGL -lGLEW -lGLU -lglut
#OPENGL LIBS FOR MAC
#GLLIB := -framework OpenGL -framework GLUT

#COMPILER FLAGS
CCFLAGS := -Wall -g -std=c++11

#include directories
#should include gl.h glut.h etc...
INCDIR := -I/usr/include
LDLIBS := $(GLLIB)

#TARGET = glutdemo
#OBJS = demo.o 
TARGET = rcasteli_p4
SOURCES = $(wildcard *.cpp)
OBJS = $(SOURCES:.cpp=.o)


all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC)  $^ $(CCFLAGS) $(LDLIBS)  -o $@

%.o : %.cpp
	$(CC) $(CCFLAGS) -o $@ -c $(INCDIR) $<

clean:
	rm -rf $(OBJS) $(TARGET)

