CC = gcc
OBJS = cache.o
TARGET = cache
 
.SUFFIXES : .c .o
 
all : $(TARGET)
 
$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)
 
clean :
	rm -f $(OBJS) $(TARGET)