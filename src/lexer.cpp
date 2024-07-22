#include "lexer.hpp"

Lexer::Lexer(std::string_view content)
{
	this->content = content;
	this->cursor = 0;
}

std::unique_ptr<Token>
Lexer::tokenize()
{
	std::unique_ptr<Token> token = nullptr;

	while (this->cursor < this->content.length()) {
		auto c = this->content[this->cursor];

		// Handle whitespaces
		if (std::isspace(c)) {
			++this->cursor;
			continue;
		}

		// Handle numbers
		if (std::isdigit(c) || c == '.') {
			TokenType type = TokenType::Integer;
			std::string value;
			for (; this->cursor < this->content.length(); ++this->cursor) {
				auto next = content[this->cursor];
				if (next == '.') {
					type = TokenType::Float;
					value.push_back(next);
				} else if (std::isdigit(next)) {
					value.push_back(next);
				} else {
					break;
				}
			}

			token = std::make_unique<Token>(Token { type, value });
			break;
		}

		// Handle unknown
		++this->cursor;
	}

	return token;
}
