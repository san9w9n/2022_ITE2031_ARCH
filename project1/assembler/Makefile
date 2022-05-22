CC = gcc
OBJS = assemble.o
TARGET = assemble
 
.SUFFIXES : .c .o
 
all : $(TARGET)
 
$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)
 
clean :
	rm -f $(OBJS) $(TARGET)