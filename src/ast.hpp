#ifndef _AST_HPP_
#define _AST_HPP_

#include <string>
#include <utility>
#include <vector>
#include <memory>

class ExprAst {
public:
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
};

class VariableExprAst : public ExprAst {
private:
	std::string name;
public:
	inline VariableExprAst(std::string name)
	{
		this->name = name;
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
};

class TypeExprAst : public ExprAst {
private:
	std::unique_ptr<ExprAst> type; // A type can be either a basic type or a function prototype as of now
public:
	inline TypeExprAst();
};

class FunctionProtoExprAst : public ExprAst {
private:
	std::vector<std::pair<VariableExprAst, TypeExprAst>> args;
public:
	inline FunctionProtoExprAst(std::vector<std::pair<VariableExprAst, TypeExprAst>> args)
	{
		this->args = std::move(args);
	}
};

class FunctionExprAst : public ExprAst {
private:
	std::unique_ptr<FunctionProtoExprAst> proto;
	std::unique_ptr<ExprAst> body;
public:
	inline FunctionExprAst(std::unique_ptr<FunctionProtoExprAst> proto, std::unique_ptr<ExprAst> body)
	{
		this->proto = std::move(proto);
		this->body = std::move(body);
	}
};

class DeclarationExprAst : public ExprAst {
private:
	std::unique_ptr<VariableExprAst> name;
	std::unique_ptr<TypeExprAst> explicit_type;
	std::unique_ptr<ExprAst> value;
public:
	inline DeclarationExprAst(std::unique_ptr<VariableExprAst> name,
	                          std::unique_ptr<TypeExprAst> explicit_type,
	                          std::unique_ptr<FunctionExprAst> value)
	{
		this->name = std::move(name);
		this->explicit_type = std::move(explicit_type);
		this->value = std::move(value);
	}
};

#endif
