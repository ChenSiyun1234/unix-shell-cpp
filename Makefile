CXX ?= g++
CXXFLAGS ?= -g -Wall -Werror

wish: wish.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

test: wish
	./run_tests.sh

clean:
	rm -f wish

.PHONY: test clean
