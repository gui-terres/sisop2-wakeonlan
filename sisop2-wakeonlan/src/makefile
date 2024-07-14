# Compiler
CC = g++

# Path's variables
SUBSYSTEMS = subsystems
DISCOVERY = $(SUBSYSTEMS)/discovery
INTERFACE = $(SUBSYSTEMS)/interface

# Files to be compiled
# You can add a new file by appending its path to the SOURCES list
SOURCES = main.cpp $(DISCOVERY)/server.cpp $(DISCOVERY)/client.cpp $(DISCOVERY)/station.cpp $(INTERFACE)/interface.cpp

# .exe name
TARGET = main

# Compile all SOURCES files
$(TARGET): $(SOURCES)
	$(CC) -std=c++11 -pthread $(SOURCES) -o sleep_server

# Clean generated files
clean:
	rm -f $(TARGET)
