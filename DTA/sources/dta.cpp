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

	void PrintNodes (std::ofstream &dumpOut, std::unordered_map<Vertex, std::set<Vertex, decltype(memInfo::cmpVert)>, std::hash<Vertex>, decltype(memInfo::cmpAddress)> &graph) {

		#if 1
		for (auto v : graph) {	
			dumpOut << "\"" << std::hex << v.first << "\"" << " " 
			<< "[shape = doublecircle lable = \"" << v.first << "\"]" << std::endl; 
		}
		#endif 
		
		for (auto v : graph) 
			for (auto u : v.second) 
				dumpOut << "\"" << v.first << "\"" << " -> " << "\"" << u.address_ << "\"" << ";" << std::endl;		
	
	}

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

	bool isColored (std::set <Vertex, decltype(memInfo::cmpVert)>::iterator toFind, std::set <Vertex, decltype(memInfo::cmpVert)>::iterator end) {

		if (toFind != end) 
			return true;

		return false;
	}
}
//----------------------------------------
//----------------------------------------

void memInfo::dumpGraph (const char *fileName) const {

	std::ofstream dumpOut(fileName, dumpOut.out | dumpOut.trunc);

	dumpOut << "digraph nodes {\n";

	PrintNodes(dumpOut, graph_);

	dumpOut << "}";
}

std::set <Vertex, decltype(memInfo::cmpVert)>::iterator memInfo::find (const QBDI::rword address) const { 

	if (vecs_.empty())
		return vecs_.end();

	Vertex find{address, sizeof (Vertex)};
	auto toFind = vecs_.lower_bound (find);

	if (toFind != vecs_.end()) {

		if ((*toFind).address_ == address) 
			return toFind;
		
		if (toFind == vecs_.begin()) 
			return vecs_.end();

		--toFind;
		if ((*toFind).address_ + (*toFind).size_ >= address) 
			return toFind;
	}

	toFind = std::prev (toFind);
	if ((*toFind).address_ + (*toFind).size_ >= address) 
		return toFind;
	
	return vecs_.end();
}
//----------------------------------------
//----------------------------------------
QBDI::VMAction enterSourceDetector (QBDI::VM *vm, QBDI::GPRState *gprState,
					  			    QBDI::FPRState *fprState, void* data) {
	
	memInfo *inf = static_cast<memInfo *> (data);

	inf->rbp_   = gprState->rbp; 
	// std::cout << "--- Calloc detected ---" << std::endl;
    return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction retMallocDetector (QBDI::VM *vm, QBDI::GPRState *gprState,
					  			  QBDI::FPRState *fprState, void* data) {
	
	memInfo *inf = static_cast<memInfo *> (data);

	if (inf->rbp_ == gprState->rbp) {
		std::cout << "\t\t---ALLOCATION: " << std::hex <<  gprState->rax << std::endl;
		inf->vecs_.emplace (gprState->rax, gprState->rdi);	
		inf->graph_.insert ({{gprState->rax, gprState->rdi}, std::set<Vertex, decltype(memInfo::cmpVert)>{}});
	}
	
    return QBDI::VMAction::CONTINUE;
}

std::ostream& operator << (std::ostream &out, const Vertex& vec) {

	out << vec.address_;
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
	if (inst->numOperands == 6 && inst->operands[5].type == QBDI::OPERAND_GPR) {

		QBDI::rword rhs = getReg (gprState, inst->operands[5].regCtxIdx);		  
 		QBDI::rword lhs = static_cast<long long> (getReg (gprState, inst->operands[0].regCtxIdx)) + static_cast<long long> (inst->operands[3].value);

		auto findLhs = info->find (lhs);
		auto findRhs = info->find (rhs);

		auto end = info->vecs_.end();
		
		if (isColored (findLhs, end) && isColored (findRhs, end)) {
			
			// QBDI::rword minAddr = std::min ((*findLhs).address_, (*findRhs).address_);
			// QBDI::rword maxAddr = std::max ((*findLhs).address_, (*findRhs).address_);

			auto findInGraph = info->graph_.find(Vertex{(*findLhs).address_, sizeof(Vertex)});
			auto res = findInGraph->second.insert (Vertex{(*findRhs).address_, sizeof (Vertex)});

		}
	}
#else
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

void monitorDependences (DBI::DTA &dta) {

	memInfo inf {}; 

    dta.vm_.addCodeAddrCB (reinterpret_cast <QBDI::rword> (source), QBDI::PREINST, enterSourceDetector, &inf);
	QBDI::rword retValue;

	uint32_t cid = dta.vm_.addCodeCB(QBDI::PREINST, showInstruction, nullptr);
  	assert(cid != QBDI::INVALID_EVENTID);
  	dta.vm_.addMnemonicCB("RET*", QBDI::PREINST, retMallocDetector, &inf);
  	dta.vm_.addMnemonicCB("MOV*", QBDI::PREINST, movDetector, &inf);
	
	bool res = dta.vm_.addInstrumentedModuleFromAddr(reinterpret_cast<QBDI::rword>(testSource));
  	assert(res == true);

	std::cout << "\t\t... Running  ..." << std::endl;
  	res = dta.vm_.call(&retValue, reinterpret_cast<QBDI::rword>(testSource));
	
	for (auto v : inf.vecs_)
	{

		std::cout << std::hex << v << std::endl;
	}
	// std::copy (inf.vecs_.begin(), inf.vecs_.end(), std::ostream_iterator<Vertex*> (std::cout, "\n"));

	inf.dumpGraph ("out.dot");
  	assert(res == true);

	// return root;
}
