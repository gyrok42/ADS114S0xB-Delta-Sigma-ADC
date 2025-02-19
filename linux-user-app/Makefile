CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2
LDFLAGS := -liio
SRC := user-space-app.cpp
TARGET := user-space-app

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
