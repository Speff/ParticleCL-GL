CC=g++
CFLAGS=-I$(INCDIR) -m32 -Wall -static-libgcc -static-libstdc++
STATIC_FLAGS=#-DGLEW_STATIC #-FREEGLUT_STATIC

INCDIR=./include
BINDIR=./bin
SRCDIR=./src
OBJDIR=obj
LIBDIR=./lib

LIBS=-lm -lopengl32 -lOpenCL -lfreeglut

_DEPS=ProgSettings.h Window.h pCL.h pGL.h gl_core_3_3.h
DEPS=$(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ=Window.o pCL.o pGL.o gl_core_3_3.o
OBJ=$(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CC) $(STATIC_FLAGS) -c -o $@ $< $(CFLAGS)

ParticleCL-GL: $(OBJ)
	$(CC) -o $(BINDIR)/$@ $^ $(CFLAGS) -L$(LIBDIR) $(LIBS)

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~
