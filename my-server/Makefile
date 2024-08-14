TARGET=server
SRC = $(wildcard $(shell pwd)/*.cpp $(shell pwd)/src/*.cpp $(shell pwd)/utils/*.cpp)
OBJ = $(patsubst %.cpp *.c,%.o,$(SRC))
CFLAGS = -g -I $(shell pwd)/public/include -I $(shell pwd)/include # -fsanitize=address
CC =g++
LIBS =  -lpthread
$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) $(DEFS) -o $@ $^ $(LIBS)
.PHONY:
clean:
	rm -rf *.o $(TARGET)