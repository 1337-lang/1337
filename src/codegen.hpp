#ifndef _CODEGEN_HPP_
#define _CODEGEN_HPP_

#include "llvm.hpp"
#include "ast.hpp"
#include <map>

class Codegen {
private:
	llvm::LLVMContext context;
	llvm::Module module;
	llvm::IRBuilder<> builder;
	std::unique_ptr<llvm::IRBuilder<>> block_builder = nullptr;
	std::map<std::string, llvm::Value *> globals;
public:
	inline Codegen()
		: context(), builder(this->context), module("<module>", this->context)
	{}
public:
	bool include(ExprAst *expr);
	bool include(DeclarationExprAst *expr);

	inline void dump()
	{
		this->module.dump();
	}
};

#endif
