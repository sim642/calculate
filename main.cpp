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
	opers.insert({"+", oper_t{false, 1, false}});
	opers.insert({"-", oper_t{false, 1, false}});
	opers.insert({"*", oper_t{false, 2, false}});
	opers.insert({"/", oper_t{false, 2, false}});
	opers.insert({"%", oper_t{false, 2, false}});
	opers.insert({"^", oper_t{true, 3, false}});
	opers.insert({"+", oper_t{false, 10, true}});
	opers.insert({"-", oper_t{false, 10, true}});

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
		return v[0] / v[1];
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
	funcs.insert({"pi", func_constant(acos(-1.L))});
	funcs.insert({"e", func_constant(exp(1.L))});
	funcs.insert({"_", func_constant(NAN)});

	string exp;
	while (cout << "> ", getline(cin, exp))
	{
		try
		{
			auto postfix = infix2postfix(exp);
			/*for (auto &tok : postfix)
				cout << tok.first << "/" << tok.second << " ";
			cout << endl;*/
			auto value = evalpostfix(postfix);
			cout << setprecision(numeric_limits<decltype(value)>::digits10) << value << endl;
			funcs.find("_")->second = func_constant(value);
		}
		catch (parse_error &e)
		{
			cerr << string(e.index() + 2, ' ') << "^" << endl;
			cerr << e.what() << " at " << e.index() << endl;
		}
		catch (runtime_error &e)
		{
			cerr << e.what() << endl;
		}
		catch (exception &e)
		{
			cerr << "Internal error: " << e.what() << endl;
		}
		cout << endl;
	}
	return 0;
}
