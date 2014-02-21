#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <utility>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <functional>
#include <stdexcept>

using namespace std;

struct op_t
{
	bool right;
	int prec;
	bool unary;
};

typedef pair<bool, double> return_t;
typedef vector<double> args_t;
typedef function<return_t(args_t)> func_t;
typedef pair<string, int> token_t;
typedef vector<token_t> postfix_t;

multimap<string, op_t> ops;
multimap<string, func_t> funcs;

func_t args(unsigned int n, function<double(args_t)> func)
{
	return [func, n](args_t v)
	{
		if (v.size() == n)
			return return_t(true, func(v));
		else
			return return_t(false, 0.0);
	};
}

postfix_t infix2postfix(string in)
{
	postfix_t out;
	stack<token_t> s;

	token_t lasttok;
	for (auto it = in.cbegin(); it != in.cend();)
	{
		static const string spaces = " \t\r\n";
		if (spaces.find(*it) != string::npos)
		{
			++it;
			continue;
		}

		/*cout << string(it, in.cend()) << endl;
		cout << lasttok.first << "/" << lasttok.second << endl;
		if (!s.empty())
			cout << s.top().first << "/" << s.top().second << endl;*/

		static const string numbers = "0123456789.";
		auto it2 = it;
		for (; it2 != in.cend() && numbers.find(*it2) != string::npos; ++it2);
		if (it2 != it)
		{
			if (lasttok.first == ")" || (ops.find(lasttok.first) == ops.end() && funcs.find(lasttok.first) != funcs.end()) || lasttok.second == -1)
				throw logic_error(string(it - in.cbegin() + 2, ' ') + "^\nMissing operator at " + to_string(it - in.cbegin()));

			out.push_back(lasttok = token_t(string(it, it2), -1));
			it = it2;
			continue;
		}

		bool unary = lasttok.first == "" || lasttok.first == "(" || lasttok.first == "," || ops.find(lasttok.first) != ops.end();
		/*cout << unary << endl;
		cout << endl;*/

		auto oit = ops.begin();
		for (; oit != ops.end(); ++oit)
		{
			if (equal(oit->first.begin(), oit->first.end(), it) && oit->second.unary == unary)
			{
				break;
			}
		}
		if (oit != ops.end())
		{
			if (unary)
			{
				s.push(lasttok = token_t(oit->first, 1));
			}
			else
			{
				while (!s.empty())
				{
					bool found = false;
					int tprec;
					auto range = ops.equal_range(s.top().first);
					for (auto oit2 = range.first; oit2 != range.second; ++oit2)
					{
						if (s.top().second == (oit2->second.unary ? 1 : 2))
						{
							tprec = oit2->second.prec;
							found = true;
							break;
						}
					}

					if ((found && ((!oit->second.right && oit->second.prec == tprec) || (oit->second.prec < tprec))) || (range.first == range.second && funcs.find(s.top().first) != funcs.end()))
					{
						out.push_back(s.top());
						s.pop();
					}
					else
						break;
				}
				s.push(lasttok = token_t(oit->first, 2));
			}
			it += oit->first.size();
			continue;
		}

		auto fit = funcs.begin();
		for (; fit != funcs.end(); ++fit)
		{
			if (ops.find(fit->first) == ops.end() && equal(fit->first.begin(), fit->first.end(), it))
			{
				break;
			}
		}
		if (fit != funcs.end())
		{
			if (lasttok.second == -1 || lasttok.first == ")")
				throw logic_error(string(it - in.cbegin() + 2, ' ') + "^\nMissing operator at " + to_string(it - in.cbegin()));

			s.push(lasttok = token_t(fit->first, 0));
			it += fit->first.size();
			continue;
		}

		if (*it == ',')
		{
			if (lasttok.first == "(" || lasttok.first == ",")
				throw logic_error(string(it - in.cbegin() + 2, ' ') + "^\nMissing argument at " + to_string(it - in.cbegin()));

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
				throw invalid_argument(string(it - in.cbegin() + 2, ' ') + "^\nFound ',' not inside function arguments at " + to_string(it - in.cbegin()));

			s.top().second++;
			lasttok = token_t(",", 0);
			++it;
			continue;
		}

		if (*it == '(')
		{
			if (lasttok.second == -1)
				throw logic_error(string(it - in.cbegin() + 2, ' ') + "^\nMissing operator at " + to_string(it - in.cbegin()));

			s.push(lasttok = token_t("(", 1));
			++it;
			continue;
		}

		if (*it == ')')
		{
			if (lasttok.first == "(" || lasttok.first == ",")
				throw logic_error(string(it - in.cbegin() + 2, ' ') + "^\nMissing argument at " + to_string(it - in.cbegin()));

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
				throw logic_error(string(it - in.cbegin() + 2, ' ') + "^\nFound excess '(' at " + to_string(it - in.cbegin()));

			token_t tok = s.top();
			s.pop();

			if (!s.empty() && ops.find(s.top().first) == ops.end() && funcs.find(s.top().first) != funcs.end())
			{
				out.push_back(token_t(s.top().first, tok.second));
				s.pop();
			}

			lasttok = token_t(")", 0);
			++it;
			continue;
		}

		throw logic_error(string(it - in.cbegin() + 2, ' ') + "^\nUnknown token found at " + to_string(it - in.cbegin()));
	}

	while (!s.empty())
	{
		token_t tok = s.top();
		s.pop();
		if (tok.first == "(")
			throw logic_error(string(in.size() + 2, ' ') + "^\nFound unclosed '(' at " + to_string(in.size()));
		out.push_back(tok);
	}

	return out;
}

