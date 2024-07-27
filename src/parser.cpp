#include "parser.hpp"
#include "ast.hpp"
#include <memory>
#include <vector>

// Token Patterns: [Colon] [Ident] [Equals] <Expr>
//                 [Colon] [Equals] <Expr>
//                 [Colon] [Ident]
std::unique_ptr<DeclarationExprAst>
Parser::parse_declaration(std::string ident)
{
	std::unique_ptr<VariableExprAst> var_ast = std::make_unique<VariableExprAst>(ident);
	std::unique_ptr<DeclarationExprAst> decl_ast = nullptr;
	std::unique_ptr<TypeExprAst> explicit_type = nullptr;
	std::unique_ptr<ExprAst> value = nullptr;

	if (!this->advance())
		return decl_ast;

	switch (this->token->type) {
	case TokenType::Identifier:
	case TokenType::Fn:
		explicit_type = this->parse_type();
		if (!explicit_type)
			return decl_ast;
		if (this->token->type == TokenType::Equals) {
			if (!this->advance())
				return nullptr;
			value = this->parse_expression();
			if (!value)
				return nullptr;
		}
		break;
	case TokenType::Equals:
		if (!this->advance())
			return nullptr;
		value = this->parse_expression();
		if (!value)
			return nullptr;
		break;
	default:
		return nullptr;
	}

	decl_ast = std::make_unique<DeclarationExprAst>(
		std::move(var_ast), std::move(explicit_type), std::move(value)
	);

	return decl_ast;
}

std::unique_ptr<TypeExprAst>
Parser::parse_type()
{
	if (this->token->type == TokenType::Fn)
		return std::make_unique<TypeExprAst>(this->parse_function_proto());

	auto basic_type = BasicTypeExprAst { this->token->value };
	this->advance();

	return std::make_unique<TypeExprAst>(std::make_unique<BasicTypeExprAst>(basic_type));
}

std::unique_ptr<ExprAst>
Parser::parse_identifier()
{
	std::string ident = this->token->value;

	if (!this->advance())
		return nullptr;

	switch (this->token->type) {
	case TokenType::Colon:
		return this->parse_declaration(ident);
	default:
		break;
	}

	return nullptr;
}

// Token Patterns: [Fn] [LeftParen] ([Identifier] [Colon] [Type] [Comma])* [RightParen]
// Token Patterns: [Fn] [LeftParen] ([Identifier] [Colon] [Type] [Comma])* [RightParen] [Identifier]
// Example: fn (x: i32, y: i32) i32
std::unique_ptr<FunctionProtoExprAst>
Parser::parse_function_proto()
{
	std::vector<std::unique_ptr<FunctionParamAst>> params = {};
	std::unique_ptr<TypeExprAst> return_type = nullptr;

	if (!this->advance())
		return nullptr;

	if (this->token->type != TokenType::LeftParen)
		return nullptr;

	if (!this->advance())
		return nullptr;

	while (this->token->type != TokenType::RightParen) {
		if (this->token->type != TokenType::Identifier)
			return nullptr;

		auto param = this->parse_function_param();
		if (!param)
			return nullptr;

		if (!this->token)
			return nullptr;

		if (this->token->type == TokenType::Comma) {
			if (!this->advance())
				return nullptr;
		}
	}

	if (this->advance() && this->token->type == TokenType::Identifier) {
		return_type = this->parse_type();
		if (!return_type)
			return nullptr;
	}

	return std::make_unique<FunctionProtoExprAst>(std::move(params), std::move(return_type));
}

std::unique_ptr<FunctionParamAst>
Parser::parse_function_param()
{
	return nullptr;
}

std::unique_ptr<NumberExprAst>
Parser::parse_number()
{
	double value;

	try {
		value = stod(this->token->value);
	} catch (...) {
		return nullptr;
	}

	this->advance();

	return std::make_unique<NumberExprAst>(value);
}

std::unique_ptr<StringExprAst>
Parser::parse_string()
{
	std::string value = this->token->value;
	
	this->advance();

	return std::make_unique<StringExprAst>(value);
}

std::unique_ptr<ExprAst>
Parser::parse_expression()
{
	if (!this->token || this->token->type == TokenType::Eof)
		return nullptr;

	switch (this->token->type) {
	case TokenType::Identifier:
		return this->parse_identifier();
	case TokenType::Integer:
	case TokenType::Float:
		return this->parse_number();
	case TokenType::String:
		return this->parse_string();
	case TokenType::Fn:
	{
		auto proto = this->parse_function_proto();
		if (!proto)
			return nullptr;

		if (!this->token || this->token->type != TokenType::LeftCurly)
			return proto;

		auto body = this->parse_expression();
		if (!body)
			return nullptr;

		return std::make_unique<FunctionExprAst>(std::move(proto), std::move(body));
	}
	default:
		break;
	}

	return nullptr;
}
