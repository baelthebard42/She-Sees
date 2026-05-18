OPENCV=0
OPENMP=0
DEBUG=0

# -------------------- FILES --------------------
OBJ=load_image.o process_image.o args.o filter_image.o resize_image.o harris_image.o matrix.o panorama_image.o flow_image.o
EXOBJ=main.o
OPENCV_OBJ=opencv_bridge.o

VPATH=./src/:./
SLIB=libuwimg.so
ALIB=libuwimg.a
EXEC=uwimg
OBJDIR=./obj/

# -------------------- TOOLS --------------------
CC=gcc
CXX=g++
AR=ar
ARFLAGS=rcs

# -------------------- FLAGS --------------------
OPTS=-Ofast
LDFLAGS= -lm -pthread
COMMON= -Iinclude/ -Isrc/
CFLAGS=-Wall -Wno-unknown-pragmas -Wfatal-errors -fPIC
CXXFLAGS=-Wall -Wfatal-errors -fPIC

# -------------------- OPENMP --------------------
ifeq ($(OPENMP), 1)
CFLAGS+= -fopenmp
CXXFLAGS+= -fopenmp
endif

# -------------------- DEBUG --------------------
ifeq ($(DEBUG), 1)
OPTS=-O0 -g
else
CFLAGS+= -flto
CXXFLAGS+= -flto
endif

CFLAGS+=$(OPTS)
CXXFLAGS+=$(OPTS)

# -------------------- OPENCV --------------------
ifeq ($(OPENCV), 1)
COMMON+= -DOPENCV
CFLAGS+= -DOPENCV
CXXFLAGS+= -DOPENCV
LDFLAGS+= `pkg-config --libs opencv4`
COMMON+= `pkg-config --cflags opencv4`
endif

# -------------------- OBJECTS --------------------
EXOBJS = $(addprefix $(OBJDIR), $(EXOBJ))
OBJS = $(addprefix $(OBJDIR), $(OBJ))

DEPS = $(wildcard src/*.h) Makefile

# -------------------- TARGETS --------------------
all: obj $(SLIB) $(ALIB) $(EXEC)

$(EXEC): $(EXOBJS) $(OBJS) $(OBJDIR)$(OPENCV_OBJ)
	$(CXX) $(COMMON) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(ALIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(SLIB): $(OBJS) $(OBJDIR)$(OPENCV_OBJ)
	$(CXX) $(CXXFLAGS) -shared $^ -o $@ $(LDFLAGS)

# -------------------- C OBJECTS --------------------
$(OBJDIR)%.o: %.c $(DEPS)
	$(CC) $(COMMON) $(CFLAGS) -c $< -o $@

# -------------------- OPENCV BRIDGE --------------------
$(OBJDIR)opencv_bridge.o: src/opencv_bridge.cpp
	$(CXX) $(COMMON) $(CXXFLAGS) `pkg-config --cflags opencv4` -c $< -o $@

# -------------------- BUILD DIR --------------------
obj:
	mkdir -p obj

# -------------------- CLEAN --------------------
.PHONY: clean
clean:
	rm -rf $(OBJDIR)/*.o $(SLIB) $(ALIB) $(EXEC)