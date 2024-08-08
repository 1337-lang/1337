#include "codegen.hpp"
#include <iostream> // TODO:DELETE

bool Codegen::include(DeclarationExprAst *expr)
{
	auto builder = this->block_builder ? this->block_builder.get() : &this->builder;

	if (auto number = dynamic_cast<NumberExprAst *>(expr->value.get()); number != nullptr) {
		auto value = llvm::ConstantFP::get(builder->getDoubleTy(), number->number);
		auto var = new llvm::GlobalVariable(module, builder->getDoubleTy(), false, llvm::GlobalValue::ExternalLinkage, value, expr->name->name);
		this->globals[expr->name->name] = var;
		return true;
	} else if (auto str = dynamic_cast<StringExprAst *>(expr->value.get()); str != nullptr) {
		auto value = builder->CreateGlobalStringPtr(str->value, expr->name->name, 0, &this->module);
		return true;
	} else if (auto func = dynamic_cast<FunctionExprAst *>(expr); func != nullptr) {
		
	}

	return false;
}

bool Codegen::include(ExprAst *expr)
{
	if (auto decl_expr = dynamic_cast<DeclarationExprAst *>(expr); decl_expr != nullptr)
		return this->include(decl_expr);

	return false;
}
