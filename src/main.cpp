#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "parser.hpp"
#include "codegen.hpp"

int main(int argc, char **argv)
{
	if (argc < 2) {
		std::cout << "usage: 1337 [SOURCE]" << std::endl;
		return 1;
	}

	/*
	auto lexer = Lexer(content);
	std::unique_ptr<Token> token;
	while (true) {
		token = lexer.tokenize();
		if (!token) {
			std::cout << "Failed to tokenize" << std::endl;
			return -1;
		}

		std::cout << "TOKEN: " << static_cast<int>(token->type) << ", " << token->value << std::endl;

		if (token->type == TokenType::Eof) {
			std::cout << "File ended" << std::endl;
			break;
		}
	}
	*/

	/*
	auto parser = Parser(argv[1]);

	while (true) {
		auto expr = parser.parse_expression();
		if (!expr)
			break;

		std::cout << "EXPR: " << expr->to_string() << std::endl;
	}

	if (!parser.is_finished()) {
		std::cout << "Failed to parse until EOF" << std::endl;
	}
	*/

	auto parser = Parser(argv[1]);
	auto codegen = Codegen();
	while (true) {
		auto expr = parser.parse_expression();
		if (!expr)
			break;

		if (!codegen.include(expr.get())) {
			std::cout << "[ERR] Failed to codegen the following expression: "
				<< expr->to_string();
		}
	}
	codegen.dump();

	return 0;
}
