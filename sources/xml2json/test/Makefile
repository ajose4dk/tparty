CC=gcc
CXX=g++
INCLUDE += -I../include/
LIB += 
CFLAGS+= -msse2 -DRAPIDJSON_SSE2 -Wall -O3
TARGET=xml2json
ALL:$(TARGET)

C_SRCFILES := $(wildcard *.c) 
CPP_SRCFILES := $(wildcard *.cpp)

C_OBJS := $(patsubst %.c,%.o, $(C_SRCFILES))
CPP_OBJS := $(patsubst %.cpp,%.o, $(CPP_SRCFILES))

OBJS := $(C_OBJS) $(CPP_OBJS)

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIB)

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c -o $@ $< 
	
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $< 

clean:
	rm -f *.o
	rm -f $(TARGET)
