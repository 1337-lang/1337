#include "codegen.hpp"
#include <iostream> // TODO:DELETE

bool Codegen::include(DeclarationExprAst *expr)
{
	auto builder = &this->builder;

	if (auto number = dynamic_cast<NumberExprAst *>(expr->value.get()); number != nullptr) {
		auto value = llvm::ConstantFP::get(builder->getDoubleTy(), number->number);
		auto var = new llvm::GlobalVariable(module, builder->getDoubleTy(), false, llvm::GlobalValue::ExternalLinkage, value, expr->name->name);
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

llvm::Value *Codegen::eval(ExprAst *expr)
{
	if (auto str = dynamic_cast<StringExprAst *>(expr); str != nullptr)
		return this->eval(str);

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
