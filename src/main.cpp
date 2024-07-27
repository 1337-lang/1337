#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.hpp"

int main(int argc, char **argv)
{
	if (argc < 2) {
		std::cout << "usage: 1337 [SOURCE]" << std::endl;
		return 1;
	}

	std::ifstream source(argv[1]);
	if (!source.is_open()) {
		std::cout << "Failed to read source file: " << argv[1] << std::endl;
		return 1;
	}

	std::stringstream ss;
	ss << source.rdbuf();
	auto content = ss.str();

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

	return 0;
}
