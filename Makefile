CXX=g++
CXXFLAGS=-Wall -std=c++20 -lfmt -ggdb -pthread

ps: pscan.cpp pscan.hpp
	$(CXX) $(CXXFLAGS) pscan.cpp -o ps

run:
	./ps

clean:
	rm -rf ps
