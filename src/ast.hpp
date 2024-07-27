#ifndef _AST_HPP_
#define _AST_HPP_

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <sstream>

class ExprAst {
public:
	virtual inline std::string to_string()
	{
		return "ExprAst{}";
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

class FunctionProtoExprAst : public ExprAst {
private:
	std::vector<std::pair<VariableExprAst, TypeExprAst>> args;
public:
	inline FunctionProtoExprAst(std::vector<std::pair<VariableExprAst, TypeExprAst>> args)
	{
		this->args = std::move(args);
	}

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "FunctionProtoExprAst {}";
		return ss.str();
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

	virtual inline std::string to_string() override
	{
		std::stringstream ss;
		ss << "FunctionExprAst {}";
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

#endif
