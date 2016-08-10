TARGET = libatbm.so

INCLUDE=-I./Server

LIB=-L../

CFLAGS=-lstdc++ -lrt -l9331

SOURCE=$(wildcard *.c)

BUILD=$(patsubst %.c,%.o,$(SOURCE))

all: $(TARGET) 

$(BUILD):$(SOURCE)
	@$(CC) -fPIC -Wall -c $^ $(INCLUDE) $(LIB) $(CFLAGS)
$(TARGET):$(BUILD)
	@$(CC) -shared -fPIC -o $@ $^ $(INCLUDE)
	cp $(TARGET) /usr/lib

clean:
	-@rm -f $(TARGET) *.o
	cd /usr/lib && rm $(TARGET)
