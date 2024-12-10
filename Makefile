# Makefile for the smash program
CXX = g++
CXXLINK = $(CXX)
CXXFLAGS = -g -Wall -std=c++11
OBJS = smash.o commands.o signals.o classes.o
RM = rm -rf
# Creating the executable
smash: $(OBJS)
	$(CXXLINK) -o smash $(OBJS) 
	$(RM) $(OBJS)
# Creating the object files
commands.o: commands.cpp commands.h
	$(CXX) $(CXXFLAGS) -c commands.cpp
smash.o: smash.cpp commands.h
	$(CXX) $(CXXFLAGS) -c smash.cpp
signals.o: signals.cpp signals.h
	$(CXX) $(CXXFLAGS) -c signals.cpp
classes.o: classes.cpp classes.h
	$(CXX) $(CXXFLAGS) -c classes.cpp
# Cleaning old files before new make
clean:
	$(RM) $(OBJS) *.o *~ "#"* core.* 