#ifndef _AST_HPP_
#define _AST_HPP_

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include "lexer.hpp"
#include "llvm.hpp"

class ExprAst {
protected:
	SourceLocation loc;
public:
	inline ExprAst(SourceLocation loc)
		: loc(loc)
	{}
public:
	virtual std::string to_string() = 0;
	virtual inline SourceLocation source_loc()
	{
		return this->loc;
	}
};

class NumberExprAst : public ExprAst {
private:
	// For now, all numbers will be double
	double number;
public:
	inline NumberExprAst(SourceLocation loc, double number)
		: ExprAst(loc), number(number)
	{}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "NumberExprAst (" << this->loc.str() << ") { number: " << this->number << " }";
		return ss.str();
	}
};

class StringExprAst : public ExprAst {
private:
	std::string value;
public:
	inline StringExprAst(SourceLocation loc, std::string value)
		: ExprAst(loc), value(value)
	{}

	virtual inline std::string to_string() override {
		std::stringstream ss;

		ss << "StringExprAst (" << this->loc.str()  << ") { value: \"" << this->value << "\" }";
		return ss.str();
	}
};

class VariableExprAst : public ExprAst {
private:
	std::string name;
public:
	inline VariableExprAst(SourceLocation loc, std::string name)
		: ExprAst(loc), name(name)
	{}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "VariableExprAst (" << this->loc.str() << ") { name: " << this->name << " }";
		return ss.str();
	}
};

class BasicTypeExprAst : public ExprAst {
private:
	std::string type; // Types that don't need special handling can be represented as just a string
public:
	inline BasicTypeExprAst(SourceLocation loc, std::string type)
		: ExprAst(loc), type(type)
	{}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "BasicTypeExprAst (" << this->loc.str() << ") { type: " << this->type << " }";
		return ss.str();
	}
};

class TypeExprAst : public ExprAst {
private:
	std::unique_ptr<ExprAst> type; // A type can be either a basic type or a function prototype as of now
public:
	inline TypeExprAst(SourceLocation loc, std::unique_ptr<ExprAst> type)
		: ExprAst(loc), type(std::move(type))
	{}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "TypeExprAst (" << this->loc.str() <<  ") { type: " << this->type->to_string() << " }";
		return ss.str();
	}
};

class ArrayTypeExprAst : public ExprAst {
private:
	std::unique_ptr<TypeExprAst> recursing_type;
public:
	inline ArrayTypeExprAst(SourceLocation loc, std::unique_ptr<TypeExprAst> recursing_type)
		: ExprAst(loc), recursing_type(std::move(recursing_type))
	{}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "ArrayTypeExprAst (" << this->loc.str() << ") { recursing_type: " << this->recursing_type->to_string() << " }";
		return ss.str();
	}
};

class FunctionParamAst {
private:
	SourceLocation loc;
	std::unique_ptr<VariableExprAst> var;
	std::unique_ptr<TypeExprAst> type;
public:
	inline FunctionParamAst(SourceLocation loc,
	                        std::unique_ptr<VariableExprAst> var,
	                        std::unique_ptr<TypeExprAst> type)
		: loc(loc), var(std::move(var)), type(std::move(type))
	{}

	inline std::string to_string()
	{
		std::stringstream ss;

		ss << "FunctionParamAst (" << this->loc.str() << ") { var: " << this->var->to_string() <<
			", type: " << this->type->to_string() << " }";

		return ss.str();
	}
};

class FunctionProtoExprAst : public ExprAst {
private:
	std::vector<std::unique_ptr<FunctionParamAst>> params;
	std::unique_ptr<TypeExprAst> return_type; // `nullptr` means no return
public:
	inline FunctionProtoExprAst(SourceLocation loc,
	                            std::vector<std::unique_ptr<FunctionParamAst>> params,
	                            std::unique_ptr<TypeExprAst> return_type)
		: ExprAst(loc), params(std::move(params)), return_type(std::move(return_type))
	{}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "FunctionProtoExprAst (" << this->loc.str() << ") { params: [";
		for (auto &param : this->params) {
			ss << param->to_string() << " ";
		}
		ss << "], return_type: " <<
			(this->return_type ? this->return_type->to_string() : "None") << " }";

		return ss.str();
	}
};

