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
			type = builder->getDoubleTy(); // TODO: Type inference

		llvm::Constant *value;
		if (type->isIntegerTy()) {
			value = llvm::Constant::getIntegerValue(type, llvm::APInt(type->getIntegerBitWidth(), number->number, 10));
		} else {
			value = llvm::ConstantFP::get(type, number->number);
		}
		auto var = new llvm::GlobalVariable(module, type, false, llvm::GlobalValue::ExternalLinkage, value, expr->name->name);
		this->variables[expr->name->name] = std::make_pair<>(type, var);
		return true;
	} else if (auto str = dynamic_cast<StringExprAst *>(expr->value.get()); str != nullptr) {
		auto value = builder->CreateGlobalStringPtr(str->value);
		auto var = new llvm::GlobalVariable(module, builder->getPtrTy(), false, llvm::GlobalValue::ExternalLinkage, value, expr->name->name);
		this->variables[expr->name->name] = std::make_pair<>(var->getType(), var);
		return true;
	} else if (auto func = dynamic_cast<FunctionExprAst *>(expr->value.get()); func != nullptr) {
		auto ret_type = builder->getVoidTy();
		std::vector<llvm::Type *> param_types = {};
		for (auto &param : func->proto->params) {
			auto type = this->type(param->type.get());
			if (!type)
				return false;

			param_types.push_back(type);
		}

		auto name = expr->name->name;
		auto type = llvm::FunctionType::get(ret_type, param_types, false);
		auto function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, this->module);
		auto block = llvm::BasicBlock::Create(this->context, "entry", function);
		builder->SetInsertPoint(block);

		for (auto &arg : function->args()) {
			auto i = arg.getArgNo();
			auto type = param_types[i];
			auto name = func->proto->params[i]->var->name;
			auto local_var = builder->CreateAlloca(type, nullptr, name);
			llvm::Value *val = &arg;
			builder->CreateStore(val, local_var);
			this->variables[name] = std::make_pair<>(type, local_var);
		}

		for (auto &subexpr : func->body->subexprs) {
			if (!this->include(subexpr.get()))
				return false;
		}

		builder->CreateRetVoid();
		builder->ClearInsertionPoint();
		this->variables[expr->name->name] = std::make_pair<>(type, function);
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

	auto type = this->variables[expr->name].first;
	auto ptr = this->variables[expr->name].second;

	return this->builder.CreateLoad(type, ptr);
}

llvm::Value *Codegen::eval(ExprAst *expr)
{
	if (auto str = dynamic_cast<StringExprAst *>(expr); str != nullptr)
		return this->eval(str);
	if (auto var = dynamic_cast<VariableExprAst *>(expr); var != nullptr)
		return this->eval(var);
	if (auto var = dynamic_cast<NumberExprAst *>(expr); var != nullptr)
		return this->eval(var);
	if (auto var = dynamic_cast<ArrayIndexExprAst *>(expr); var != nullptr)
		return this->eval(var);

	return nullptr;
}

llvm::Type *Codegen::type(TypeExprAst *expr)
{
	llvm::Type *type = nullptr;
	
	if (auto basic = dynamic_cast<BasicTypeExprAst *>(expr->type.get()); basic != nullptr) {
		auto number_regex = std::regex("[iuf]([0-9]+)$");
		std::cmatch m;

		if (std::regex_match(basic->type.c_str(), m, number_regex)) {
			auto nbits = atoi(m[1].str().c_str());
			if (nbits <= 0)
				return nullptr;

			if (m[0].str()[0] == 'f') {
				switch (nbits) {
				case 32:
					type = this->builder.getFloatTy();
					break;
				case 64:
					type = this->builder.getDoubleTy();
					break;
				default:
					return nullptr;
				}
			} else {
				type = this->builder.getIntNTy(static_cast<unsigned int>(nbits));
			}
		} else if (basic->type == "str") {
			type = this->builder.getPtrTy();
		}
	} else if (auto arr = dynamic_cast<ArrayTypeExprAst *>(expr->type.get()); arr != nullptr) {
		auto recursing_type = this->type(arr->recursing_type.get());
		if (!recursing_type)
			return type;

		// TODO: Implement arrays with size
		//       Size-less arrays are just pointers
		// type = llvm::ArrayType::get(...);
		type = this->builder.getPtrTy();
	}

	return type;
}

llvm::Value *Codegen::eval(NumberExprAst *expr)
{
	llvm::Type *type;

	// TODO: Type inference or receive type in param
	type = builder.getInt32Ty(); 

	llvm::Constant *value;
	if (type->isIntegerTy()) {
		value = llvm::Constant::getIntegerValue(type, llvm::APInt(type->getIntegerBitWidth(), expr->number, 10));
	} else {
		value = llvm::ConstantFP::get(type, expr->number);
	}

	return value;
}

llvm::Value *Codegen::eval(ArrayIndexExprAst *expr)
{
	if (this->variables.find(expr->var->name) == this->variables.end())
		return nullptr;

	auto arr = this->variables[expr->var->name].second;
	if (!arr)
		return nullptr;

	auto loaded_arr = this->builder.CreateLoad(builder.getPtrTy(), arr, "__array_loaded__");

	auto index = this->eval(expr->index.get());
	if (!index)
		return nullptr;

	auto deref = this->builder.getInt32(0);
	auto type = llvm::ArrayType::get(this->builder.getPtrTy(), 64); // TODO: Just don't do this. This is such a hack
	auto bigtype = llvm::ArrayType::get(type, 64); // TODO: Just don't do this. This is such a hack
	auto ptr = this->builder.CreateGEP(bigtype, loaded_arr, { deref,  index, index }, "__array_deref__");
	auto ptr_load = this->builder.CreateLoad(builder.getPtrTy(), ptr, "__array_value__");

	return ptr_load;
}

bool Codegen::include(ExprAst *expr)
{
	if (auto decl_expr = dynamic_cast<DeclarationExprAst *>(expr); decl_expr != nullptr)
		return this->include(decl_expr);

	if (auto call_expr = dynamic_cast<CallExprAst *>(expr); call_expr != nullptr)
		return this->include(call_expr);

	return false;
}
