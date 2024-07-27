#include "parser.hpp"
#include "ast.hpp"
#include <memory>

// Token Patterns: [Identifier] [Colon] [Ident] [Equals] <Expr>
//                 [Identifier] [Colon] [Equals] <Expr>
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
		explicit_type = this->parse_type();
		if (!explicit_type)
			return decl_ast;
		break;
	case TokenType::Equals:
		if (!this->advance())
			return decl_ast;
		value = this->parse_expression();
		if (!value)
			return decl_ast;
		decl_ast = std::make_unique<DeclarationExprAst>(DeclarationExprAst(
			std::move(var_ast), std::move(explicit_type), std::move(value)
		));
		break;
	default:
		break;
	}

	return decl_ast;
}

// Token Patterns: [Identifier]
//                 [Fn] [Identifier] [LParen] [... args] [RParen]
std::unique_ptr<TypeExprAst>
Parser::parse_type()
{
	// TODO: parse function types

	auto basic_type = BasicTypeExprAst { this->token->value };
	return std::make_unique<TypeExprAst>(TypeExprAst(std::make_unique<ExprAst>(basic_type)));
}

std::unique_ptr<ExprAst>
Parser::parse_identifier()
{
	std::unique_ptr<ExprAst> ast = nullptr;
	std::string ident = this->token->value;

	if (!this->advance())
		return ast;

	switch (this->token->type) {
	case TokenType::Colon:
	default:
		return this->parse_declaration(ident);
	}

	return ast;
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
	default:
		break;
	}

	return nullptr;
}
