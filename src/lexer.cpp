#include "lexer.hpp"
#include <unordered_map>

Lexer::Lexer(std::string_view content)
{
	this->content = content;
	this->cursor = 0;
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
		{ ':', TokenType::Colen },
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
			++this->cursor;
			continue;
		}

		// Handle numbers
		if (std::isdigit(c) || c == '.') {
			token.type = TokenType::Integer;
			for (; this->cursor < this->content.length(); ++this->cursor) {
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
			while (++this->cursor < this->content.length()) {
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

		// Handle symbols
		if (symbols.find(c) != symbols.end()) {
			token.type = symbols[c];
			token.value = std::string(1, c);
			++this->cursor;
			return std::make_unique<Token>(token);
		}

		// Handle unknown
		token.type = TokenType::Unknown;
		for (; this->cursor < this->content.length(); ++this->cursor) {
			auto next = this->content[this->cursor];
			if (std::isspace(next)) {
				break;
			}

			token.value.push_back(next);
		}

		return std::make_unique<Token>(token);
	}

	return nullptr;
}
