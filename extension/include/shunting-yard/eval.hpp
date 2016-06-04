/* Simple expression evaluator, forked from Simmo Saan's original code.
 * - rlyeh, mit licensed.

 * The MIT License (MIT)
 * 
 * Copyright (c) 2014 Simmo Saan
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#define EVAL_VERSION "1.0.1" /* (2016/03/26): Add simple support for assignment and variables
#define EVAL_VERSION "1.0.0" // (2016/02/20): Extra math stuff; Header-only now; Initial SemVer adherence */

#include <cmath>

#include <algorithm>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>

// public api: evaluate string
double eval( const std::string &expr, std::string *err = 0 );
void extend( const std::string &expr, std::string *err = 0 );

// private api: (based on calculate.hpp/.cpp)
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <functional>
#include <stdexcept>
#include <sstream>
#include <stack>

struct evaluator {

    struct oper_t
    {
        bool right;
        int prec;
        bool unary;
    };

    typedef long double num_t;
    typedef std::pair<bool, num_t> return_t;
    typedef std::vector<num_t> args_t;
    typedef std::function<return_t(args_t)> func_t;
    typedef std::pair<std::string, int> token_t;
    typedef std::vector<token_t> postfix_t;

    class parse_error : public std::runtime_error
    {
    public:
        parse_error(const std::string &what_arg, unsigned int i_arg) : runtime_error(what_arg), i(i_arg) {}

        unsigned int index() const
        {
            return i;
        }
    protected:
        unsigned int i;
    };

    struct funcname_compare
    {
        bool operator() (const std::string &lhs, const std::string &rhs) const
        {
            if (equal(lhs.begin(), lhs.end(), rhs.begin())) // lhs is prefix of rhs
                return false;
            else if (equal(rhs.begin(), rhs.end(), lhs.begin())) // rhs is prefix of lhs
                return true;
            else
                return lhs < rhs;
        }
    };

    std::multimap<std::string, oper_t> opers;
    std::multimap<std::string, func_t, funcname_compare> funcs;

    /// function proxy to force specific number of arguments for it
    func_t func_args(unsigned int n, std::function<num_t(args_t)> func)
    {
        return [func, n](args_t v)
        {
            if (v.size() == n)
                return return_t(true, func(v));
            else
                return return_t(false, 0.0);
        };
    }

    /// function proxy to make a constant returning function
    func_t func_constant(num_t c)
    {
        return func_args(0, [c](args_t v)
        {
            return c;
        });
    }

    /// pop element from stack and return it
    /// std::stack<T>::pop() is stupid and returns void
    template<typename T>
    T pop(std::stack<T> &s)
    {
        T val = s.top();
        s.pop();
        return val;
    }

    /// insert binary operator specified by oit into Shunting-Yard
    void insert_binaryoper(postfix_t &out, std::stack<token_t> &s, decltype(opers)::iterator oit)
    {
        while (!s.empty())
        {
            bool found = false; // stack top element is operator
            int tprec; // prec of stack top element
            auto range = opers.equal_range(s.top().first);
            for (auto oit2 = range.first; oit2 != range.second; ++oit2)
            {
                if (s.top().second == (oit2->second.unary ? 1 : 2)) // find the correct arity version of the operator
                {
                    tprec = oit2->second.prec;
                    found = true;
                    break;
                }
            }

            if ((found && ((!oit->second.right && oit->second.prec == tprec) || (oit->second.prec < tprec))) || (!found && funcs.find(s.top().first) != funcs.end()))
                out.push_back(pop(s));
            else
                break;
        }
        s.push(token_t(oit->first, 2));
    }

    /// find operator with specific string and arity
    decltype(opers)::iterator find_oper(const std::string &str, bool unary)
    {
        auto range = opers.equal_range(str);
        auto oit = range.first;
        for (; oit != range.second; ++oit)
        {
            if (oit->second.unary == unary)
                break;
        }
        return oit == range.second ? opers.end() : oit;
    }

    /// insert implicit multiplication at current state
    void insert_implicitmult(postfix_t &out, std::stack<token_t> &s)
    {
        auto oit = find_oper("*", false);
        if (oit != opers.end()) // if binary multiplication operator exists
            insert_binaryoper(out, s, oit);
    }

