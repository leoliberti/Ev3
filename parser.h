//                                               -*- C++ -*-
/**
 *  @brief C++ class that builds an Expression v3 n-ary tree from a string containing a mathematical expression in n variables
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

#ifndef __EV3PARSERH__
#define __EV3PARSERH__

#define RCS9 "$Id: parser.h,v 1.7 2006/07/30 05:36:42 liberti Exp liberti $"

// includes
#include <iostream>  // I/O
#if __GNUC__ >= 3
#include <sstream>
#else
#include <strstream>
#endif
#include <string>    // strings
#include <map>       // map
#include <cctype>    // isalpha(), etc.
#include <cmath> 

#include "expression.h"

// global declarations
//using namespace std;

enum Token_value {
  PEV3NAME, PEV3NUMBER, PEV3END, PEV3PLUS = '+', PEV3NLPLUS = '|', 
  PEV3MINUS = '-', PEV3MUL = '*', PEV3DIV = '/', PEV3POWER = '^', 
  PEV3PRINT = ';', PEV3ASSIGN = '=', PEV3LP = '(', PEV3RP = ')'
};

// constants
const double PEV3infinity = 1.0e300;
#define PEV3PI 3.1415926535897932385
#define PEV3E 2.7182818284590452354
#define PEV3NOVARIABLE -1
#define PEV3UNKNOWNVAR -2

// the Eval class
class ExpressionParser {

private:
  // user data 
  map<string, int> variable;
  map<int, string> varname;
  
  // internal parser/lexer data 
  Token_value curr_tok;
  // contains the map varnames -> varindices
  map<string, double> table;
  double number_value;
  char string_value[1024];
  int no_of_errors, no_of_functions;
  istream *input;    
  // if parser hasn't been initialized with SetVariableIDs, false
  bool isinitialized;

public:

  // constructors & destructors (public methods)
  ExpressionParser();
  ~ExpressionParser();

  // set variable ID
  void SetVariableID(string vname, int vid);

  // get variable ID
  int GetVariableID(string vname);

  // get variable name
  string GetVariableName(int vid);

  // driver evaluating routine (public method)
  Expression Parse(const char* buf, int& nerrors);

private:  
  // parser: report error (private method)
  double error(const string& s);
  double error(const string& s, Token_value tk);
  
  // parser: primary expressions (private method)
  Expression prim(bool get);

  // parser: power
  Expression power(bool get);

  // parser: products and fractions (private method)
  Expression term(bool get);

  // parser: sums and subtractions (private method)
  Expression expr(bool get);

  // lexical analyser (private method)
  Token_value get_token();

  // set variable ID (for internal use - doesn't set isinitialized)
  void InternalSetVariableID(string vname, int vid);
  
  // check whether a string is a function name or an unknown variable
  bool IsVariableName(string vname);

  // progressive vid
  int currentvid;

}; // end class ExpressionParser

#endif

