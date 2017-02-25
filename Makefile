#CXX = ~/tableau-cache/devtools/clang/3.9.1.170216225523/bin/clang++
CXXFLAGS = -fPIC -Wall -Werror -std=c++1y -stdlib=libc++ -I./include 

all: lib test

lib: src/teenypath.cpp
	mkdir -p lib
	$(CXX) $(CXXFLAGS) -shared -o lib/libteenypath.so $^

test: src/teenypath.cpp test/teenypathtest.cpp
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -o bin/test_teenypath.o $^

.PHONY: clean lib test
clean:
	rm -rf bin
	rm -rf lib
