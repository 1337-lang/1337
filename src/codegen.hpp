#ifndef _CODEGEN_HPP_
#define _CODEGEN_HPP_

#include "llvm.hpp"
#include "ast.hpp"
#include <map>
#include <iostream>
#include <utility>

class Codegen {
private:
	llvm::LLVMContext context;
	llvm::Module module;
	llvm::IRBuilder<> builder;
	std::map<std::string, std::pair<llvm::Type *, llvm::Value *>> variables;
public:
	inline Codegen()
		: context(), builder(this->context), module("<module>", this->context)
	{
		// Add printf declaration
		auto printf_type = llvm::FunctionType::get(builder.getInt32Ty(), { builder.getPtrTy() }, true);
		auto printf_func = llvm::Function::Create(printf_type, llvm::Function::ExternalLinkage, "printf", module);
	}
public:
	bool include(ExprAst *expr);
	bool include(DeclarationExprAst *expr);
	bool include(CallExprAst *expr);
	llvm::Value *eval(StringExprAst *expr);
	llvm::Value *eval(ExprAst *expr);
	llvm::Value *eval(VariableExprAst *expr);
	llvm::Type *type(TypeExprAst *expr);

	inline void dump()
	{
		this->module.dump();
	}

	inline bool write_object(std::string path)
	{
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargets();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllAsmParsers();
		llvm::InitializeAllAsmPrinters();
		std::string error;
		auto triple = llvm::sys::getDefaultTargetTriple();
		auto target = llvm::TargetRegistry::lookupTarget(triple, error);
		llvm::TargetOptions opt;
		auto target_machine = target->createTargetMachine(triple, "generic", "", opt, llvm::Reloc::PIC_);
		module.setDataLayout(target_machine->createDataLayout());

		// Generate object
		std::error_code errcode;
		auto output_file = llvm::raw_fd_ostream(path, errcode, llvm::sys::fs::OF_None);
		if (errcode) {
			std::cout << "failed to open object file" << std::endl;
			return false;
		}

		llvm::legacy::PassManager pass;
		if (target_machine->addPassesToEmitFile(pass, output_file, nullptr, llvm::CodeGenFileType::ObjectFile)) {
			std::cout << "failed to add passes to emit file" << std::endl;
			return false;
		}

		pass.run(module);
		output_file.flush();

		std::cout << "Successfully created object file 'output.o'" << std::endl;

		return true;
	}
};

#endif
