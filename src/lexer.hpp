#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <memory>
#include <sstream>

struct SourceLocation {
	std::string filepath = "<memory>";
	size_t line = 0;
	size_t column = 0;
	size_t cursor = 0;

	inline std::string str()
	{
		std::stringstream ss;

		ss << this->filepath << "@"
			<< this->line << ":" << this->column;
		return ss.str();
	}
};

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
	SourceLocation loc;
};

class Lexer {
private:
	std::string content;
	size_t cursor;
	SourceLocation loc;
public:
	Lexer(std::string content, std::string filepath) noexcept;
	Lexer(std::string filepath); // Can throw exception if the file doesn't open or doesn't read
public:
	std::unique_ptr<Token>
	tokenize();
private:
	inline size_t
	advance()
	{
		++this->cursor;
		this->loc.cursor = this->cursor;

		if (this->cursor >= this->content.length())
			return this->cursor;

		auto c = this->content[this->cursor];
		if (c == '\n') {
			++this->loc.line;
			this->loc.column = 0;
		} else if (c != '\r') {
			++this->loc.column;
		}

		return this->cursor;
	}
};

#endif
