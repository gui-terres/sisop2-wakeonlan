# Compiler
CC = g++

# Path's variables
STATIONS = stations
SUBSYSTEMS = subsystems
DISCOVERY = $(SUBSYSTEMS)/discovery
INTERFACE = $(SUBSYSTEMS)/interface
MANAGEMENT = $(SUBSYSTEMS)/management
MONITORING = $(SUBSYSTEMS)/monitoring

# Files to be compiled
# You can add a new file by appending its path to the SOURCES list
# SOURCES = main.cpp $(DISCOVERY)/server.cpp $(DISCOVERY)/client.cpp $(DISCOVERY)/station.cpp $(INTERFACE)/interface.cpp
SOURCES = main.cpp $(STATIONS)/server.cpp $(STATIONS)/client.cpp $(STATIONS)/station.cpp $(DISCOVERY)/discovery.cpp $(INTERFACE)/interface.cpp $(MANAGEMENT)/management.cpp $(MONITORING)/monitoring.cpp

# .exe name
TARGET = main

# Compile all SOURCES files
$(TARGET): $(SOURCES)
# $(CC) -std=c++11 -pthread $(SOURCES) -lncurses -o sleep_server
	$(CC) -std=c++11 -pthread $(SOURCES) -o sleep_server

# Clean generated files
clean:
	rm -f $(TARGET)
