#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <string_view>
#include <memory>

enum class TokenType: int {
	Eof = -1,
	Unknown,
	Identifier,

	// Values
	Integer,
	Float,
	String,
	Char,

	// Keywords
	Fn,
	Mut,

	// Symbols
	LeftCurly,
	RightCurly,
	LeftParen,
	RightParen,
	LeftBracket,
	RightBracket,
	Comma,
	Colon,
	Plus,
	Minus,
	Divide,
	Multiply,
	Equals,
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
public:
	std::unique_ptr<Token>
	tokenize();
};

#endif
