#include "calculate.hpp"
#include <sstream>
#include <stack>

using namespace std;

multimap<string, oper_t> opers;
multimap<string, func_t, funcname_compare> funcs;

/// pop element from stack and return it
/// std::stack<T>::pop() is stupid and returns void
template<typename T>
T pop(stack<T> &s)
{
	T val = s.top();
	s.pop();
	return val;
}

/// insert binary operator specified by oit into Shunting-Yard
void insert_binaryoper(postfix_t &out, stack<token_t> &s, decltype(opers)::iterator oit)
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
decltype(opers)::iterator find_oper(const string &str, bool unary)
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
void insert_implicitmult(postfix_t &out, stack<token_t> &s)
{
	auto oit = find_oper("*", false);
	if (oit != opers.end()) // if binary multiplication operator exists
		insert_binaryoper(out, s, oit);
}

/// convert infix string into postfix token list
postfix_t infix2postfix(string in)
{
	postfix_t out;
	stack<token_t> s;

	token_t lasttok;
	for (auto it = in.cbegin(); it != in.cend();)
	{
		const unsigned int i = it - in.cbegin(); // index of current character for parse_error purposes

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

		// try to parse number
		static const string numbers = "0123456789.";
		auto it2 = it;
		for (; it2 != in.cend() && numbers.find(*it2) != string::npos; ++it2); // TODO: find_first_not_of
		if (it2 != it)
		{
			if (lasttok.first == ")" || (opers.find(lasttok.first) == opers.end() && funcs.find(lasttok.first) != funcs.end()) || lasttok.second == -1)
				throw parse_error("Missing operator", i);

			out.push_back(lasttok = token_t(string(it, it2), -1));
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
	stack<num_t> s;
	for (token_t &tok : in)
	{
		if (tok.second == -1) // number
			s.push(stod(tok.first));
		else
		{
			if (s.size() < tok.second)
				throw runtime_error("Not enough arguments (have " + to_string(s.size()) + ") for function '" + tok.first + "' (want " + to_string(tok.second) + ")");
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
					ostringstream args; // stringstream because to_string adds trailing zeroes
					for (auto vit = v.begin(); vit != v.end(); ++vit) // construct exception argument list
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
