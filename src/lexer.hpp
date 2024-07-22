#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <string_view>
#include <memory>

enum class TokenType: int {
	Unknown,
	Identifier,

	// Values
	Integer,
	Float,
};

struct Token {
	TokenType type;
	std::string value;
	// TODO: keep position information (line, column)
};

class Lexer {
private:
	std::string_view content;
	size_t cursor;
public:
	Lexer(std::string_view content);

	std::unique_ptr<Token>
	tokenize();
};

#endif
