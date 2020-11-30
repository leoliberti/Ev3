//                                               -*- C++ -*-
/**
 *  @brief Expression v3 exceptions
 *
 *  Copyright (C) 2008-2010 Leo Liberti
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __EV3EXCEPTIONSH__
#define __EV3EXCEPTIONSH__

#define RCS8 "$Id: exceptions.h,v 1.5 2003/10/08 11:13:58 liberti Exp liberti $"

#define HELPURL "http://liberti.dhs.org/"
#define NONE "[none]"
#define STDACTION cerr << interface << "::" << scope << ": in [" << operation << "]: " << description << ", code = " << code << ", see " << moreinfo << endl

class ErrBase {
 public:
  unsigned long code;
  string interface;
  string scope;
  string operation;
  string description;
  string moreinfo;
  ErrBase() :
    code(0), interface(NONE), scope(NONE), operation(NONE), 
    description(NONE),  moreinfo(HELPURL) { STDACTION; }
  ErrBase(unsigned long mycode,
	  string myif,
	  string myscope,
	  string myop,
	  string mydesc,
	  string myinfo) :
    code(mycode), interface(myif), scope(myscope), operation(myop), 
    description(mydesc), moreinfo(myinfo) { STDACTION; }
};

class ErrUnknown : public ErrBase { 
 public:
  ErrUnknown(unsigned long mycode,
	     string myif,
	     string myscope,
	     string myop,
	     string mydesc,
	     string myinfo) : 
    ErrBase(mycode, myif, myscope, myop, mydesc, myinfo) { STDACTION; }
};

class ErrNotPermitted : public ErrBase { 
 public:
  ErrNotPermitted(unsigned long mycode,
		  string myif,
		  string myscope,
		  string myop,
		  string mydesc,
		  string myinfo) :
    ErrBase(mycode, myif, myscope, myop, mydesc, myinfo) { STDACTION; }
};

class ErrDivideByZero : public ErrBase { 
 public:
  string dividend;
  ErrDivideByZero(unsigned long mycode,
		  string myif,
		  string myscope,
		  string myop,
		  string mydesc,
		  string myinfo, 
		  string mydiv) : 
    ErrBase(mycode, myif, myscope, myop, mydesc, myinfo),
    dividend(mydiv) { STDACTION; }
};

#endif
