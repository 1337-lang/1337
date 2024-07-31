#include "lexer.hpp"
#include <stdexcept>
#include <unordered_map>
#include <sstream>
#include <fstream>

Lexer::Lexer(std::string filepath)
{
	std::ifstream source(filepath);
	if (!source.is_open()) {
		throw std::runtime_error("Failed to open file");
	}

	std::stringstream ss;
	ss << source.rdbuf();
	auto content = ss.str();

	this->content = content;
	this->cursor = 0;
	this->loc.line = 1;
	this->loc.column = 1;
	this->loc.filepath = filepath;
}

Lexer::Lexer(std::string content, std::string filepath) noexcept
{
	this->content = content;
	this->cursor = 0;
	this->loc.line = 1;
	this->loc.column = 1;
	this->loc.filepath = filepath;
}

std::unique_ptr<Token>
Lexer::tokenize()
{
	Token token;

	static std::unordered_map<std::string, TokenType> keywords = {
		{ "fn", TokenType::Fn },
		{ "mut", TokenType::Mut }
	};

	static std::unordered_map<char, TokenType> symbols = {
		{ '{', TokenType::LeftCurly },
		{ '}', TokenType::RightCurly },
		{ '(', TokenType::LeftParen },
		{ ')', TokenType::RightParen },
		{ '[', TokenType::LeftBracket },
		{ ']', TokenType::RightBracket },
		{ ',', TokenType::Comma },
		{ ':', TokenType::Colon },
		{ '+', TokenType::Plus },
		{ '-', TokenType::Minus },
		{ '/', TokenType::Divide },
		{ '*', TokenType::Multiply },
		{ '=', TokenType::Equals },
	};

	while (this->cursor < this->content.length()) {
		auto c = this->content[this->cursor];

		// Handle whitespaces
		if (std::isspace(c)) {
			this->advance();
			continue;
		}

		token.loc = this->loc;

		// Handle numbers
		if (std::isdigit(c) || c == '.') {
			token.type = TokenType::Integer;
			for (; this->cursor < this->content.length(); this->advance()) {
				auto next = content[this->cursor];
				if (next == '.') {
					token.type = TokenType::Float;
					token.value.push_back(next);
				} else if (std::isdigit(next)) {
					token.value.push_back(next);
				} else {
					break;
				}
			}

			return std::make_unique<Token>(token);
		}

		// Handle identifiers
		if (std::isalpha(c)) {
			token.type = TokenType::Identifier;
			token.value = std::string(1, c);
			while (this->advance() < this->content.length()) {
				auto next = this->content[this->cursor];
				if (!std::isalnum(next) && next != '_') {
					break;
				}

				token.value.push_back(next);
			}

			if (keywords.find(token.value) != keywords.end())
				token.type = keywords[token.value];

			return std::make_unique<Token>(token);
		}

		// Handle strings
		if (c == '"') {
			token.type = TokenType::String;
			while (this->advance() < this->content.length()) {
				auto next = this->content[this->cursor];
				if (next == '"') {
					this->advance();
					break;
				}

				token.value.push_back(next);
			}

			return std::make_unique<Token>(token);
		}

		// Handle characters
		if (c == '\'') {
			token.type = TokenType::Char;
			while (this->advance() < this->content.length()) {
				auto next = this->content[this->cursor];
				if (next == '\'') {
					this->advance();
					break;
				}

				token.value.push_back(next);
			}

			return std::make_unique<Token>(token);
		}

		// Handle symbols
		if (symbols.find(c) != symbols.end()) {
			token.type = symbols[c];
			token.value = std::string(1, c);
			this->advance();
			return std::make_unique<Token>(token);
		}

		// Handle unknown
		token.type = TokenType::Unknown;
		for (; this->cursor < this->content.length(); this->advance()) {
			auto next = this->content[this->cursor];
			if (std::isspace(next)) {
				break;
			}

			token.value.push_back(next);
		}

		return std::make_unique<Token>(token);
	}

	if (this->cursor == this->content.length()) {
		token.type = TokenType::Eof;
		token.value = "EOF";
		return std::make_unique<Token>(token);
	}

	return nullptr;
}
