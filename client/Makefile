CC = gcc

TARGET=client

SRC=client.c

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) 

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

