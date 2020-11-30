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

#include "parser.h"
#include <cstring>

#define NOTVARNAME "_var_not_found_"

#define RCS10 "$Id: parser.cxx,v 1.11 2006/07/30 05:36:43 liberti Exp liberti $"

ExpressionParser::ExpressionParser() { 
  isinitialized = false; 
  currentvid = 1;
}

ExpressionParser::~ExpressionParser() { }

// set variable ID
void ExpressionParser::SetVariableID(string vname, int vid) {
  isinitialized = true;
  variable[vname] = vid;
  varname[vid] = vname;
}

// set variable ID for internal use
void ExpressionParser::InternalSetVariableID(string vname, int vid) {
  variable[vname] = vid;
  varname[vid] = vname;
}

// get variable ID
int ExpressionParser::GetVariableID(string vname) {
  if (variable.find(vname) != variable.end())
    return variable[vname];
  else {
    if (IsVariableName(vname)) {
      return PEV3UNKNOWNVAR;
    } else {
      return PEV3NOVARIABLE;
    }
  }
}

// get variable name
string ExpressionParser::GetVariableName(int vid) {
  if (varname.find(vid) != varname.end()) 
    return varname[vid];
  else
    return NOTVARNAME;
}

bool ExpressionParser::IsVariableName(string vname) {  
  if (vname == "sin" ||
      vname == "cos" ||
      vname == "tan" ||
      vname == "cot" ||
      vname == "exp" ||
      vname == "log" ||
      vname == "sinh" ||
      vname == "cosh" ||
      vname == "tanh" ||
      vname == "sqrt") {
    return false;
  } else {
    return true;
  }
}

// driver evaluating routine (public method)
Expression ExpressionParser::Parse(const char* buf, int& nerrors) {
    
  curr_tok = PEV3PRINT;
  Expression ret;
#if __GNUC__ >= 3
  input = new stringstream(buf);
#else
  input = new strstream(buf, sizeof(buf));
#endif
  no_of_functions = 0;
  no_of_errors = 0;
  
  table["pi"] = PEV3PI;
  table["e"] = PEV3E;
  
  while (*input) {
    get_token();
    switch(curr_tok) {
    case PEV3END:
      break;
    case PEV3PRINT:
      continue;
    case PEV3RP:
      if (no_of_functions == 0) {
	error("primary expected, found", curr_tok);
      } else {
	no_of_functions--;
      }
      continue;
    default:
      ret = expr(false);
    }
  }

  delete input;
  nerrors = no_of_errors;
  return ret;
}

// parser: report error (private method)
double ExpressionParser::error(const string& s) {
  no_of_errors++;
  cerr << "error: " << s << endl;
  return 0;
}
double ExpressionParser::error(const string& s, Token_value tk) {
  no_of_errors++;
  char tks = tk;
  cerr << "error: " << s << " " << tk << ":" << tks << endl;
  return 0;
}
  
