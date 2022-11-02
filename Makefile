CC = gcc#aarch64-himix100-linux-gcc
CFLAGS  = -Wall -g -fPIC -shared -o
LIBS 	= -lpthread
LDFLAGS = -L./lib
INC     = -I ./inc
SRC     = $(wildcard ./src/*.c)
OBJS    = $(patsubst ./src/%.c,./obj/%.o,$(SRC))

TARGET  = rtsp_server
TARGET_LIB = ./lib/librtsp_server.so

ALL: $(TARGET) $(OBJS)

$(TARGET):$(TARGET_LIB)
	$(CC) main.c -o $@ -lrtsp_server $(LDFLAGS) $(INC)

$(TARGET_LIB):$(SRC)
	$(CC) $(CFLAGS) $@ $^ $(INC) $(LIBS)

#$$(OBJS):$(SRC)
$(OBJS):./obj/%.o:./src/%.c
	$(CC) -c $< -o $@ $(LIBS) $(INC)

.PHONY:clean
clean:
	rm -rf $(OBJS) $(TARGET_LIB) $(TARGET)
