#include "dta.hpp"
#include "test.hpp"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <vector>

#include <iterator>
//----------------------------------------
//----------------------------------------
namespace {

	QBDI::VMAction showInstruction(QBDI::VM *vm, QBDI::GPRState *gprState,
                               QBDI::FPRState *fprState, void *data) {
  		// Obtain an analysis of the instruction from the VM
 		const QBDI::InstAnalysis *instAnalysis = vm->getInstAnalysis();

  		// Printing disassembly
  		std::cout << std::setbase(16) << instAnalysis->address << ": "
            << instAnalysis->disassembly << std::endl
            << std::setbase(10);

  		return QBDI::VMAction::CONTINUE;
	}

	QBDI::rword getReg (QBDI::GPRState* state, QBDI::rword id) {
		
		if (id > 18) 
			throw std::runtime_error ("Unknown id");
		
		QBDI::rword* regPtr = (QBDI::rword*) state;

		return *(regPtr + id);
	}
}
//----------------------------------------
//----------------------------------------
QBDI::VMAction enterSourceDetector (QBDI::VM *vm, QBDI::GPRState *gprState,
					  			    QBDI::FPRState *fprState, void* data) {
	
	memInfo *inf = static_cast<memInfo *> (data);

	inf->enter_ = true;
	inf->rbp_   = gprState->rbp; 
	// std::cout << "--- Calloc detected ---" << std::endl;
    return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction sizeMallocDetector (QBDI::VM *vm, QBDI::GPRState *gprState,
					  			   QBDI::FPRState *fprState, void* data) {
	
	memInfo *inf = static_cast<memInfo *> (data);
	
	if (inf->enter_) 
		inf->size_ = gprState->rdi;
	
    return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction retMallocDetector (QBDI::VM *vm, QBDI::GPRState *gprState,
					  			  QBDI::FPRState *fprState, void* data) {
	
	memInfo *inf = static_cast<memInfo *> (data);

	if (inf->rbp_ == gprState->rbp) {

		inf->enter_ = false;
		inf->mem_ = gprState->rax;

		inf->vecs_.insert (new Vertex{inf->mem_, inf->size_});		
	}
	
    return QBDI::VMAction::CONTINUE;
}
std::ostream& operator << (std::ostream &out, const Vertex* vec) {

	out << vec->address_ << std::endl;

	return out;
}

QBDI::VMAction movDetector (QBDI::VM *vm, QBDI::GPRState *gprState,
					  		QBDI::FPRState *fprState, void* data) {

	memInfo *info = (memInfo *) data;

	// std::vector<QBDI::MemoryAccess> memaccesses = vm->getInstMemoryAccess();
	const QBDI::InstAnalysis* inst = vm->getInstAnalysis (QBDI::ANALYSIS_INSTRUCTION |
														  QBDI::ANALYSIS_DISASSEMBLY |
														  QBDI::ANALYSIS_OPERANDS	 |
														  QBDI::ANALYSIS_SYMBOL		 );

	#if 1
	if (inst->numOperands == 2 && inst->operands[1].type == QBDI::OPERAND_GPR) {

		QBDI::rword rhs = getReg (gprState, inst->operands[1].regCtxIdx);

		Vertex find{rhs, sizeof (Vertex)};
		auto toFind = info->vecs_.lower_bound (&find);
		std::cout << "\t\t---2 operands --- " << std::endl;
		std::copy (info->vecs_.begin(), info->vecs_.end(), std::ostream_iterator<Vertex*> (std::cout, "\n"));

		if (toFind != info->vecs_.end()) {

			if ((*toFind)->address_ == rhs) {
			std::cout << "\t\t--- NOT COLORED" << std::endl;

				info->coloredRegs_[inst->operands[0].regCtxIdx] = true;
				return QBDI::VMAction::CONTINUE; 
			}

			if (toFind == info->vecs_.begin()) {
				std::cout << "\t\t--- NOT COLORED" << std::endl;

				return QBDI::VMAction::CONTINUE;
			}
			
			--toFind;

			if ((*toFind)->address_ + (*toFind)->size_ >= rhs) {
				
				std::cout << "\t\t--- COLORED" << std::endl;
				info->coloredRegs_[inst->operands[0].regCtxIdx] = true;
				return QBDI::VMAction::CONTINUE; 
			}

			std::cout << "\t\t--- NOT COLORED" << std::endl;
			return QBDI::VMAction::CONTINUE;
		}
	}

	if (inst->numOperands == 6) {

		if (inst->operands[5].type == QBDI::OPERAND_GPR) {
			
			QBDI::rword rhs = getReg (gprState, inst->operands[5].regCtxIdx);

			Vertex find{rhs, sizeof (Vertex)};
			auto toFind = info->vecs_.lower_bound (&find);
			std::cout << "\t\t---6 operands and read--- " << std::endl;
			std::copy (info->vecs_.begin(), info->vecs_.end(), std::ostream_iterator<Vertex*> (std::cout));

			if (toFind != info->vecs_.end()) {

				if ((*toFind)->address_ == rhs) {
				std::cout << "\t\t--- COLORED" << std::endl;

					std::cout << "LEFT = " << (long long) getReg (gprState, inst->operands[0].regCtxIdx) << std::endl;
					std::cout << "RIGHT = " << static_cast<long long> (inst->operands[3].value) << std::endl; 

					QBDI::rword mem = (long long) getReg (gprState, inst->operands[0].regCtxIdx) + static_cast<long long> (inst->operands[3].value);
					info->vecs_.insert(new Vertex{mem, (*toFind)->size_});
					std::cout << "\t\t\t\tMEM = " << mem << std::endl;
					return QBDI::VMAction::CONTINUE; 
				}

				if (toFind == info->vecs_.begin()) {
					std::cout << "\t\t--- NOT COLORED" << std::endl;

					return QBDI::VMAction::CONTINUE;
				}
				
				--toFind;

				if ((*toFind)->address_ + (*toFind)->size_ >= rhs) {
					
					std::cout << "\t\t--- COLORED" << std::endl;
					QBDI::rword mem = (long long) getReg (gprState, inst->operands[0].regCtxIdx) + static_cast<long long> (inst->operands[3].value);
					info->vecs_.insert(new Vertex{mem, (*toFind)->size_});
					return QBDI::VMAction::CONTINUE; 
				}

				std::cout << "\t\t--- NOT COLORED" << std::endl;
				return QBDI::VMAction::CONTINUE;
			}
		}

		QBDI::rword rhs = (long long) getReg (gprState, inst->operands[1].regCtxIdx) + static_cast<long long> (inst->operands[4].value);

		std::cout << "\t\t\t\tRHS = " << rhs << std::endl;
		Vertex find{rhs, sizeof (Vertex)};
		auto toFind = info->vecs_.lower_bound (&find);
		std::cout << "\t\t---6 operands and write--- " << std::endl;
		std::copy (info->vecs_.begin(), info->vecs_.end(), std::ostream_iterator<Vertex*> (std::cout));

		if (toFind != info->vecs_.end()) {

			if ((*toFind)->address_ == rhs) {
			std::cout << "\t\t--- COLORED" << std::endl;

				info->coloredRegs_[inst->operands[0].regCtxIdx] = true;
				return QBDI::VMAction::CONTINUE; 
			}

			if (toFind == info->vecs_.begin()) {
				std::cout << "\t\t--- NOT COLORED" << std::endl;

				return QBDI::VMAction::CONTINUE;
			}
			
			--toFind;

			if ((*toFind)->address_ + (*toFind)->size_ >= rhs) {
				
				std::cout << "\t\t--- COLORED" << std::endl;
				info->coloredRegs_[inst->operands[0].regCtxIdx] = true;
				return QBDI::VMAction::CONTINUE; 
			}

			std::cout << "\t\t--- NOT COLORED" << std::endl;
			return QBDI::VMAction::CONTINUE;
		}
	}
	#endif
#if 0
	if (inst->numOperands == 2 && inst->operands[1].type == QBDI::OPERAND_GPR) {
		
		QBDI::rword rhs = getReg (gprState, inst->operands[1].regCtxIdx);

		Vertex find{rhs, sizeof (Vertex)};
		auto toFind = info->vecs_.lower_bound (&find);

		if (toFind != info->vecs_.end()) {

			std::cout << "Find it to write" << std::endl;
		}
	}

	if (inst->numOperands == 6) {

		for (int i = 0; i < 6; ++i) {


		}
	}
	switch (inst->operands[1].type) {

		case QBDI::OPERAND_GPR:
			std::cout << "\t\t ---OPERAND_GPR, NUM: " << (int)inst->numOperands << std::endl;
			std::cout << "\t\t --- Type: " << QBDI::OPERAND_GPR << std::endl;
			break;
		case QBDI::OPERAND_IMM:
			std::cout << "\t\t ---OPERAND_IMM, NUM: " << (int)inst->numOperands << std::endl;
			std::cout << "\t\t --- Type: " << QBDI::OPERAND_IMM << std::endl;
			break;
		case QBDI::OPERAND_PRED:
			std::cout << "\t\t ---OPERAND_PRED, NUM: " << (int)inst->numOperands << std::endl;
			std::cout << "\t\t --- Type: " << QBDI::OPERAND_GPR << std::endl;
			break;
		case QBDI::OPERAND_SEG:
			std::cout << "\t\t ---OPERAND_SEG, NUM: " << (int)inst->numOperands << std::endl;
			std::cout << "\t\t --- Type: " << QBDI::OPERAND_GPR << std::endl;
			break;
		case QBDI::OPERAND_FPR:
			std::cout << "\t\t ---OPERAND_FPR, NUM: " << (int)inst->numOperands << std::endl;
			std::cout << "\t\t --- Type: " << QBDI::OPERAND_GPR << std::endl;
			break;
		default:
			std::cout << "\t\t--- WHAT???" << std::endl;
	}
	std::cout << "\t\t\t--- MOV DETECTED" << std::endl;

	if (inst->numOperands == 6)
		for (int i = 0; i < 6; ++i) {

			std::cout << "\t\t[ " << i << " ]" << inst->operands[i].type << std::endl;

			if (inst->operands[i].type == QBDI::OPERAND_IMM)
				std::cout << "\t\t---Value: " << (long long)inst->operands[i].value << std::endl;
			if (inst->operands[i].type == QBDI::OPERAND_GPR) {
				std::cout << "\t\t---Reg Value: " << getReg (gprState, inst->operands[i].regCtxIdx) << std::endl;
				std::cout << "\t\t---Reg type : " << (long long)inst->operands[i].regAccess << std::endl;
			}

		}
	else {

		for (int i = 0; i < 2; ++i) {

			std::cout << "\t\t[ " << i << " ]" << inst->operands[i].type << std::endl;

			if (inst->operands[i].type == QBDI::OPERAND_IMM)
				std::cout << "\t\t---Value: " << (long long)inst->operands[i].value << std::endl;
		}	
	}
#endif
	return QBDI::VMAction::CONTINUE;
}

Graph* monitorDependences (DBI::DTA &dta) {

	memInfo inf {}; 

    dta.vm_.addCodeAddrCB (reinterpret_cast <QBDI::rword> (source), QBDI::PREINST, enterSourceDetector, &inf);
	QBDI::rword retValue;

	uint32_t cid = dta.vm_.addCodeCB(QBDI::PREINST, showInstruction, nullptr);
  	assert(cid != QBDI::INVALID_EVENTID);
  	dta.vm_.addMnemonicCB("CALL*", QBDI::PREINST, sizeMallocDetector, &inf);
  	dta.vm_.addMnemonicCB("RET*", QBDI::PREINST, retMallocDetector, &inf);
  	dta.vm_.addMnemonicCB("MOV*", QBDI::PREINST, movDetector, &inf);
	// dta.vm_.addMemAccessCB (QBDI::MEMORY_READ, memoryReadDetector, &inf);
	// dta.vm_.addMemAccessCB (QBDI::MEMORY_WRITE, memoryWriteDetector, &inf);

	
	bool res = dta.vm_.addInstrumentedModuleFromAddr(reinterpret_cast<QBDI::rword>(testSource));
  	assert(res == true);

	std::cout << "\t\t... Running  ..." << std::endl;
  	res = dta.vm_.call(&retValue, reinterpret_cast<QBDI::rword>(testSource));
	
	for (auto v : inf.vecs_)
	{

		std::cout << v;
	}
	// std::copy (inf.vecs_.begin(), inf.vecs_.end(), std::ostream_iterator<Vertex*> (std::cout, "\n"));
  	assert(res == true);

	// return root;
}