// parser: primary expressions (private method)
Expression ExpressionParser::prim(bool get) {
    
  Expression ret;
  
  if (get)
    get_token();
  
  switch (curr_tok) {
  case PEV3NUMBER:
    {
      ret = number_value;
      get_token();
    }
    break;
  case PEV3NAME:
    {
      int vid = GetVariableID(string_value);      
      string vn = GetVariableName(vid);
      double v = table[string_value];
      if (v != 0) {
	ret = v;
	get_token();
      } else if (vid == PEV3UNKNOWNVAR && !isinitialized) {
	InternalSetVariableID(string_value, currentvid);
	ret->SetOpType(VAR);
	ret->SetVarIndex(currentvid);
	ret->SetVarName(string_value);
	currentvid++;
	get_token();
      } else if (vid != PEV3NOVARIABLE) {
	ret->SetOpType(VAR);
	ret->SetVarIndex(vid);
	ret->SetVarName(vn);
	get_token();
      } else {
	Token_value tk = get_token();
	if (tk == PEV3LP) {
	  string s(string_value);
	  string::iterator si(s.begin());
	  while (si != s.end()) {
	    *si = tolower(*si);
	    si++;
	  }
	  no_of_functions++;
	  ret = expr(true);
	  //cerr << "ExpressionParser::prim: s = " << s << endl;
	  if (s == "sin") {
	    ret = SinLink(ret);
	  } else if (s == "cos") {
	    ret = CosLink(ret);
	  } else if (s == "tan") {
	    ret = TanLink(ret);
	  } else if (s == "cot") {
	    ret = CotLink(ret);
	  } else if (s == "log") {
	    ret = LogLink(ret);
	  } else if (s == "exp") {
	    ret = ExpLink(ret);
	  } else if (s == "sinh") {
	    ret = SinhLink(ret);
	  } else if (s == "cosh") {
	    ret = CoshLink(ret);
	  } else if (s == "tanh") {
	    ret = TanhLink(ret);
	  } else if (s == "coth") {
	    ret = CothLink(ret);
	  } else if (s == "sqrt") {
	    ret = SqrtLink(ret);
	  } else {
	    error("unknown function");
	  }
	  if (curr_tok != PEV3RP)
	    error("bracket ) expected for end-of-function");
	  else {
	    no_of_functions--;
	    get_token();
	  }
	}
      }
    }
    break;
  case PEV3MINUS:
    ret = MinusLink(prim(true));
    break;
  case PEV3LP:
    {
      ret = expr(true);
      if (curr_tok != PEV3RP)
	error("bracket ) expected");
      else 
	get_token();	
    }     
    break;
  default:
    error("primary expected, found", curr_tok);
  }

  return ret;
}     

// parser: power
Expression ExpressionParser::power(bool get) {    
  Expression ret = prim(get);
  for (;;) {
    switch (curr_tok) {
    case PEV3POWER:
      ret = PowerLink(ret, prim(true));
      break;
    default:
      return ret;
    }
  }
}

// parser: products and fractions (private method)
Expression ExpressionParser::term(bool get) {
  
  Expression ret = power(get);
  
  for (;;) {
    switch (curr_tok) {
    case PEV3MUL:
      ret = ProductLink(ret, power(true));
      break;
    case PEV3DIV:
      ret = FractionLink(ret, power(true));
      break;
    default:
      return ret;
    }
  }
}

// parser: sums and subtractions (private method)
Expression ExpressionParser::expr(bool get) {
  
  Expression ret = term(get);
  
  for (;;) {
    switch (curr_tok) {
    case PEV3PLUS: case PEV3NLPLUS:
      ret = SumLink(ret, term(true));
      break;
    case PEV3MINUS:
      ret = DifferenceLink(ret, term(true));
      break;
    default:
      return ret;
    }
  }
}

// lexical analyser (private method)
Token_value ExpressionParser::get_token() {
    
  char ch;
  
  do { // skip whitespace except '\n' 
    if (!input->get(ch))
      return curr_tok = PEV3END;
  } while (ch != '\n' && isspace(ch));
  
  switch(ch) {
  case ';': case '\n':
    return curr_tok = PEV3PRINT;
  case 0:
    return curr_tok = PEV3END;
  case '*': case '/': case '+': case '-': case '|': 
  case '(': case ')': case '=':
  case '^':
    return curr_tok = Token_value(ch);
  case '0': case '1': case '2': case '3': case '4': case '5':
  case '6': case '7': case '8': case '9': case '.':
    input->putback(ch);
    *input >> number_value;
    return curr_tok = PEV3NUMBER;
  default:  // PEV3NAME, PEV3NAME=, or error
    if (isalpha(ch)) {
      char* sv = string_value;
      *sv = ch;
      while (input->get(ch) && (isalnum(ch) || ch == '_')) {
	*(++sv) = ch;
      }
      *(++sv) = '\0';
      input->putback(ch);
      return curr_tok = PEV3NAME;
    }
    error("bad token");
    return curr_tok = PEV3PRINT;
  }
}
