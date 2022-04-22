#include "dta.hpp"
#include "test.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <iterator>
//----------------------------------------
//----------------------------------------
namespace {
	QBDI::VMAction showInstruction(QBDI::VM *vm, QBDI::GPRState *gprState,
                               	   QBDI::FPRState *fprState, void *data) {

 		const QBDI::InstAnalysis *instAnalysis = vm->getInstAnalysis();

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

	inline bool isPadding (std::set <Memory::Memory>::iterator toFind, 
					std::set <Memory::Memory>::iterator end) {

		return toFind != end ? true : false;
	}
}
//----------------------------------------
//----------------------------------------
namespace Memory {
	

    bool operator < (const Memory& lhs, const Memory& rhs) {

        return lhs.address_ < rhs.address_;
    }

	Info::memIt Info::find (const QBDI::rword address) const { 

		if (shadowMem_.empty())
			return shadowMem_.end();

		Memory find{address, sizeof (Memory)};
		auto toFind = shadowMem_.lower_bound (find);

		if (toFind != shadowMem_.end()) {

			if (toFind->address_ == address) 
				return toFind;
			
			if (toFind == shadowMem_.begin()) 
				return shadowMem_.end();

			--toFind;
			if (toFind->address_ + toFind->size_ >= address) 
				return toFind;
			
			return shadowMem_.end();
		}

		toFind = std::prev (toFind);
		if (toFind->address_ + toFind->size_ >= address) 
			return toFind;
		
		return shadowMem_.end();
	}
}
//----------------------------------------
//----------------------------------------

QBDI::VMAction readDetector (QBDI::VM *vm, QBDI::GPRState *gprState,
				   			 QBDI::FPRState *fprState, void* data) {

	Memory::Info* info = static_cast<Memory::Info*>(data);
	const QBDI::InstAnalysis* inst = vm->getInstAnalysis (QBDI::ANALYSIS_INSTRUCTION |
														  QBDI::ANALYSIS_DISASSEMBLY |
														  QBDI::ANALYSIS_OPERANDS	 |
														  QBDI::ANALYSIS_SYMBOL		 );

	auto memaccesses = vm->getInstMemoryAccess();

	if (memaccesses.empty() || inst->numOperands < 6)
		return QBDI::VMAction::CONTINUE;

	int16_t lhsIdx = inst->operands[0].regCtxIdx;
	QBDI::rword	lhs = getReg (gprState, lhsIdx);
	QBDI::rword rhs = memaccesses[0].accessAddress;

	auto findLhs = info->shadowReg_.find (lhsIdx);
	auto findRhs = info->find (rhs);
	auto endLhs  = info->shadowReg_.end ();
	auto endRhs  = info->shadowMem_.end ();  

	Memory::Info::changeShadowFunctor<Memory::Info::regIt,
									  Memory::Info::memIt,
									  QBDI::rword> shadowChanger(info->shadowReg_.find (lhsIdx), info->find (rhs), lhs);
	
	// shadowChanger (info->shadowReg_.find (lhsIdx), info->find (rhs), lhs);
	// if (findLhs == endLhs && findRhs != endRhs) {

	// 	info->shadowReg_.emplace (lhsIdx);
	// 	return QBDI::VMAction::CONTINUE;
	// }
	// if (findRhs == endRhs) 
	// 	if (findLhs == endLhs)
	// 		return QBDI::VMAction::CONTINUE;
	// 	else 
	// 		info->shadowReg_.erase (findLhs);	

	return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction writeDetector (QBDI::VM *vm, QBDI::GPRState *gprState,
				   			  QBDI::FPRState *fprState, void* data) {

	QBDI::rword lhs = 0, rhs = 0;
	Memory::Info* info = static_cast<Memory::Info*>(data);
	const QBDI::InstAnalysis* inst = vm->getInstAnalysis (QBDI::ANALYSIS_INSTRUCTION |
														  QBDI::ANALYSIS_DISASSEMBLY |
														  QBDI::ANALYSIS_OPERANDS	 |
														  QBDI::ANALYSIS_SYMBOL		 );
	auto memaccesses = vm->getInstMemoryAccess();

	if (memaccesses.empty() || inst->numOperands < 6)
		return QBDI::VMAction::CONTINUE;

	int16_t rhsIdx = inst->operands[5].regCtxIdx;
	if (rhsIdx == -1) 	
		rhs = inst->operands[5].value;
	
	else
		rhs = getReg (gprState, rhsIdx);
	// if (rhsIdx >= 19)	
	// 	return QBDI::VMAction::CONTINUE;
	lhs = memaccesses[0].accessAddress;

 	Memory::Info::changeShadowFunctor<Memory::Info::memIt,
									  Memory::Info::regIt,
									  QBDI::rword> shadowChanger(info->find (lhs), info->shadowReg_.find (rhsIdx), lhs);


	// std::cout << sizeof (shadowChanger) << std::endl;

	// if (findLhs == endLhs && findRhs != endRhs) {

	// 	info->shadowMem_.emplace (lhs);
	// 	return QBDI::VMAction::CONTINUE;
	// }
	// if (findRhs == endRhs) 
	// 	if (findLhs == endLhs)
	// 		return QBDI::VMAction::CONTINUE;
	// 	else 
	// 		info->shadowMem_.erase (findLhs);	

	return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction movDetector(QBDI::VM *vm, QBDI::GPRState *gprState,
				   			QBDI::FPRState *fprState, void* data) {

	Memory::Info* info = static_cast<Memory::Info*>(data);
	const QBDI::InstAnalysis* inst = vm->getInstAnalysis (QBDI::ANALYSIS_INSTRUCTION |
														  QBDI::ANALYSIS_DISASSEMBLY |
														  QBDI::ANALYSIS_OPERANDS	 |
														  QBDI::ANALYSIS_SYMBOL		 );

	if (inst->numOperands != 2 || inst->operands[1].type != QBDI::OPERAND_GPR)
		return QBDI::VMAction::CONTINUE;

	int16_t lhsIdx = inst->operands[0].regCtxIdx;
	int16_t rhsIdx = inst->operands[1].regCtxIdx;

	QBDI::rword lhs = getReg (gprState, lhsIdx);
	QBDI::rword rhs = getReg (gprState, rhsIdx);

	Memory::Info::changeShadowFunctor<Memory::Info::regIt,
									  Memory::Info::regIt,
									  QBDI::rword> shadowChanger(info->shadowReg_.find (lhsIdx), info->shadowReg_.find (rhsIdx), lhs);
		
	// if (findLhs == end && findRhs != end) {

	// 	info->shadowReg_.emplace (lhsIdx);
	// 	return QBDI::VMAction::CONTINUE;
	// }
	// if (findRhs == end) 
	// 	if (findLhs == end)
	// 		return QBDI::VMAction::CONTINUE;
	// 	else 
	// 		info->shadowReg_.erase (findLhs);	

	return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction enterSourceDetector (QBDI::VM *vm, QBDI::GPRState *gprState,
					  			    QBDI::FPRState *fprState, void* data) {
		
	Memory::Info *inf = static_cast<Memory::Info *> (data);
	
	std::cout << "\t\t--- SOURCE DETECTED" << std::endl;
	inf->type_ = Memory::Info::FuncType::SOURCE; 
	inf->size_ = gprState->rdi;

	vm->addMemAccessCB(QBDI::MEMORY_READ,  readDetector,  inf);
	vm->addMemAccessCB(QBDI::MEMORY_WRITE, writeDetector, inf);

	vm->addMnemonicCB("MOV*", QBDI::PREINST, movDetector, inf);

    return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction retSourceDetector (QBDI::VM *vm, QBDI::GPRState *gprState,
					  			  QBDI::FPRState *fprState, void* data) {
	
	Memory::Info *inf = static_cast<Memory::Info *> (data);
	Memory::Info::FuncType &type = inf->type_;
	
	if (type == Memory::Info::FuncType::SOURCE) {

		std::cout << "\t\t--Here" << std::endl;
		type = Memory::Info::FuncType::NEUTRAL;
		QBDI::rword rax = gprState->rax;

		// inf->shadowMem_.emplace (rax, inf->size_);
	}
    return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction enterSinkDetector (QBDI::VM *vm, QBDI::GPRState *gprState,
					  		  	  QBDI::FPRState *fprState, void* data) {

	Memory::Info *info = static_cast<Memory::Info *> (data);

	auto argIt = info->shadowReg_.find (5);

	if (argIt != info->shadowReg_.cend()) 
		std::cout << "\t\t\t####POPALSYA" << std::endl;

	return QBDI::VMAction::CONTINUE;
}

std::ostream& operator << (std::ostream &out, const Memory::Memory& vec) {

	out << vec.address_;
	return out;
}

Memory::Info leakDetector (DBI::DTA &dta) {

	Memory::Info inf {}; 

    dta.vm_.addCodeAddrCB (reinterpret_cast <QBDI::rword> (source), QBDI::PREINST, enterSourceDetector, &inf);
    dta.vm_.addCodeAddrCB (reinterpret_cast <QBDI::rword> (sink), QBDI::PREINST, enterSinkDetector, &inf);

	QBDI::rword retValue;

#ifndef DISASM
	uint32_t cid = dta.vm_.addCodeCB(QBDI::PREINST, showInstruction, nullptr);
  	assert(cid != QBDI::INVALID_EVENTID);
#endif

  	dta.vm_.addMnemonicCB("RET*", QBDI::PREINST, retSourceDetector, &inf);

  	// dta.vm_.addMnemonicCB("MOV*", QBDI::PREINST, movDetector, &inf);
	
	bool res = dta.vm_.addInstrumentedModuleFromAddr(reinterpret_cast<QBDI::rword>(testSink));
  	assert(res == true);

  	res = dta.vm_.call(&retValue, reinterpret_cast<QBDI::rword>(testSink));
  	assert(res == true);

	return inf;
}
