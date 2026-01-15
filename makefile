# Makefile for Irrigator project

CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++11 -g

# Object files
OBJS = irrigator.o

# Default driver build (if you have a main.cpp driver)
driver: $(OBJS) main.cpp
	$(CXX) $(CXXFLAGS) $(OBJS) main.cpp -o main
	./main

# Build irrigator object
irrigator.o: irrigator.cpp irrigator.h
	$(CXX) $(CXXFLAGS) -c irrigator.cpp

# Unit test suite (mytest.cpp)
test: $(OBJS) mytest.cpp
	$(CXX) $(CXXFLAGS) $(OBJS) mytest.cpp -o test
	./test testDefaultConstructor
	./test testParameterizedConstructor
	./test testCopyConstructor
	./test testAssignmentOperator
	./test testInsertCrop
	./test testGetNextCrop
	./test testMergeWithQueue
	./test testClear
	./test testNumCrops
	./test testPrintCropsQueue
	./test testSetPriorityFn
	./test testSetStructure
	./test testDump
	./test testGetRegPrior

# Compile only (no run)
compile: $(OBJS) mytest.cpp
	$(CXX) $(CXXFLAGS) $(OBJS) mytest.cpp -o test

# Memory check with valgrind
testMemory: $(OBJS) mytest.cpp
	$(CXX) $(CXXFLAGS) $(OBJS) mytest.cpp -o test
	valgrind --leak-check=yes ./test

# Clean up
clean:
	rm -f *.o main test