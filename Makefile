CXX=g++
CXXFLAGS=-Wall -std=c++23 -lfmt -ggdb 

ps: pscan.cpp pscan.hpp
	$(CXX) $(CXXFLAGS) pscan.cpp -o ps $(D)

test: pscan.cpp pscan.hpp
	$(CXX) $(CXXFLAGS) pscan.cpp -o ps -DTEST

run:
	./ps

clean:
	rm -rf ps
