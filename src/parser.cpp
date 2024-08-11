#include "parser.hpp"
#include "ast.hpp"
#include <memory>
#include <vector>

// Token Patterns: [Colon] [Ident] [Equals] <Expr>
//                 [Colon] [Equals] <Expr>
//                 [Colon] [Ident]
std::unique_ptr<DeclarationExprAst>
Parser::parse_declaration(SourceLocation loc, std::string ident)
{
	std::unique_ptr<VariableExprAst> var_ast = std::make_unique<VariableExprAst>(loc, ident);
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
		loc, std::move(var_ast), std::move(explicit_type), std::move(value)
	);

	return decl_ast;
}

std::unique_ptr<TypeExprAst>
Parser::parse_type()
{
	auto loc = this->token->loc;

	switch (this->token->type) {
	case TokenType::Fn:
	{
		auto proto = this->parse_function_proto();
		if (!proto)
			return nullptr;
		return std::make_unique<TypeExprAst>(loc, std::move(proto));
	}
	case TokenType::Identifier:
	{
		auto basic_type = BasicTypeExprAst { loc, this->token->value };
		this->advance();
		return std::make_unique<TypeExprAst>(loc, std::make_unique<BasicTypeExprAst>(basic_type));
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

		return std::make_unique<TypeExprAst>(loc, std::make_unique<ArrayTypeExprAst>(loc, std::move(recursing_type)));
	}
	default:
		break;
	}

	return nullptr;
}

std::unique_ptr<CallExprAst>
Parser::parse_call(SourceLocation loc, std::string ident)
{
	std::vector<std::unique_ptr<ExprAst>> args;

	this->advance();

	while (true) {
		if (!this->token)
			return nullptr;

		if (this->token->type == TokenType::RightParen) {
			break;
		} else if (this->token->type == TokenType::Comma) {
			this->advance();
			continue;
		}

		auto arg = this->parse_expression();
		if (!arg)
			return nullptr;

		args.push_back(std::move(arg));
	};

	this->advance(); // Skip over right parenthesis

	return std::make_unique<CallExprAst>(loc, ident, std::move(args));
}

std::unique_ptr<ExprAst>
Parser::parse_identifier()
{
	std::string ident = this->token->value;
	SourceLocation loc = this->token->loc;

	if (!this->advance())
		return nullptr;

	switch (this->token->type) {
	case TokenType::Colon:
		return this->parse_declaration(loc, ident);
	case TokenType::LeftParen:
		return this->parse_call(loc, ident);
	default:
		break;
	}

	return std::make_unique<VariableExprAst>(loc, ident);
}

// Token Patterns: [Fn] [Identifier] [LeftParen] ([Identifier] [Colon] [Type] [Comma])* [RightParen]
// Token Patterns: [Fn] [LeftParen] ([Identifier] [Colon] [Type] [Comma])* [RightParen]
// Example: fn i32 (x: i32, y: i32)
std::unique_ptr<FunctionProtoExprAst>
Parser::parse_function_proto()
{
	std::vector<std::unique_ptr<FunctionParamAst>> params = {};
	std::unique_ptr<TypeExprAst> return_type = nullptr;
	auto loc = this->token->loc;

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

	return std::make_unique<FunctionProtoExprAst>(loc, std::move(params), std::move(return_type));
}

// Token Patterns: [Ident] [Colon] [Type]
std::unique_ptr<FunctionParamAst>
Parser::parse_function_param()
{
	auto loc = this->token->loc;
	std::unique_ptr<VariableExprAst> var = std::make_unique<VariableExprAst>(loc, this->token->value);
	if (!this->advance() || this->token->type != TokenType::Colon)
		return nullptr;

	if (!this->advance())
		return nullptr;

	auto type = this->parse_type();
	if (!type)
		return nullptr;

	return std::make_unique<FunctionParamAst>(loc, std::move(var), std::move(type));
}

// Token Patterns: [LeftCurly] <Expr>* [RightCurly]
std::unique_ptr<CodeblockExprAst>
Parser::parse_codeblock()
{
	auto loc = this->token->loc;
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

	return std::make_unique<CodeblockExprAst>(loc, std::move(subexprs));
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
	auto loc = this->token->loc;
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

		lhs = std::make_unique<BinaryOpExprAst>(loc, std::move(lhs), op, std::move(rhs));
	}
}

std::unique_ptr<NumberExprAst>
Parser::parse_number()
{
	auto loc = this->token->loc;
	auto value = this->token->value;

	this->advance();

	return std::make_unique<NumberExprAst>(loc, value);
}

std::unique_ptr<StringExprAst>
Parser::parse_string()
{
	auto loc = this->token->loc;
	std::string value = this->token->value;
	std::string fmt = "";

	bool prev_backslash = false;
	for (size_t i = 0; i < value.length(); ++i) {
		auto c = value[i];

		if (c == '\\') {
			prev_backslash = true;
			continue;
		}

		if (!prev_backslash) {
			fmt.push_back(c);
			continue;
		}

		// Previous character was a backslash, do special handling
		switch (c) {
		case 'n':
			fmt.push_back('\n');
			break;
		default:
			fmt.push_back(c);
			break;
		}

		prev_backslash = false;
	}
	
	this->advance();

	return std::make_unique<StringExprAst>(loc, fmt);
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
		auto loc = this->token->loc;
		auto proto = this->parse_function_proto();
		if (!proto)
			return nullptr;

		if (!this->token || this->token->type != TokenType::LeftCurly)
			return proto;

		auto body = this->parse_codeblock();
		if (!body)
			return nullptr;

		expr = std::make_unique<FunctionExprAst>(loc, std::move(proto), std::move(body));
		break;
	}
	case TokenType::Extern:
	{
		if (!this->advance() || this->token->type != TokenType::Identifier)
			return nullptr;
		auto ident = *this->token;
		if (!this->advance() || this->token->type != TokenType::Colon)
			return nullptr;

		auto decl_expr = this->parse_declaration(ident.loc, ident.value);
		if (!decl_expr)
			return nullptr;
		expr = std::make_unique<ExternExprAst>(ident.loc, std::move(decl_expr));
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
