#include "ast.hpp"
#include <unordered_map>

int BinaryOpExprAst::get_precedence(std::string op)
{
	static std::unordered_map<std::string, int> precedence {
		{ "+", 20 },
		{ "-", 20 },
		{ "*", 40 },
		{ "/", 40 },
	};

	if (precedence.find(op) == precedence.end())
		return -1;

	return precedence[op];
}