    /// convert infix string into postfix token list
    postfix_t infix2postfix(const std::string &in)
    {
        postfix_t out;
        std::stack<token_t> s;

        token_t lasttok;
        for (auto it = in.cbegin(); it != in.cend();)
        {
            const unsigned int i = it - in.cbegin(); // index of current character for parse_error purposes

            static const std::string spaces = " \t\r\n";
            if (spaces.find(*it) != std::string::npos)
            {
                ++it;
                continue;
            }

            /*cout << string(it, in.cend()) << endl;
            cout << lasttok.first << "/" << lasttok.second << endl;
            if (!s.empty())
                cout << s.top().first << "/" << s.top().second << endl;*/

            // try to parse number
            static const std::string numbers = "0123456789.";
            auto it2 = it;
            for (; it2 != in.cend() && numbers.find(*it2) != std::string::npos; ++it2); // TODO: find_first_not_of
            if (it2 != it)
            {
                if (lasttok.first == ")" || (opers.find(lasttok.first) == opers.end() && funcs.find(lasttok.first) != funcs.end()) || lasttok.second == -1)
                    throw parse_error("Missing operator", i);

                out.push_back(lasttok = token_t(std::string(it, it2), -1));
                it = it2;
                continue;
            }


            // try to parse operator
            auto lastoper = opers.find(lasttok.first);
            bool lunary = lasttok.first == "" || lasttok.first == "(" || lasttok.first == "," || (lastoper != opers.end() && !(lastoper->second.unary && lastoper->second.right)); // true if operator at current location would be left unary
            /*cout << unary << endl;
            cout << endl;*/

            auto oit = opers.begin();
            for (; oit != opers.end(); ++oit)
            {
                if (equal(oit->first.begin(), oit->first.end(), it) && (oit->second.unary == lunary || (oit->second.unary && oit->second.right)))
                    break;
            }
            if (oit != opers.end())
            {
                if (lunary)
                {
                    s.push(lasttok = token_t(oit->first, 1));
                }
                else if (oit->second.unary && oit->second.right) // right unary operator
                {
                    // allow right unary operators to be used on constants and apply higher prec operators before
                    while (!s.empty())
                    {
                        token_t tok = s.top();

                        auto oit2 = find_oper(tok.first, true);
                        if ((oit2 != opers.end() && oit2->second.prec > oit->second.prec) || (oit2 == opers.end() && funcs.find(tok.first) != funcs.end()))
                            out.push_back(pop(s));
                        else
                            break;
                    }
                    out.push_back(lasttok = token_t(oit->first, 1)); // needs stack popping before?
                }
                else
                {
                    insert_binaryoper(out, s, oit);
                    lasttok = token_t(oit->first, 2);
                }
                it += oit->first.size();
                continue;
            }


            // try to parse function
            auto fit = funcs.begin();
            for (; fit != funcs.end(); ++fit)
            {
                if (opers.find(fit->first) == opers.end() && equal(fit->first.begin(), fit->first.end(), it))
                    break;
            }
            if (fit != funcs.end())
            {
                if (lasttok.first == ")" || (opers.find(lasttok.first) == opers.end() && funcs.find(lasttok.first) != funcs.end()))
                    throw parse_error("Missing operator", i);
                else if (lasttok.second == -1)
                    insert_implicitmult(out, s);

                s.push(lasttok = token_t(fit->first, 0));
                it += fit->first.size();
                continue;
            }

            // try to parse function argument separator
            if (*it == ',')
            {
                if (lasttok.first == "(" || lasttok.first == ",")
                    throw parse_error("Missing argument", i);

                bool found = false;
                while (!s.empty())
                {
                    token_t tok = s.top();

                    if (tok.first == "(")
                    {
                        found = true;
                        break;
                    }
                    else
                    {
                        out.push_back(tok);
                        s.pop();
                    }
                }

                if (!found)
                    throw parse_error("Found ',' not inside function arguments", i);

                s.top().second++; // increase number of arguments in current parenthesis
                lasttok = token_t(",", 0);
                ++it;
                continue;
            }

            if (*it == '(')
            {
                if (lasttok.second == -1 || lasttok.first == ")")
                    insert_implicitmult(out, s);

                s.push(lasttok = token_t("(", 1));
                ++it;
                continue;
            }

            if (*it == ')')
            {
                if (lasttok.first == "(" || lasttok.first == ",")
                    throw parse_error("Missing argument", i);

                bool found = false;
                while (!s.empty())
                {
                    token_t tok = s.top();
                    if (tok.first == "(")
                    {
                        found = true;
                        break;
                    }
                    else
                    {
                        out.push_back(tok);
                        s.pop();
                    }
                }

                if (!found)
                    throw parse_error("Found excess '('", i);

                token_t tok = pop(s); // pop '('

                if (!s.empty() && opers.find(s.top().first) == opers.end() && funcs.find(s.top().first) != funcs.end()) // if parenthesis part of function arguments
                    out.push_back(token_t(pop(s).first, tok.second));

                lasttok = token_t(")", 0);
                ++it;
                continue;
            }

            throw parse_error("Unknown token found", i);
        }

        while (!s.empty())
        {
            token_t tok = pop(s);
            if (tok.first == "(")
                throw parse_error("Found unclosed '('", in.size());
            out.push_back(tok);
        }

        return out;
    }

