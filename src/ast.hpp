#ifndef _AST_HPP_
#define _AST_HPP_

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include "llvm.hpp"

class ExprAst {
public:
	virtual std::string to_string() = 0;
	virtual llvm::Value *codegen()
	{
		return nullptr;
	}
};

class NumberExprAst : public ExprAst {
private:
	// For now, all numbers will be double
	double number;
public:
	inline NumberExprAst(double number)
	{
		this->number = number;
	}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "NumberExprAst { number: " << this->number << " }";
		return ss.str();
	}
};

class StringExprAst : public ExprAst {
private:
	std::string value;
public:
	inline StringExprAst(std::string value) {
		this->value = value;
	}

	virtual inline std::string to_string() override {
		std::stringstream ss;

		ss << "StringExprAst { value: \"" << this->value << "\" }";
		return ss.str();
	}
};

class VariableExprAst : public ExprAst {
private:
	std::string name;
public:
	inline VariableExprAst(std::string name)
	{
		this->name = name;
	}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "VariableExprAst { name: " << this->name << " }";
		return ss.str();
	}
};

class BasicTypeExprAst : public ExprAst {
private:
	std::string type; // Types that don't need special handling can be represented as just a string
public:
	inline BasicTypeExprAst(std::string type)
	{
		this->type = type;
	}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "BasicTypeExprAst { type: " << this->type << " }";
		return ss.str();
	}
};

class TypeExprAst : public ExprAst {
private:
	std::unique_ptr<ExprAst> type; // A type can be either a basic type or a function prototype as of now
public:
	inline TypeExprAst(std::unique_ptr<ExprAst> type)
	{
		this->type = std::move(type);
	}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "TypeExprAst { type: " << this->type->to_string() << " }";
		return ss.str();
	}
};

class ArrayTypeExprAst : public ExprAst {
private:
	std::unique_ptr<TypeExprAst> recursing_type;
public:
	inline ArrayTypeExprAst(std::unique_ptr<TypeExprAst> recursing_type)
		: recursing_type(std::move(recursing_type))
	{}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "ArrayTypeExprAst { recursing_type: " << this->recursing_type->to_string() << " }";
		return ss.str();
	}
};

class FunctionParamAst {
private:
	std::unique_ptr<VariableExprAst> var;
	std::unique_ptr<ExprAst> type;
public:
	inline FunctionParamAst(std::unique_ptr<VariableExprAst> var, std::unique_ptr<TypeExprAst> type)
	{
		this->var = std::move(var);
		this->type = std::move(type);
	}

	inline std::string to_string()
	{
		std::stringstream ss;

		ss << "FunctionParamAst { var: " << this->var->to_string() <<
			", type: " << this->type->to_string() << " }";

		return ss.str();
	}
};

class FunctionProtoExprAst : public ExprAst {
private:
	std::vector<std::unique_ptr<FunctionParamAst>> params;
	std::unique_ptr<TypeExprAst> return_type; // `nullptr` means no return
public:
	inline FunctionProtoExprAst(std::vector<std::unique_ptr<FunctionParamAst>> args,
	                            std::unique_ptr<TypeExprAst> return_type)
	{
		this->params = std::move(args);
		this->return_type = std::move(return_type);
	}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "FunctionProtoExprAst { params: [";
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
	inline CodeblockExprAst(std::vector<std::unique_ptr<ExprAst>> subexprs)
		: subexprs(std::move(subexprs))
	{}

	virtual inline std::string to_string() override {
		std::stringstream ss;
		ss << "CodeblockExprAst { subexprs: [";
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
	inline FunctionExprAst(std::unique_ptr<FunctionProtoExprAst> proto, std::unique_ptr<CodeblockExprAst> body)
		: proto(std::move(proto)), body(std::move(body))
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
private:
	std::unique_ptr<VariableExprAst> name;
	std::unique_ptr<TypeExprAst> explicit_type; // Can be null (type should be infered)
	std::unique_ptr<ExprAst> value; // Can be null (should be zeroed)
public:
	inline DeclarationExprAst(std::unique_ptr<VariableExprAst> name,
	                          std::unique_ptr<TypeExprAst> explicit_type,
	                          std::unique_ptr<ExprAst> value)
	{
		this->name = std::move(name);
		this->explicit_type = std::move(explicit_type);
		this->value = std::move(value);
	}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "DeclarationExprAst { name: " << this->name->to_string() << ", explicit_type: " <<
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
	inline BinaryOpExprAst(std::unique_ptr<ExprAst> left, std::string op, std::unique_ptr<ExprAst> right)
		: left(std::move(left)), op(op), right(std::move(right))
	{}

	virtual inline std::string to_string() override {
		std::stringstream ss;

		ss << "BinaryOpExprAst { left: " << this->left->to_string() <<
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
	inline CallExprAst(std::string function, std::vector<std::unique_ptr<ExprAst>> args)
		: function(function), args(std::move(args))
	{}

	virtual inline std::string to_string() override {
		std::stringstream ss;

		ss << "CallExprAst { function: " << this->function << ", args: [";
		for (auto &arg : this->args) {
			ss << arg->to_string();
		}
		ss << "] }";

		return ss.str();
	}
};

#endif
