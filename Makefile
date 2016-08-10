TARGET = lib9331.so

#CC=/disk4/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin/mips-openwrt-linux-uclibc-gcc

INCLUDE=-I./Server -I./TsParse

CFLAGS=-lstdc++ -lrt 

LIBPATH=-L./cxd2837 -L./cxd2872 -L./GaoTuo -L./TsParse

LIB=-lcxd2837 -lcxd2872  -latbm -ltsparse

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