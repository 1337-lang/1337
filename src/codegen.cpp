#include "codegen.hpp"
#include "ast.hpp"
#include <regex>

bool Codegen::include(DeclarationExprAst *expr)
{
	auto builder = &this->builder;

	if (auto number = dynamic_cast<NumberExprAst *>(expr->value.get()); number != nullptr) {
		llvm::Type *type;
		if (expr->explicit_type != nullptr)
			type = this->type(expr->explicit_type.get());
		else
			type = builder->getDoubleTy();

		llvm::Constant *value;
		if (type->isIntegerTy()) {
			value = llvm::Constant::getIntegerValue(type, llvm::APInt(type->getIntegerBitWidth(), number->number, 10));
		} else {
			value = llvm::ConstantFP::get(type, number->number);
		}
		auto var = new llvm::GlobalVariable(module, type, false, llvm::GlobalValue::ExternalLinkage, value, expr->name->name);
		this->variables[expr->name->name] = var;
		return true;
	} else if (auto str = dynamic_cast<StringExprAst *>(expr->value.get()); str != nullptr) {
		auto value = builder->CreateGlobalStringPtr(str->value, expr->name->name, 0, &this->module);
		return true;
	} else if (auto func = dynamic_cast<FunctionExprAst *>(expr->value.get()); func != nullptr) {
		auto ret_type = builder->getVoidTy();
		std::vector<llvm::Type *> param_types = {};
		auto name = expr->name->name;
		auto type = llvm::FunctionType::get(ret_type, param_types, false);
		auto function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, this->module);
		auto block = llvm::BasicBlock::Create(this->context, "entry", function);
		builder->SetInsertPoint(block);

		for (auto &subexpr : func->body->subexprs) {
			if (!this->include(subexpr.get()))
				return false;
		}

		builder->CreateRetVoid();
		builder->ClearInsertionPoint();
		this->variables[expr->name->name] = function;
		return true;
	}

	return false;
}

bool Codegen::include(CallExprAst *expr)
{
	auto function = this->module.getFunction(expr->function);

	std::vector<llvm::Value *> args;
	for (auto &arg : expr->args) {
		args.push_back(this->eval(arg.get()));
	}
	this->builder.CreateCall(function, args);

	return true;
}

llvm::Value *Codegen::eval(StringExprAst *str)
{
	auto value = this->builder.CreateGlobalStringPtr(str->value, "", 0, &this->module);
	return value;
}

llvm::Value *Codegen::eval(VariableExprAst *expr)
{
	if (this->variables.find(expr->name) == this->variables.end())
		return nullptr;

	auto ptr = this->variables[expr->name];
	auto type = ptr->getType();
	return this->builder.CreateLoad(type, ptr);
}

llvm::Value *Codegen::eval(ExprAst *expr)
{
	if (auto str = dynamic_cast<StringExprAst *>(expr); str != nullptr)
		return this->eval(str);
	if (auto var = dynamic_cast<VariableExprAst *>(expr); var != nullptr)
		return this->eval(var);

	return nullptr;
}

llvm::Type *Codegen::type(TypeExprAst *expr)
{
	if (auto basic = dynamic_cast<BasicTypeExprAst *>(expr->type.get()); basic != nullptr) {
		auto integer_regex = std::regex("[iu]([0-9]+)");
		std::cmatch m;
		if (std::regex_match(basic->type.c_str(), m, integer_regex)) {
			auto nbits = atoi(m[1].str().c_str());
			if (nbits <= 0)
				return nullptr;

			auto type = this->builder.getIntNTy(static_cast<unsigned int>(nbits));
			return type;
		}
	}

	return nullptr;
}

bool Codegen::include(ExprAst *expr)
{
	if (auto decl_expr = dynamic_cast<DeclarationExprAst *>(expr); decl_expr != nullptr)
		return this->include(decl_expr);

	if (auto call_expr = dynamic_cast<CallExprAst *>(expr); call_expr != nullptr) {
		return this->include(call_expr);
	}

	return false;
}
