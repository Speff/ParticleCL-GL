INCDIR=./include
CC=g++
CFLAGS=-I$(INCDIR) -m32 -Wall -static-libgcc -static-libstdc++
STATIC_FLAGS=-DGLEW_STATIC #-FREEGLUT_STATIC

SRCDIR=./src
OBJDIR=obj
LIBDIR=./lib
LIBS=-lm -lglew32_static -lopengl32 -lOpenCL -lfreeglut

_DEPS=ProgSettings.h Window.h pCL.h pGL.h
DEPS=$(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ=Window.o pCL.o pGL.o
OBJ=$(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CC) $(STATIC_FLAGS) -c -o $@ $< $(CFLAGS)

ParticleCL-GL: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -L$(LIBDIR) $(LIBS)

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~
