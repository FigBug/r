#
# Makefile: r
#
OBJS=block_allocator.o json.o mainheaders.o r.o tinystr.o tinyxml.o tinyxmlerror.o tinyxmlparser.o ultragetopt.o url.o
CFLAGS=-g -Wall
CC=gcc
CXX=g++

# all
all: r

# clean
clean: 
	rm -f $(OBJS) r 

# r
r: $(OBJS)
	$(CXX) $(CFLAGS) -lcurl -framework CoreFoundation -framework ApplicationServices -o r $(OBJS)

block_allocator.o: block_allocator.cpp
	$(CXX) $(CFLAGS) -c block_allocator.cpp

json.o: json.cpp
	$(CXX) $(CFLAGS) -c json.cpp

mainheaders.o: mainheaders.cpp
	$(CXX) $(CFLAGS) -c mainheaders.cpp

r.o: r.cpp
	$(CXX) $(CFLAGS) -c r.cpp

tinystr.o: tinystr.cpp
	$(CXX) $(CFLAGS) -c tinystr.cpp

tinyxml.o: tinyxml.cpp
	$(CXX) $(CFLAGS) -c tinyxml.cpp

tinyxmlerror.o: tinyxmlerror.cpp
	$(CXX) $(CFLAGS) -c tinyxmlerror.cpp

tinyxmlparser.o: tinyxmlparser.cpp
	$(CXX) $(CFLAGS) -c tinyxmlparser.cpp

ultragetopt.o: ultragetopt.c
	$(CC) $(CFLAGS) -c ultragetopt.c

url.o: url.cpp
	$(CXX) $(CFLAGS) -c url.cpp


