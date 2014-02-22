#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>
#include "calculate.hpp"

using namespace std;

int main()
{
	opers.insert(make_pair("+", oper_t{false, 1, false}));
	opers.insert(make_pair("-", oper_t{false, 1, false}));
	opers.insert(make_pair("*", oper_t{false, 2, false}));
	opers.insert(make_pair("/", oper_t{false, 2, false}));
	opers.insert(make_pair("%", oper_t{false, 2, false}));
	opers.insert(make_pair("^", oper_t{true, 3, false}));
	opers.insert(make_pair("+", oper_t{false, 10, true}));
	opers.insert(make_pair("-", oper_t{false, 10, true}));

	funcs.insert(make_pair("+", func_args(1, [](args_t v)
	{
		return v[0];
	})));
	funcs.insert(make_pair("+", func_args(2, [](args_t v)
	{
		return v[0] + v[1];
	})));
	funcs.insert(make_pair("-", func_args(1, [](args_t v)
	{
		return -v[0];
	})));
	funcs.insert(make_pair("-", func_args(2, [](args_t v)
	{
		return v[0] - v[1];
	})));
	funcs.insert(make_pair("*", func_args(2, [](args_t v)
	{
		return v[0] * v[1];
	})));
	funcs.insert(make_pair("/", func_args(2, [](args_t v)
	{
		return v[0] / v[1];
	})));
	funcs.insert(make_pair("%", func_args(2, [](args_t v)
	{
		return fmod(v[0], v[1]);
	})));
	funcs.insert(make_pair("^", func_args(2, [](args_t v)
	{
		return pow(v[0], v[1]);
	})));
	funcs.insert(make_pair("abs", func_args(1, [](args_t v)
	{
		return abs(v[0]);
	})));
	funcs.insert(make_pair("log", [](args_t v)
	{
		if (v.size() == 1)
			return return_t(true, log10(v[0]));
		else if (v.size() == 2)
			return return_t(true, log(v[1]) / log(v[0]));
		else
			return return_t(false, 0.0);
	}));
	funcs.insert(make_pair("sqrt", func_args(1, [](args_t v)
	{
		return sqrt(v[0]);
	})));
	funcs.insert(make_pair("min", [](args_t v)
	{
		if (v.size() > 0)
			return return_t(true, *min_element(v.begin(), v.end()));
		else
			return return_t(false, 0.0);
	}));
	funcs.insert(make_pair("max", [](args_t v)
	{
		if (v.size() > 0)
			return return_t(true, *max_element(v.begin(), v.end()));
		else
			return return_t(false, 0.0);
	}));
	funcs.insert(make_pair("pi", func_constant(acos(-1.L))));
	funcs.insert(make_pair("e", func_constant(exp(1.L))));
	funcs.insert(make_pair("_", func_constant(NAN)));

	string exp;
	while (cout << "> ", getline(cin, exp))
	{
		try
		{
			auto postfix = infix2postfix(exp);
			for (auto &tok : postfix)
				cout << tok.first << "/" << tok.second << " ";
			cout << endl;
			auto value = evalpostfix(postfix);
			cout << setprecision(numeric_limits<decltype(value)>::digits10) << value << endl;
			funcs.find("_")->second = func_constant(value);
		}
		catch (parse_error &e)
		{
			cout << string(e.index() + 2, ' ') << "^" << endl;
			cout << e.what() << " at " << e.index() << endl;
		}
		catch (exception &e)
		{
			cout << e.what() << endl;
		}
		cout << endl;
	}
	return 0;
}
