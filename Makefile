CXX = g++
CXXFLAGS = -std=c++11 -Wall
INCLUDE = -Iinclude
SRC = src/config.cpp src/database.cpp src/package.cpp src/util.cpp src/main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = lxpkg

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

install:
	cp $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)
