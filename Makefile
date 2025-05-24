OPENCV=1
OPENMP=0
DEBUG=0

OBJ=load_image.o process_image.o args.o filter_image.o resize_image.o test.o harris_image.o matrix.o panorama_image.o flow_image.o image_opencv.o
EXOBJ=main.o

VPATH=./src/:./
SLIB=libuwimg.dll
ALIB=libuwimg.a
EXEC=uwimg
OBJDIR=./obj/

CC=gcc
CXX=g++
AR=ar
ARFLAGS=rcs
OPTS=-Ofast
LDFLAGS= -lm -pthread 
COMMON= -Iinclude/ -Isrc/ 
CFLAGS=-Wall -Wno-unknown-pragmas -Wfatal-errors -fPIC

ifeq ($(OPENMP), 1)
CFLAGS+= -fopenmp
endif

ifeq ($(DEBUG), 1)
OPTS=-O0 -g
COMMON= -Iinclude/ -Isrc/
else
CFLAGS+= -flto
endif

CFLAGS+=$(OPTS)

ifeq ($(OPENCV), 1)
CFLAGS+= `pkg-config --cflags opencv4`
LDFLAGS+= `pkg-config --libs opencv4`
COMMON+= -DOPENCV
endif

EXOBJS = $(addprefix $(OBJDIR), $(EXOBJ))
OBJS = $(addprefix $(OBJDIR), $(OBJ))
DEPS = $(wildcard src/*.h) Makefile

all: obj $(SLIB) $(ALIB) $(EXEC)

$(EXEC): $(EXOBJS) $(OBJS)
	$(CXX) $(COMMON) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(ALIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(SLIB): $(OBJS)
	g++ $(CFLAGS) -shared $^ -o $@ $(LDFLAGS)

$(OBJDIR)%.o: %.c $(DEPS)
	$(CC) $(COMMON) $(CFLAGS) -c $< -o $@

$(OBJDIR)%.o: %.cpp $(DEPS)
	$(CXX) $(COMMON) $(CFLAGS) -c $< -o $@

obj:
	mkdir -p obj

.PHONY: clean

clean:
	rm -rf $(OBJS) $(SLIB) $(ALIB) $(EXEC) $(EXOBJS) $(OBJDIR)/*
