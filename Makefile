CXX=g++
CXXFLAGS=-pthread -Wall -std=c++20 -lfmt -ggdb

ps: pscan.cpp
	$(CXX) $(CXXFLAGS) pscan.cpp -o ps

run:
	./ps
