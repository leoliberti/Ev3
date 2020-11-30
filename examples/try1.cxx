//                                               -*- C++ -*-
/**
 *  @brief main driver program for Expression v3 
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

#include "expression.h"
#include "parser.h"

#include <iostream>
#include <cstring>

#define BUFSIZE 8192

int main(int argc, char** argv) {

  char buffer[BUFSIZE];

  ExpressionParser p;
  Expression e, f;
  int nerr;
  bool ischanged;

  e = p.Parse("x+y*z-2*x+x", nerr);  
  f = p.Parse("z*y", nerr);

  Simplify(&e);
  Simplify(&f);

  cout << e->ToString() << endl;
  cout << f->ToString() << endl;
  cout << (e == f) << endl;

  return 0;
}
