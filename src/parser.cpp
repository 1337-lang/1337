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
	case TokenType::LeftBracket:
	{
		this->advance();
		if (!this->token || this->token->type != TokenType::RightBracket)
			return nullptr;

		if (!this->advance())
			return nullptr;

		auto recursing_type = this->parse_type();
		if (!recursing_type)
			return nullptr;

		return std::make_unique<TypeExprAst>(std::make_unique<ArrayTypeExprAst>(std::move(recursing_type)));
	}
	default:
		break;
	}

	return nullptr;
}

std::unique_ptr<CallExprAst>
Parser::parse_call(std::string ident)
{
	std::vector<std::unique_ptr<ExprAst>> args;
	while (true) {
		if (!this->advance())
			return nullptr;

		if (this->token->type == TokenType::RightParen)
			break;
		else if (this->token->type == TokenType::Comma)
			continue;

		auto arg = this->parse_expression();
		if (!arg)
			return nullptr;

		args.push_back(std::move(arg));
	};

	this->advance(); // Skip over right parenthesis

	return std::make_unique<CallExprAst>(ident, std::move(args));
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
	case TokenType::LeftParen:
		return this->parse_call(ident);
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

std::unique_ptr<ExprAst>
Parser::parse_binop_rhs(int expr_prec, std::unique_ptr<ExprAst> lhs)
{
	/*
	 * Let's say it takes the input `1 + 2 / 3 * 4 - 5`
	 * - It gets `1` and the precedence of the operator `+`
	 * - It parses the primary expression for the RHS, which is gonna be `2`
	 * - It compares the precedence of the next operator `/`, which is greater than the precedence of `+`
	 * - It's gonna attempt to parse the expression `2 / 3 * 4 - 5` now
	 * - The LHS is `2`, and the operator is `/`
	 * - The RHS is `3`
	 * - It gets the operator `*`, which has the same precedence as the current operator
	 * - It will create the first BinOp expression, with `{ left: 2, op: '/', right: 3 }`
	 * - It's gonna create the second BinOp expression, with `{ left: { left: 2, op: '/', right: 3 }, op: '*', right: 4 }`
	 * - The next operator is `-`, so the precedence is smaller. So we return the BinOp to the previous call
	 * - In the previous call, the following BinOp expression will be created: `{ left: 1, op: '+', right: { <newly returned RHS> }`
	 * - Finally, it's gonna build the following expression on the next loop: `{ left: <everything until now>, op: '-', right: 5 }`
	 */
	while (true) {
		if (BinaryOpExprAst::get_precedence(this->token->value) < expr_prec)
			return lhs;

		auto op = this->token->value;
		auto token_prec = BinaryOpExprAst::get_precedence(op);
		if (!this->advance())
			return nullptr;

		auto rhs = this->parse_primary();
		if (!rhs)
			return nullptr;

		// If next operator precedence is bigger than the current operator precedence,
		// we parse a binop having the left hand side set to our right hand side
		if (this->token && BinaryOpExprAst::get_precedence(this->token->value) > token_prec) {
			// We pass 'token_prec + 1' to avoid swaping contents with the same precedence
			rhs = this->parse_binop_rhs(token_prec + 1, std::move(rhs));
			if (!rhs)
				return nullptr;
		}

		lhs = std::make_unique<BinaryOpExprAst>(std::move(lhs), op, std::move(rhs));
	}
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
	auto lhs = this->parse_primary();
	if (!lhs || !this->token)
		return lhs;

	// If the is a next token, attempt to parse this as a BinOp
	// The BinOp parse function will just return the LHS if it's
	// not a BinOp
	return this->parse_binop_rhs(0, std::move(lhs));
}

std::unique_ptr<ExprAst>
Parser::parse_primary()
{
	std::unique_ptr<ExprAst> expr = nullptr;

	if (!this->token || this->token->type == TokenType::Eof)
		return nullptr;

	switch (this->token->type) {
	case TokenType::Identifier:
		expr = this->parse_identifier();
		break;
	case TokenType::Integer:
	case TokenType::Float:
		expr = this->parse_number();
		break;
	case TokenType::String:
		expr = this->parse_string();
		break;
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

		expr = std::make_unique<FunctionExprAst>(std::move(proto), std::move(body));
		break;
	}
	case TokenType::LeftCurly:
		expr = this->parse_codeblock();
		break;
	default:
		break;
	}

	return expr;
}