    /// evaluate postfix expression
    num_t evalpostfix(postfix_t in)
    {
        std::stack<num_t> s;
        for (token_t &tok : in)
        {
            if (tok.second == -1) // number
                s.push(stod(tok.first));
            else
            {
                if (s.size() < tok.second)
                    throw std::runtime_error("Not enough arguments (have " + std::to_string(s.size()) + ") for function '" + tok.first + "' (want " + std::to_string(tok.second) + ")");
                else
                {
                    args_t v;
                    for (int i = 0; i < tok.second; i++)
                        v.insert(v.begin(), pop(s)); // pop elements for function arguments in reverse order

                    auto range = funcs.equal_range(tok.first);
                    return_t ret(false, 0);
                    for (auto it = range.first; it != range.second; ++it)
                    {
                        ret = it->second(v);
                        if (ret.first) // find a function that can evaluate given parameters
                            break;
                    }

                    if (ret.first)
                        s.push(ret.second);
                    else
                    {
                        std::ostringstream args; // stringstream because to_string adds trailing zeroes
                        for (auto vit = v.begin(); vit != v.end(); ++vit) // construct exception argument list
                        {
                            args << *vit;
                            if ((vit + 1) != v.end())
                                args << ", ";
                        }
                        throw std::runtime_error("Unacceptable arguments (" + args.str() + ") for function '" + tok.first + "'");
                    }
                }
            }
        }

        if (s.size() == 1)
            return s.top();
        else
            throw std::runtime_error("No single result found");
    }
};

// Eval implementation follows

struct evaluator_extra : public evaluator {