class CodeblockExprAst : public ExprAst {
private:
	std::vector<std::unique_ptr<ExprAst>> subexprs;
public:
	inline CodeblockExprAst(SourceLocation loc, std::vector<std::unique_ptr<ExprAst>> subexprs)
		: ExprAst(loc), subexprs(std::move(subexprs))
	{}

	virtual inline std::string to_string() override {
		std::stringstream ss;
		ss << "CodeblockExprAst (" << this->loc.str() << ") { subexprs: [";
		for (auto &expr : this->subexprs) {
			ss << expr->to_string() << " ";
		}
		ss << "] }";

		return ss.str();
	}
};

class FunctionExprAst : public ExprAst {
private:
	std::unique_ptr<FunctionProtoExprAst> proto;
	std::unique_ptr<CodeblockExprAst> body;
public:
	inline FunctionExprAst(SourceLocation loc,
	                       std::unique_ptr<FunctionProtoExprAst> proto,
	                       std::unique_ptr<CodeblockExprAst> body)
		: ExprAst(loc), proto(std::move(proto)), body(std::move(body))
	{}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "FunctionExprAst { proto: " << this->proto->to_string() <<
			", body: " << this->body->to_string() << " }";
		return ss.str();
	}
};

class DeclarationExprAst : public ExprAst {
public:
	std::unique_ptr<VariableExprAst> name;
	std::unique_ptr<TypeExprAst> explicit_type; // Can be null (type should be infered)
	std::unique_ptr<ExprAst> value; // Can be null (should be zeroed)
public:
	inline DeclarationExprAst(SourceLocation loc,
	                          std::unique_ptr<VariableExprAst> name,
	                          std::unique_ptr<TypeExprAst> explicit_type,
	                          std::unique_ptr<ExprAst> value)
		: ExprAst(loc), name(std::move(name)), explicit_type(std::move(explicit_type)), value(std::move(value))
	{}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "DeclarationExprAst (" << this->loc.str() << ") { name: " <<
			this->name->to_string() << ", explicit_type: " <<
			(this->explicit_type ? this->explicit_type->to_string() : "None") << ", value: " <<
			(this->value ? this->value->to_string() : "None");
		return ss.str();
	}
};

class BinaryOpExprAst : public ExprAst {
public:
	std::unique_ptr<ExprAst> left;
	std::string op;
	std::unique_ptr<ExprAst> right;
public:
	inline BinaryOpExprAst(SourceLocation loc,
	                       std::unique_ptr<ExprAst> left,
	                       std::string op,
	                       std::unique_ptr<ExprAst> right)
		: ExprAst(loc), left(std::move(left)), op(op), right(std::move(right))
	{}

	virtual inline std::string to_string() override {
		std::stringstream ss;

		ss << "BinaryOpExprAst (" << this->loc.str() << ") { left: " << this->left->to_string() <<
			", op: " << this->op <<
			", right: " << this->right->to_string() << " }";

		return ss.str();
	}
public:
	static int get_precedence(std::string op);
};

class CallExprAst : public ExprAst {
private:
	std::string function;
	std::vector<std::unique_ptr<ExprAst>> args;
public:
	inline CallExprAst(SourceLocation loc,
	                   std::string function,
	                   std::vector<std::unique_ptr<ExprAst>> args)
		: ExprAst(loc), function(function), args(std::move(args))
	{}

	virtual inline std::string to_string() override {
		std::stringstream ss;

		ss << "CallExprAst (" << this->loc.str() << ") { function: " << this->function << ", args: [";
		for (auto &arg : this->args) {
			ss << arg->to_string();
		}
		ss << "] }";

		return ss.str();
	}
};

class ExternExprAst : public ExprAst {
private:
	std::unique_ptr<DeclarationExprAst> decl;
public:
	inline ExternExprAst(SourceLocation loc, std::unique_ptr<DeclarationExprAst> decl)
		: ExprAst(loc), decl(std::move(decl))
	{}
	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "ExternExprAst (" << this->loc.str() << ") { name: " <<
			this->decl->name->to_string() << ", explicit_type: " <<
			(this->decl->explicit_type ? this->decl->explicit_type->to_string() : "None") << ", value: " <<
			(this->decl->value ? this->decl->value->to_string() : "None");
		return ss.str();
	}
};

#endif
