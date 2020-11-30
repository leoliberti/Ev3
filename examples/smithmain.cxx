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
#include <sys/time.h>
#include <cstdlib>

#define BUFSIZE 8192
#define DEFAULTVALUE 100

int main(int argc, char** argv) {

  ExpressionParser p;
  Expression e, f;
  int nerr;

  // e = p.Parse("(x-2)^2 + (y-3)^2 + (z+1)^2", nerr);  
  // e=p.Parse("sin (13*(x-y)) * (x*x + y*exp (x)) + 3 * log (1 + x^4)", nerr);
  // e = p.Parse("(x1+x3)*x2 - 2*x1*(x3-1) +5*x1 + 3*x2 + x3^2", nerr);
  e = p.Parse("(x+y)*(x-z)*x*z^2", nerr);
  Simplify(&e);
  cout << e->ToString() << endl;

  int vi = 3;
  string vn = "w";
  
  //Expression x(1, 1, "x1"), y(1, 2, "x2");
  //Expression schema = x * y;

  // replace by sum
  f = e->ReplaceByOperator(vi, vn, SUM);
  while(!f->IsZero()) {
    cout << vn << "[" << vi << "] = " << f->ToString() << endl;
    vi++;
    f = e->ReplaceByOperator(vi, vn, SUM);
  }
  vi++;

  // replace by product
  f = e->ReplaceByOperator(vi, vn, PRODUCT);
  while(!f->IsZero()) {
    cout << vn << "[" << vi << "] = " << f->ToString() << endl;
    vi++;
    f = e->ReplaceByOperator(vi, vn, PRODUCT);
  }

  // replace by power operator
  f = e->ReplaceByOperator(vi, vn, POWER);
  while(!f->IsZero()) {
    cout << vn << "[" << vi << "] = " << f->ToString() << endl;
    vi++;
    f = e->ReplaceByOperator(vi, vn, POWER);
  }

  // replace by operand
  Expression x(1,1,"x");
  x->SetExponent(2);
  // replace by power operator
  f = e->ReplaceBySchema(vi, vn, x);
  while(!f->IsZero()) {
    cout << vn << "[" << vi << "] = " << f->ToString() << endl;
    vi++;
    f = e->ReplaceBySchema(vi, vn, x);
  }
  
  cout << e->ToString() << endl;

  return 0;
}