    evaluator_extra() {
        opers.insert({"+", oper_t{false, 1, false}});
        opers.insert({"-", oper_t{false, 1, false}});
        opers.insert({"*", oper_t{false, 2, false}});
        opers.insert({"/", oper_t{false, 2, false}});
        opers.insert({"%", oper_t{false, 2, false}});
        opers.insert({"^", oper_t{true, 3, false}});
        opers.insert({"+", oper_t{false, 10, true}});
        opers.insert({"-", oper_t{false, 10, true}});
        opers.insert({"!", oper_t{true, 11, true}});

        funcs.insert({"+", func_args(1, [](args_t v)
        {
            return v[0];
        })});
        funcs.insert({"+", func_args(2, [](args_t v)
        {
            return v[0] + v[1];
        })});
        funcs.insert({"-", func_args(1, [](args_t v)
        {
            return -v[0];
        })});
        funcs.insert({"-", func_args(2, [](args_t v)
        {
            return v[0] - v[1];
        })});
        funcs.insert({"*", func_args(2, [](args_t v)
        {
            return v[0] * v[1];
        })});
        funcs.insert({"/", func_args(2, [](args_t v)
        {
            return v[1] != 0 ? v[0] / v[1] : std::numeric_limits<double>::quiet_NaN();
        })});
        funcs.insert({"%", func_args(2, [](args_t v)
        {
            return fmod(v[0], v[1]);
        })});
        funcs.insert({"^", func_args(2, [](args_t v)
        {
            return pow(v[0], v[1]);
        })});
        funcs.insert({"abs", func_args(1, [](args_t v)
        {
            return abs(v[0]);
        })});
        funcs.insert({"log", func_args(1, [](args_t v)
        {
            return log10(v[0]);
        })});
        funcs.insert({"log", func_args(2, [](args_t v)
        {
            return log(v[1]) / log(v[0]);
        })});
        funcs.insert({"ln", func_args(1, [](args_t v)
        {
            return log(v[0]);
        })});
        funcs.insert({"sqrt", func_args(1, [](args_t v)
        {
            return sqrt(v[0]);
        })});
        funcs.insert({"root", func_args(2, [](args_t v)
        {
            return pow(v[1], 1.0 / v[0]);
        })});
        funcs.insert({"sin", func_args(1, [](args_t v)
        {
            return sin(v[0]);
        })});
        funcs.insert({"cos", func_args(1, [](args_t v)
        {
            return cos(v[0]);
        })});
        funcs.insert({"tan", func_args(1, [](args_t v)
        {
            return tan(v[0]);
        })});
        funcs.insert({"asin", func_args(1, [](args_t v)
        {
            return asin(v[0]);
        })});
        funcs.insert({"acos", func_args(1, [](args_t v)
        {
            return acos(v[0]);
        })});
        funcs.insert({"atan", func_args(1, [](args_t v)
        {
            return atan(v[0]);
        })});
        funcs.insert({"atan2", func_args(2, [](args_t v)
        {
            return atan2(v[0], v[1]);
        })});
        funcs.insert({"ceil", func_args(1, [](args_t v)
        {
            return ceil(v[0]);
        })});
        funcs.insert({"floor", func_args(1, [](args_t v)
        {
            return floor(v[0]);
        })});
        funcs.insert({"min", [](args_t v)
        {
            if (v.size() > 0)
                return return_t(true, *min_element(v.begin(), v.end()));
            else
                return return_t(false, 0.0);
        }});
        funcs.insert({"max", [](args_t v)
        {
            if (v.size() > 0)
                return return_t(true, *max_element(v.begin(), v.end()));
            else
                return return_t(false, 0.0);
        }});
        funcs.insert({"!", func_args(1, [](args_t v)
        {
            return tgamma(v[0] + 1);
        })});

        funcs.insert({"true", func_constant(1)});
        funcs.insert({"false", func_constant(0)});
        funcs.insert({"on", func_constant(1)});
        funcs.insert({"off", func_constant(0)});

        funcs.insert({"pi", func_constant(acos(-1.L))});
        funcs.insert({"e", func_constant(exp(1.L))});
        funcs.insert({"nan", func_constant(NAN)});
        funcs.insert({"_", func_constant(NAN)});
    }

    std::set<std::string> variables;

    double eval( const std::string &expr, std::string *err ) {
        try
        {
            // handle var=expr {
            for( auto end = expr.size(), pos = end - end; pos < end; ++pos ) {
                const auto &ch = expr[pos];
                if( ch == '=' && pos > 0 ) {
                    auto key = expr.substr(0, pos);
                    auto val = eval(expr.substr(pos + 1), 0);
                    auto f = funcs.find( key );
                    if( f != funcs.end() ) f->second = func_constant(val);
                    else funcs.insert( {key, func_constant(val) } ), variables.insert( key );
                    return val;
                }
            }
            // }
            auto postfix = infix2postfix(expr);
            /*for (auto &tok : postfix)
                cout << tok.first << "/" << tok.second << " ";
            cout << std::endl;*/
            auto value = evalpostfix(postfix);
            funcs.find("_")->second = func_constant(value);
            if(err) err->clear();
            return value;
        }
        catch (parse_error &e)
        {
            std::stringstream cerr;
            cerr << std::string(e.index() + 2, ' ') << "^" << std::endl;
            cerr << e.what() << " at " << e.index() << std::endl;
            if( err ) *err = cerr.str();
        }
        catch (std::runtime_error &e)
        {
            std::stringstream cerr;
            cerr << e.what() << std::endl;
            if( err ) *err = cerr.str();
        }
        catch (std::exception &e)
        {
            std::stringstream cerr;
            cerr << "Internal error: " << e.what() << std::endl;
            if( err ) *err = cerr.str();
        }
        return std::numeric_limits<double>::quiet_NaN();
    }
};

inline
evaluator_extra &eval_get_singleton() {
    static evaluator_extra ev;
    return ev;
}

inline
double eval( const std::string &expr, std::string *err ) {
    return eval_get_singleton().eval( expr, err );
}



#ifdef EVAL_BUILD_DEMO

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "eval.hpp"

