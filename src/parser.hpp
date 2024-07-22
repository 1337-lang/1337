#ifndef _PARSER_HPP_
#define _PARSER_HPP_

#include <string_view>
#include "lexer.hpp"
#include "ast.hpp"

class Parser {
private:
	Lexer lexer;
	std::unique_ptr<Token> token = nullptr;
public:
	Parser(std::string_view content);
public:
	std::unique_ptr<Expr>
	parse_expression();

	std::unique_ptr<Stmt>
	parse_statement();

	void
	advance();
};

#endif
