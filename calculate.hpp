#ifndef CALCULATE_H
#define CALCULATE_H

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <functional>
#include <stdexcept>

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

extern std::multimap<std::string, oper_t> opers;
extern std::multimap<std::string, func_t, funcname_compare> funcs;

postfix_t infix2postfix(std::string in);
num_t evalpostfix(postfix_t in);

/// function proxy to force specific number of arguments for it
inline func_t func_args(unsigned int n, std::function<num_t(args_t)> func)
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
inline func_t func_constant(num_t c)
{
	return func_args(0, [c](args_t v)
	{
		return c;
	});
}

#endif // CALCULATE_H