void tests() {

    std::stringstream right, wrong;

    #define test1(A) [&]() { auto _A_ = (A); if( _A_ != decltype(A)(0) ) \
        return right << "[ OK ] " __FILE__ ":" << __LINE__ << " -> " #A " -> " << _A_ << std::endl, true; else \
        return wrong << "[FAIL] " __FILE__ ":" << __LINE__ << " -> " #A " -> " << _A_ << std::endl, false; \
    }()

    #define test3(A,op,B) [&]() { auto _A_ = (A); auto _B_ = (B); if( _A_ op _B_ ) \
        return right << "[ OK ] " __FILE__ ":" << __LINE__ << " -> " #A " " #op " " #B " -> " << _A_ << " " #op " " << _B_ << std::endl, true; else \
        return wrong << "[FAIL] " __FILE__ ":" << __LINE__ << " -> " #A " " #op " " #B " -> " << _A_ << " " #op " " << _B_ << std::endl, false; \
    }()

    #define testN(NaN)    test3(NaN,!=,NaN)

    /* eval() */
    double val1 = eval("5*(4+4+1)");      // ->  45
    assert( val1 == 45 );

    double val2 = eval("-5*(2*(1+3)+1)"); // -> -45
    assert( val2 == -45 );

    double val3 = eval("5*((1+3)*2+1)");  // ->  45
    assert( val3 == 45 );

    test3( eval("(2+3)*2"), ==, 10 );

    // Some simple expressions
    test3( eval("1234"), ==, 1234 );
    test3( eval("1+2*3"), ==, 7 );

    // Parenthesis
    test3( eval("5*(4+4+1)"), ==, 45 );
    test3( eval("5*(2*(1+3)+1)"), ==, 45 );
    test3( eval("5*((1+3)*2+1)"), ==, 45 );

    // Spaces
    test3( eval("5 * ((1 + 3) * 2 + 1)"), ==, 45 );
    test3( eval("5 - 2 * ( 3 )"), ==, -1 );
    test3( eval("5 - 2 * ( ( 4 )  - 1 )"), ==, -1 );

    // Sign before parenthesis
    test3( eval("-(2+1)*4"), ==, -12 );
    test3( eval("-4*(2+1)"), ==, -12 );

    // Fractional numbers
    test3( eval("1.5/5"), ==, 0.3 );
    //test3( eval("1/5e10"), ==, 2e-11 );
    test3( eval("(4-3)/(4*4)"), ==, 0.0625 );
    test3( eval("1/2/2"), ==, 0.25 );
    test3( eval("0.25 * .5 * 0.5"), ==, 0.0625 );
    test3( eval(".25 / 2 * .5"), ==, 0.0625 );

    // Repeated operators
    test3( eval("1+-2"), ==, -1 );
    test3( eval("--2"), ==, 2 );
    test3( eval("2---2"), ==, 0 );
    test3( eval("2-+-2"), ==, 4 );

    // Check for parenthesis error
    testN( eval("5*((1+3)*2+1") );
    testN( eval("5*((1+3)*2)+1)") );

    // Check for repeated operators
    testN( eval("5*/2") );

    // Check for wrong positions of an operator
    testN( eval("*2") );
    testN( eval("2+") );
    testN( eval("2*") );

    // Check for divisions by zero
    testN( eval("2/0") );
    testN( eval("3+1/(5-5)+4") );
    testN( eval("2/") ); // Erroneously detected as division by zero, but that's ok for us

    // Check for invalid characters
    testN( eval("~5") );
    testN( eval("5x") );

    // Check for multiply errors
    testN( eval("3+1/0+4$") ); // Only one error will be detected (in this case, the last one)
    testN( eval("3+1/0+4") );
    testN( eval("q+1/0)") ); // ...or the first one
    testN( eval("+1/0)") );
    testN( eval("+1/0") );

    // Check for emtpy string
    testN( eval("") );

    // Check assignment and variables
    test3( eval("A=1"), ==, 1 );
    test3( eval("A=A+1"), ==, 2 );
    test3( eval("B=2"), ==, 2 );
    test3( eval("A*B"), ==, 4 );

    std::cout << right.str();
    std::cout << wrong.str();
}

int main() {
    tests();

    std::string exp;
    while (std::cout << "> ", getline(std::cin, exp) && exp != "!") {
        std::string err;
        auto value = eval(exp, &err);
        if( err.empty() )
            std::cout << std::setprecision(std::numeric_limits<decltype(value)>::digits10) << value << std::endl;
        else
            std::cout << err << std::endl;
    }

    for( auto &var : eval_get_singleton().variables ) {
        std::cout << var << "=" << eval( var ) << std::endl;
    }
}

#endif
