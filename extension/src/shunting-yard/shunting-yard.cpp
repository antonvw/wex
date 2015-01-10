// Source: http://www.daniweb.com/software-development/cpp/code/427500/calculator-using-shunting-yard-algorithm#
// Author: Jesse Brown
// Modifications: Brandon Amos

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <shunting-yard/shunting-yard.h>

#define isvariablechar(c) (isalpha(c) || c == '_')
TokenQueue_t calculator::toRPN(const char* expr,
    std::map<std::string, double>* vars,
    std::map<std::string, int> opPrecedence) {
  TokenQueue_t rpnQueue; std::stack<std::string> operatorStack;
  bool lastTokenWasOp = true;

  // In one pass, ignore whitespace and parse the expression into RPN
  // using Dijkstra's Shunting-yard algorithm.
  while (*expr && isspace(*expr )) ++expr;
  while (*expr ) {
    if (isdigit(*expr )) {
      // If the token is a number, add it to the output queue.
      char* nextChar = 0;
      double digit = strtod(expr , &nextChar);
#     ifdef DEBUG
        std::cout << digit << std::endl;
#     endif
      rpnQueue.push(new Token<double>(digit));
      expr = nextChar;
      lastTokenWasOp = false;
    } else if (isvariablechar(*expr )) {
      // If the function is a variable, resolve it and
      // add the parsed number to the output queue.
      if (!vars) {
        throw std::domain_error(
            "Detected variable, but the variable map is null.");
      }

      std::stringstream ss;
      ss << *expr;
      ++expr;
      while (isvariablechar(*expr )) {
        ss << *expr;
        ++expr;
      }
      std::string key = ss.str();
      std::map<std::string, double>::iterator it = vars->find(key);
      if (it == vars->end()) {
        throw std::domain_error(
            "Unable to find the variable '" + key + "'.");
      }
      double val = vars->find(key)->second;
#     ifdef DEBUG
        std::cout << val << std::endl;
#     endif
      rpnQueue.push(new Token<double>(val));;
      lastTokenWasOp = false;
    } else {
      // Otherwise, the variable is an operator or paranthesis.
      switch (*expr) {
        case '(':
          operatorStack.push("(");
          ++expr;
          break;
        case ')':
          while (operatorStack.top().compare("(")) {
            rpnQueue.push(new Token<std::string>(operatorStack.top()));
            operatorStack.pop();
          }
          operatorStack.pop();
          ++expr;
          break;
        default:
          {
            // The token is an operator.
            //
            // Let p(o) denote the precedence of an operator o.
            //
            // If the token is an operator, o1, then
            //   While there is an operator token, o2, at the top
            //       and p(o1) <= p(o2), then
            //     pop o2 off the stack onto the output queue.
            //   Push o1 on the stack.
            std::stringstream ss;
            ss << *expr;
            ++expr;
            while (*expr && !isspace(*expr ) && !isdigit(*expr )
                && !isvariablechar(*expr) && *expr != '(' && *expr != ')') {
              ss << *expr;
              ++expr;
            }
            ss.clear();
            std::string str;
            ss >> str;
#           ifdef DEBUG
              std::cout << str << std::endl;
#           endif

            if (lastTokenWasOp) {
              // Convert unary operators to binary in the RPN.
              if (!str.compare("-") || !str.compare("+")) {
                rpnQueue.push(new Token<double>(0));
              } else {
                throw std::domain_error(
                    "Unrecognized unary operator: '" + str + "'.");
              }
            }

            while (!operatorStack.empty() &&
                opPrecedence[str] <= opPrecedence[operatorStack.top()]) {
              rpnQueue.push(new Token<std::string>(operatorStack.top()));
              operatorStack.pop();
            }
            operatorStack.push(str);
            lastTokenWasOp = true;
          }
      }
    }
    while (*expr && isspace(*expr )) ++expr;
  }
  while (!operatorStack.empty()) {
    rpnQueue.push(new Token<std::string>(operatorStack.top()));
    operatorStack.pop();
  }
  return rpnQueue;
}

double calculator::calculate(const char* expr,
    std::map<std::string, double>* vars) {
  // 1. Create the operator precedence map.
  std::map<std::string, int> opPrecedence;
  opPrecedence["("] = -1;
  opPrecedence["<<"] = 1; opPrecedence[">>"] = 1;
  opPrecedence["+"]  = 2; opPrecedence["-"]  = 2;
  opPrecedence["*"]  = 3; opPrecedence["/"]  = 3;
  opPrecedence["^"]  = 1; 
  opPrecedence["|"]  = 1; 
  opPrecedence["&"]  = 1; 
  opPrecedence["%"]  = 1; 

  // 2. Convert to RPN with Dijkstra's Shunting-yard algorithm.
  TokenQueue_t rpn = toRPN(expr, vars, opPrecedence);

  // 3. Evaluate the expression in RPN form.
  std::stack<double> evaluation;
  while (!rpn.empty()) {
    TokenBase* base = rpn.front();
    rpn.pop();

    Token<std::string>* strTok = dynamic_cast<Token<std::string>*>(base);
    Token<double>* doubleTok = dynamic_cast<Token<double>*>(base);
    if (strTok) {
      std::string str = strTok->val;
      if (evaluation.size() < 2) {
        throw std::domain_error("Invalid equation.");
      }
      double right = evaluation.top(); evaluation.pop();
      double left  = evaluation.top(); evaluation.pop();
      if (!str.compare("+")) {
        evaluation.push(left + right);
      } else if (!str.compare("*")) {
        evaluation.push(left * right);
      } else if (!str.compare("-")) {
        evaluation.push(left - right);
      } else if (!str.compare("/")) {
        if (right == 0)
        {
          throw std::domain_error("Divide by zero");
        }
        evaluation.push(left / right);
      } else if (!str.compare("<<")) {
        evaluation.push((int) left << (int) right);
      } else if (!str.compare(">>")) {
        evaluation.push((int) left >> (int) right);
      } else if (!str.compare("^")) {
        evaluation.push((int) left ^ (int) right);
      } else if (!str.compare("|")) {
        evaluation.push((int) left | (int) right);
      } else if (!str.compare("&")) {
        evaluation.push((int) left & (int) right);
      } else if (!str.compare("%")) {
        evaluation.push((int) left % (int) right);
      } else {
        throw std::domain_error("Unknown operator: '" + str + "'.");
      }
    } else if (doubleTok) {
      evaluation.push(doubleTok->val);
    } else {
      throw std::domain_error("Invalid token.");
    }
    delete base;
  }
  return evaluation.top();
}
