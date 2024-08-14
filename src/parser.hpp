#ifndef _PARSER_HPP_
#define _PARSER_HPP_

#include "lexer.hpp"
#include "ast.hpp"

class Parser {
private:
	Lexer lexer;
	std::unique_ptr<Token> token = nullptr;
public:
	inline Parser(std::string content, std::string filepath) noexcept: lexer(content, filepath) {
		this->advance();
	}

	inline Parser(std::string filepath): lexer(filepath) {
		this->advance();
	}
public:
	std::unique_ptr<ExprAst>
	parse_expression();

	std::unique_ptr<ExprAst>
	parse_primary();

	inline std::unique_ptr<Token> &
	advance()
	{
		this->token = this->lexer.tokenize();
		return this->token;
	}

	inline bool
	is_finished()
	{
		return this->token && this->token->type == TokenType::Eof;
	}
private:
	std::unique_ptr<ExprAst>
	parse_identifier();

	std::unique_ptr<NumberExprAst>
	parse_number();

	std::unique_ptr<StringExprAst>
	parse_string();

	std::unique_ptr<DeclarationExprAst>
	parse_declaration(SourceLocation loc, std::string ident);

	std::unique_ptr<CallExprAst>
	parse_call(SourceLocation loc, std::string ident);

	std::unique_ptr<ArrayIndexExprAst>
	parse_array_index(SourceLocation loc, std::string ident);

	std::unique_ptr<TypeExprAst>
	parse_type();

	std::unique_ptr<FunctionProtoExprAst>
	parse_function_proto();

	std::unique_ptr<FunctionParamAst>
	parse_function_param();

	std::unique_ptr<CodeblockExprAst>
	parse_codeblock();

	std::unique_ptr<ExprAst>
	parse_binop_rhs(int expr_prec, std::unique_ptr<ExprAst> lhs);
};

#endif
