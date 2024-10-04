CC = gcc

TARGET=server

SRC=server.c

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) 

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

