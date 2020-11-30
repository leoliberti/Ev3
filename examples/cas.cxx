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
#include <map>
#include <string>
#include <cstring>

#define BUFSIZE 8192
#define MAXVARS 1000

enum { EVALUATE, DIFFERENTIATE, SIMPLIFY, ASSIGN, UNKNOWN };

int main(int argc, char** argv) {

  char buffer[BUFSIZE];
  char bufsave[BUFSIZE];
  char *tok;
  char *nexttok;
  int cmd;

  double varvalues[MAXVARS];

  ExpressionParser p;
  Expression e, f, d;
  map<string, Expression> store;
  int nerr;

  while(true) {
    cout << "Ev3> ";
    cin >> buffer;
    strncpy(bufsave, buffer, BUFSIZE);
    tok = buffer;
    // parse the command 
    if (strncmp(buffer, "quit", 4) == 0) {
      break;
    } else if (strncmp(tok, "eval", 4) == 0) {
      cmd = EVALUATE;
      nexttok = strchr(tok, '(');
      if (!nexttok) {
	cerr << "syntax error: was expecting a (\n";
	continue;
      } else {
	tok = nexttok + 1;
	nexttok = strchr(tok, ',');
	if (!nexttok) {
	  cerr << "syntax error: was expecting a ,\n";
	  continue;
	} else {
	  // delimit tok
	  *nexttok = '\0';	  
	  string t(tok);
	  if (store.find(t) != store.end()) {
	    // string t (i.e. tok) was in the store
	    e = store[t];
	  } else {
	    // not in the store, parse it in
	    e = p.Parse(tok, nerr);
	  }
	  int vv = e->NumberOfVariables();
	  // "undelimit"
	  *nexttok = ',';
	  tok = nexttok + 1;
	  // read in variable values
	  int i = 0;
	  bool errflag = false;
	  while(nexttok) {
	    if (i >= vv) {
	      break;
	    }
	    tok = nexttok + 1;
	    if (i == vv - 1) {
	      nexttok = strchr(tok, ')');
	    } else {
	      nexttok = strchr(tok, ',');
	    }
	    if (!nexttok) {
	      cerr << "can't evaluate, not enough values: " << i + 1 
		   << " out of " << vv << " needed\n";
	      errflag = true;
	      break;
	    }
	    *nexttok = '\0';
	    varvalues[i] = atof(tok);
	    i++;
	    *nexttok = ',';
	  }
	  if (errflag) {
	    continue;
	  }
	  cout << "  " << e->Eval(varvalues, vv) << endl;
	}
      }      
    } else if (strncmp(tok, "diff", 4) == 0) {
      cmd = DIFFERENTIATE;
    } else if (strncmp(tok, "simp", 4) == 0) {
      cmd = SIMPLIFY;
    } else if (strncmp(tok, "let", 3) == 0) {
      cmd = ASSIGN;      
    } else {
      cmd = UNKNOWN;      
    }

    /*
    bool ischanged;
    cout << "D(" << e->ToString();
    f = SimplifyCopy(&e, ischanged);
    cout << " --> " << f->ToString();
    d = Diff(f, 1);
    cout << ", var1) = " << d->ToString() << endl;
    */
  }

  return 0;
}
