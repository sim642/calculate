#include "calculate.hpp"
#include <sstream>
#include <stack>

using namespace std;

multimap<string, oper_t> opers;
multimap<string, func_t> funcs;

void insert_binaryoper(postfix_t &out, stack<token_t> &s, decltype(opers)::iterator oit)
{
	while (!s.empty())
	{
		bool found = false;
		int tprec;
		auto range = opers.equal_range(s.top().first);
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
	s.push(token_t(oit->first, 2));
}

void insert_implicitmult(postfix_t &out, stack<token_t> &s)
{
	auto range = opers.equal_range("*");
	auto oit = range.first;
	for (; oit != range.second; ++oit)
	{
		if (oit->second.unary == false)
		{
			break;
		}
	}
	if (oit != range.second)
	{
		insert_binaryoper(out, s, oit);
	}
}

postfix_t infix2postfix(string in)
{
	postfix_t out;
	stack<token_t> s;

	token_t lasttok;
	for (auto it = in.cbegin(); it != in.cend();)
	{
		const unsigned int i = it - in.cbegin();

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
			if (lasttok.first == ")" || (opers.find(lasttok.first) == opers.end() && funcs.find(lasttok.first) != funcs.end()) || lasttok.second == -1)
				throw parse_error("Missing operator", i);

			out.push_back(lasttok = token_t(string(it, it2), -1));
			it = it2;
			continue;
		}

		bool unary = lasttok.first == "" || lasttok.first == "(" || lasttok.first == "," || opers.find(lasttok.first) != opers.end();
		/*cout << unary << endl;
		cout << endl;*/

		auto oit = opers.begin();
		for (; oit != opers.end(); ++oit)
		{
			if (equal(oit->first.begin(), oit->first.end(), it) && oit->second.unary == unary)
			{
				break;
			}
		}
		if (oit != opers.end())
		{
			if (unary)
			{
				s.push(lasttok = token_t(oit->first, 1));
			}
			else
			{
				insert_binaryoper(out, s, oit);
				lasttok = token_t(oit->first, 2);
			}
			it += oit->first.size();
			continue;
		}

		auto fit = funcs.begin();
		for (; fit != funcs.end(); ++fit)
		{
			if (opers.find(fit->first) == opers.end() && equal(fit->first.begin(), fit->first.end(), it))
			{
				break;
			}
		}
		if (fit != funcs.end())
		{
			if (lasttok.first == ")")
				throw parse_error("Missing operator", i);
			else if (lasttok.second == -1)
				insert_implicitmult(out, s);

			s.push(lasttok = token_t(fit->first, 0));
			it += fit->first.size();
			continue;
		}

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

			s.top().second++;
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

			token_t tok = s.top();
			s.pop();

			if (!s.empty() && opers.find(s.top().first) == opers.end() && funcs.find(s.top().first) != funcs.end())
			{
				out.push_back(token_t(s.top().first, tok.second));
				s.pop();
			}

			lasttok = token_t(")", 0);
			++it;
			continue;
		}

		throw parse_error("Unknown token found", i);
	}

	while (!s.empty())
	{
		token_t tok = s.top();
		s.pop();
		if (tok.first == "(")
			throw parse_error("Found unclosed '('", in.size());
		out.push_back(tok);
	}

	return out;
}

num_t evalpostfix(postfix_t in)
{
	stack<num_t> s;
	for (token_t &tok : in)
	{
		if (tok.second == -1)
			s.push(stod(tok.first));
		else
		{
			if (s.size() < tok.second)
				throw runtime_error("Not enough arguments (have " + to_string(s.size()) + ") for function '" + tok.first + "' (want " + to_string(tok.second) + ")");
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
				{
					ostringstream args; // stringstream because to_string adds trailing zeroes
					for (auto vit = v.begin(); vit != v.end(); ++vit)
					{
						args << *vit;
						if ((vit + 1) != v.end())
							args << ", ";
					}
					throw runtime_error("Unacceptable arguments (" + args.str() + ") for function '" + tok.first + "'");
				}
			}
		}
	}

	if (s.size() == 1)
		return s.top();
	else
		throw runtime_error("No single result found");
}
