#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <algorithm>
#include <functional>

using namespace std;

struct Op
{
	bool right;
	int prec;
	//function<double(double, double)> func;
};

map<string, Op> ops;
map<string, function<double(vector<double>)>> funcs;

vector<string> infix2postfix(string in)
{
	vector<string> out;
	stack<string> s;

	for (string::const_iterator it = in.cbegin(); it != in.cend();)
	{
		static string spaces = " \t\r\n";
		if (spaces.find(*it) != string::npos)
		{
			++it;
			continue;
		}

		static string numbers = "0123456789.";
		string::const_iterator it2;
		for (it2 = it; it2 != in.cend() && numbers.find(*it2) != string::npos; ++it2);
		if (it2 != it)
		{
			out.push_back(string(it, it2));
			it = it2;
			continue;
		}

		decltype(funcs)::iterator fit;
		for (fit = funcs.begin(); fit != funcs.end(); ++fit)
		{
			if (equal(fit->first.begin(), fit->first.end(), it))
			{
				break;
			}
		}
		if (fit != funcs.end())
		{
			s.push(fit->first);
			it += fit->first.size();
			continue;
		}

		if (*it == ',')
		{
			bool found = false;
			while (!s.empty())
			{
				string tok = s.top();
				s.pop();
				if (tok == "(")
				{
					found = true;
					break;
				}
				else
					out.push_back(tok);
			}

			if (!found)
				cout << "ERROR" << endl;

			++it;
			continue;
		}

		decltype(ops)::iterator oit;
		for (oit = ops.begin(); oit != ops.end(); ++oit)
		{
			if (equal(oit->first.begin(), oit->first.end(), it))
			{
				break;
			}
		}
		if (oit != ops.end())
		{
			while (!s.empty() && ops.find(s.top()) != ops.end() && ((!oit->second.right && oit->second.prec == ops[s.top()].prec) || (oit->second.prec < ops[s.top()].prec)))
			{
				out.push_back(s.top());
				s.pop();
			}
			s.push(oit->first);
			it += oit->first.size();
			continue;
		}

		if (*it == '(')
		{
			s.push("(");
			++it;
			continue;
		}

		if (*it == ')')
		{
			bool found = false;
			while (!s.empty())
			{
				string tok = s.top();
				s.pop();
				if (tok == "(")
				{
					found = true;
					break;
				}
				else
					out.push_back(tok);
			}

			if (!found)
				cout << "ERROR" << endl;

			if (!s.empty() && funcs.find(s.top()) != funcs.end())
			{
				out.push_back(s.top());
				s.pop();
			}

			++it;
			continue;
		}

		cout << "ERROR" << endl;
		return vector<string>();
	}

	while (!s.empty())
	{
		string tok = s.top();
		s.pop();
		if (tok == "(")
			cout << "ERROR" << endl;
		out.push_back(tok);
	}

	return out;
}

int main()
{
	ops["+"] = Op{false, 1};
	ops["-"] = Op{false, 1};
	ops["*"] = Op{false, 2};
	ops["/"] = Op{false, 2};
	ops["^"] = Op{true, 3};

	string exp;
    while (cout << "> ", getline(cin, exp))
	{
		auto postfix = infix2postfix(exp);
		for (auto &tok : postfix)
			cout << tok << " ";
		cout << endl;
		cout << endl;
	}
    return 0;
}
