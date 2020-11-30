# Expression v3 main Makefile
# RCS2 "$Id: Makefile,v 1.11 2005/09/08 23:37:13 liberti Exp liberti $"

CXX	 = c++
#CXXFLAGS = -g -Wall
CXXFLAGS = -O2
OBJS	= expression.o parser.o fastexpression.o
LIBS	= $(OBJS)
AR	= ar
RM	= rm

all:	libEv3.a

libEv3.a: test
	$(AR) r libEv3.a $(OBJS)

expression.o:	expression.h expression.cxx tree.cxx
	$(CXX) -c $(CXXFLAGS) -o expression.o expression.cxx

test:	main.cxx $(OBJS)
	$(CXX) $(CXXFLAGS) -o test $(LIBS) main.cxx

clean:	
	$(RM) -rf $(OBJS) test libEv3.a

distclean:	clean
	$(RM) -rf *~ core

mrproper:	distclean

%.o:	%.cxx %.h
	$(CXX) -c $(CXXFLAGS) -o $@ $<
