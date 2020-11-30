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

  ExpressionParser p;
  Expression e;
  int nerr;

  string es = "-x1^2 -x2^2 - x3^2 - x1*x2";
  e = p.Parse(es.c_str(), nerr); 
  Simplify(&e);
  cout << es << " simplifies to: " << endl;
  cout << "  " << e->ToString() << endl;
  cout << "derivatives: " << endl;
  cout << "  w.r.t. x1 = " << Diff(e, 1)->ToString() << endl;
  cout << "  w.r.t. x2 = " << Diff(e, 2)->ToString() << endl;
  cout << "  w.r.t. x3 = " << Diff(e, 3)->ToString() << endl;

  return 0;
}
