TARGET = libcxd2837.so

INCLUDE=-I./Server

CFLAGS=-lstdc++ -lrt 

SOURCE=$(wildcard *.c)

BUILD=$(patsubst %.c,%.o,$(SOURCE))

all: $(TARGET) 

$(BUILD):$(SOURCE)
	@$(CC) -fPIC -Wall -c $^ $(INCLUDE) $(CFLAGS)
$(TARGET):$(BUILD)
	@$(CC) -shared -fPIC -o $@ $^ $(INCLUDE)
	cp $(TARGET) /usr/lib

clean:
	-@rm -f $(TARGET) *.o
	cd /usr/lib/ && rm $(TARGET)
