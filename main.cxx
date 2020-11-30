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
  
  
  e = p.Parse(argv[1], nerr);  

  cout << "Parsed expression: " << e->ToString() << endl;

  Simplify(&e);

  cout << "Simplified expression: " << e->ToString() << endl;

  return 0;

  int n = DEFAULTVALUE;
  if (argc > 1) {
    n = atoi(argv[1]);
    if (n == 0) {
      n = DEFAULTVALUE;
    }
  }

  // initialize randomizer
  struct timeval theTV;
  struct timezone theTZ;
  gettimeofday(&theTV, &theTZ);
  srandom(theTV.tv_usec);
  
  double x[3];
  double t;
  cout << "x\t\ty\t\tz\t\tf\tfeval\n";
  for(int i = 0; i < n; i++) {
    x[0] = (double) random() / (double) RAND_MAX;
    x[1] = (double) random() / (double) RAND_MAX;
    x[2] = (double) random() / (double) RAND_MAX;
    t = e->Eval(x, 3);
    cout << x[0] << "\t" << x[1] << "\t" << x[2] << "\t" << t;
    t = e->FastEval(x, 3);
    cout << "\t" << t << endl;
  }

  return 0;
}
