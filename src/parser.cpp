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
	switch (this->token->type) {
	case TokenType::Fn:
	{
		auto proto = this->parse_function_proto();
		if (!proto)
			return nullptr;
		return std::make_unique<TypeExprAst>(std::move(proto));
	}

	case TokenType::Identifier:
	{
		auto basic_type = BasicTypeExprAst { this->token->value };
		this->advance();
		return std::make_unique<TypeExprAst>(std::make_unique<BasicTypeExprAst>(basic_type));
	}
	default:
		break;
	}

	return nullptr;
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

	return std::make_unique<VariableExprAst>(ident);
}

// Token Patterns: [Fn] [Identifier] [LeftParen] ([Identifier] [Colon] [Type] [Comma])* [RightParen]
// Token Patterns: [Fn] [LeftParen] ([Identifier] [Colon] [Type] [Comma])* [RightParen]
// Example: fn i32 (x: i32, y: i32)
std::unique_ptr<FunctionProtoExprAst>
Parser::parse_function_proto()
{
	std::vector<std::unique_ptr<FunctionParamAst>> params = {};
	std::unique_ptr<TypeExprAst> return_type = nullptr;

	if (!this->advance())
		return nullptr;

	// Parse return type if it exists
	if (this->token->type != TokenType::LeftParen) {
		return_type = this->parse_type();
		if (!return_type)
			return nullptr;
	}

	if (!this->token || this->token->type != TokenType::LeftParen)
		return nullptr;

	if (!this->advance())
		return nullptr;

	while (this->token->type != TokenType::RightParen) {
		if (this->token->type != TokenType::Identifier)
			return nullptr;

		auto param = this->parse_function_param();
		if (!param)
			return nullptr;

		params.push_back(std::move(param));

		if (!this->token)
			return nullptr;

		if (this->token->type == TokenType::Comma) {
			if (!this->advance())
				return nullptr;
		}
	}

	this->advance();

	return std::make_unique<FunctionProtoExprAst>(std::move(params), std::move(return_type));
}

// Token Patterns: [Ident] [Colon] [Type]
std::unique_ptr<FunctionParamAst>
Parser::parse_function_param()
{
	std::unique_ptr<VariableExprAst> var = std::make_unique<VariableExprAst>(this->token->value);
	if (!this->advance() || this->token->type != TokenType::Colon)
		return nullptr;

	if (!this->advance())
		return nullptr;

	auto type = this->parse_type();
	if (!type)
		return nullptr;

	return std::make_unique<FunctionParamAst>(std::move(var), std::move(type));
}

// Token Patterns: [LeftCurly] <Expr>* [RightCurly]
std::unique_ptr<CodeblockExprAst>
Parser::parse_codeblock()
{
	std::vector<std::unique_ptr<ExprAst>> subexprs;

	if (!this->advance())
		return nullptr;

	while (this->token->type != TokenType::RightCurly) {
		auto expr = this->parse_expression();
		if (!expr || !this->token)
			return nullptr;
		subexprs.push_back(std::move(expr));
	}

	this->advance();

	return std::make_unique<CodeblockExprAst>(std::move(subexprs));
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

		auto body = this->parse_codeblock();
		if (!body)
			return nullptr;

		return std::make_unique<FunctionExprAst>(std::move(proto), std::move(body));
	}
	case TokenType::LeftCurly:
		return this->parse_codeblock();
	default:
		break;
	}

	return nullptr;
}