double evalpostfix(postfix_t in)
{
	stack<double> s;
	for (token_t &tok : in)
	{
		if (tok.second == -1)
			s.push(stod(tok.first));
		else
		{
			if (s.size() < tok.second)
				throw invalid_argument("Not enough arguments (have " + to_string(s.size()) + ") for function '" + tok.first + "' (want " + to_string(tok.second) + ")");
			else
			{
				args_t v;
				for (int i = 0; i < tok.second; i++)
				{
					v.insert(v.begin(), s.top());
					s.pop();
				}

				auto range = funcs.equal_range(tok.first);
				auto it = range.first;
				return_t ret(false, 0);
				for (; it != range.second; ++it)
				{
					ret = it->second(v);
					if (ret.first)
						break;
				}

				if (ret.first)
					s.push(ret.second);
				else
					throw domain_error("Unacceptable arguments for function '" + tok.first + "'");
			}
		}
	}

	if (s.size() == 1)
		return s.top();
	else
		throw logic_error("No single result found");
}

int main()
{
	ops.insert(make_pair("+", op_t{false, 1, false}));
	ops.insert(make_pair("-", op_t{false, 1, false}));
	ops.insert(make_pair("*", op_t{false, 2, false}));
	ops.insert(make_pair("/", op_t{false, 2, false}));
	ops.insert(make_pair("%", op_t{false, 2, false}));
	ops.insert(make_pair("^", op_t{true, 3, false}));
	ops.insert(make_pair("+", op_t{false, 10, true}));
	ops.insert(make_pair("-", op_t{false, 10, true}));

	funcs.insert(make_pair("+", args(1, [](vector<double> v)
	{
		return v[0];
	})));
	funcs.insert(make_pair("+", args(2, [](vector<double> v)
	{
		return v[0] + v[1];
	})));
	funcs.insert(make_pair("-", args(1, [](vector<double> v)
	{
		return -v[0];
	})));
	funcs.insert(make_pair("-", args(2, [](vector<double> v)
	{
		return v[0] - v[1];
	})));
	funcs.insert(make_pair("*", args(2, [](vector<double> v)
	{
		return v[0] * v[1];
	})));
	funcs.insert(make_pair("/", args(2, [](vector<double> v)
	{
		return v[0] / v[1];
	})));
	funcs.insert(make_pair("%", args(2, [](vector<double> v)
	{
		return fmod(v[0], v[1]);
	})));
	funcs.insert(make_pair("^", args(2, [](vector<double> v)
	{
		return pow(v[0], v[1]);
	})));
	funcs.insert(make_pair("abs", args(1, [](vector<double> v)
	{
		return abs(v[0]);
	})));
	funcs.insert(make_pair("log", [](vector<double> v)
	{
		if (v.size() == 1)
			return make_pair(true, log10(v[0]));
		else if (v.size() == 2)
			return make_pair(true, log(v[1]) / log(v[0]));
		else
			return make_pair(false, 0.0);
	}));
	funcs.insert(make_pair("sqrt", args(1, [](vector<double> v)
	{
		return sqrt(v[0]);
	})));
	funcs.insert(make_pair("min", [](vector<double> v)
	{
		if (v.size() > 0)
			return make_pair(true, *min_element(v.begin(), v.end()));
		else
			return make_pair(false, 0.0);
	}));
	funcs.insert(make_pair("max", [](vector<double> v)
	{
		if (v.size() > 0)
			return make_pair(true, *max_element(v.begin(), v.end()));
		else
			return make_pair(false, 0.0);
	}));
	funcs.insert(make_pair("pi", args(0, [](vector<double> v)
	{
		return M_PI;
	})));
	funcs.insert(make_pair("e", args(0, [](vector<double> v)
	{
		return M_E;
	})));

	string exp;
	while (cout << "> ", getline(cin, exp))
	{
		try
		{
			auto postfix = infix2postfix(exp);
			for (auto &tok : postfix)
				cout << tok.first << "/" << tok.second << " ";
			cout << endl;
			cout << evalpostfix(postfix) << endl;
		}
		catch (exception &e)
		{
			cout << e.what() << endl;
		}
		cout << endl;
	}
	return 0;
}
