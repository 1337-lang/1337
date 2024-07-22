CXXFLAGS += -std=c++17 -Wall -Wextra -Wpedantic -g -ggdb

all:
	mkdir -p build
	$(CXX) -o build/1337 $(CXXFLAGS) src/*.cpp $(LDFLAGS)
