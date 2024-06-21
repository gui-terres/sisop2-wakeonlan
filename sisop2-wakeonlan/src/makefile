# Compiler
CC = g++

# Path's variables
SUBSYSTEMS = subsystems
DISCOVERY = $(SUBSYSTEMS)/discovery
INTERFACE = $(SUBSYSTEMS)/interface

# Files to be compiled
# You can add a new file by appending its path to the SOURCES list
SOURCES = main.cpp $(DISCOVERY)/server.cpp $(DISCOVERY)/client.cpp $(INTERFACE)/interface.cpp

# .exe name
TARGET = main

# Compile all SOURCES files
$(TARGET): $(SOURCES)
	$(CC) $(SOURCES) -o $(TARGET)

# Clean generated files
clean:
	rm -f $(TARGET)
