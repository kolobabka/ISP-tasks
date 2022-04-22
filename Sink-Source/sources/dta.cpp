#include "dta.hpp"
#include "test.hpp"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
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

QBDI::rword getReg(QBDI::GPRState *state, QBDI::rword id) {

  if (id > 18)
    return id;

  QBDI::rword *regPtr = (QBDI::rword *)state;
  return *(regPtr + id);
}
} // namespace
//----------------------------------------
//----------------------------------------
namespace Memory {
Info::memIt Info::find(const QBDI::rword address) const {

  if (shadowMem_.empty())
    return shadowMem_.end();

  Memory find{address, sizeof(Memory)};
  auto toFind = shadowMem_.lower_bound(find);

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

  toFind = std::prev(toFind);
  if (toFind->address_ + toFind->size_ >= address)
    return toFind;

  return shadowMem_.end();
}
} // namespace Memory
//----------------------------------------
//----------------------------------------

QBDI::VMAction readDetector(QBDI::VM *vm, QBDI::GPRState *gprState,
                            QBDI::FPRState *fprState, void *data) {

  Memory::Info *info = static_cast<Memory::Info *>(data);
  const QBDI::InstAnalysis *inst = vm->getInstAnalysis(
      QBDI::ANALYSIS_INSTRUCTION | QBDI::ANALYSIS_DISASSEMBLY |
      QBDI::ANALYSIS_OPERANDS | QBDI::ANALYSIS_SYMBOL);

  auto memaccesses = vm->getInstMemoryAccess();

  if (memaccesses.empty() || inst->numOperands < 6)
    return QBDI::VMAction::CONTINUE;

  int16_t lhsIdx = inst->operands[0].regCtxIdx;
  QBDI::rword lhs = getReg(gprState, lhsIdx);
  QBDI::rword rhs = memaccesses[0].accessAddress;

  Memory::Info::changeShadowFunctor<Memory::Info::regIt, Memory::Info::memIt,
                                    int16_t>
      shadowChanger(info->shadowReg_.find(lhsIdx), info->find(rhs), lhsIdx);

  return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction writeDetector(QBDI::VM *vm, QBDI::GPRState *gprState,
                             QBDI::FPRState *fprState, void *data) {

  QBDI::rword lhs = 0, rhs = 0;
  Memory::Info *info = static_cast<Memory::Info *>(data);
  const QBDI::InstAnalysis *inst = vm->getInstAnalysis(
      QBDI::ANALYSIS_INSTRUCTION | QBDI::ANALYSIS_DISASSEMBLY |
      QBDI::ANALYSIS_OPERANDS | QBDI::ANALYSIS_SYMBOL);

  auto memaccesses = vm->getInstMemoryAccess();

  if (memaccesses.empty() || inst->numOperands < 6)
    return QBDI::VMAction::CONTINUE;

  int16_t rhsIdx = inst->operands[5].regCtxIdx;

  rhs = getReg(gprState, rhsIdx);
  lhs = memaccesses[0].accessAddress;

  Memory::Info::changeShadowFunctor<Memory::Info::memIt, Memory::Info::regIt,
                                    QBDI::rword>
      shadowChanger(info->find(lhs), info->shadowReg_.find(rhsIdx), lhs);

  return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction movDetector(QBDI::VM *vm, QBDI::GPRState *gprState,
                           QBDI::FPRState *fprState, void *data) {

  Memory::Info *info = static_cast<Memory::Info *>(data);
  const QBDI::InstAnalysis *inst = vm->getInstAnalysis(
      QBDI::ANALYSIS_INSTRUCTION | QBDI::ANALYSIS_DISASSEMBLY |
      QBDI::ANALYSIS_OPERANDS | QBDI::ANALYSIS_SYMBOL);

  if (inst->numOperands != 2 || inst->operands[1].type != QBDI::OPERAND_GPR)
    return QBDI::VMAction::CONTINUE;

  int16_t lhsIdx = inst->operands[0].regCtxIdx;
  int16_t rhsIdx = inst->operands[1].regCtxIdx;

  QBDI::rword lhs = getReg(gprState, lhsIdx);
  QBDI::rword rhs = getReg(gprState, rhsIdx);

  Memory::Info::changeShadowFunctor<Memory::Info::regIt, Memory::Info::regIt,
                                    int16_t>
      shadowChanger(info->shadowReg_.find(lhsIdx),
                    info->shadowReg_.find(rhsIdx), lhsIdx);

  return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction enterSourceDetector(QBDI::VM *vm, QBDI::GPRState *gprState,
                                   QBDI::FPRState *fprState, void *data) {

  Memory::Info *inf = static_cast<Memory::Info *>(data);

  inf->type_ = Memory::Info::FuncType::SOURCE;
  inf->size_ = gprState->rdi;

  vm->addMemAccessCB(QBDI::MEMORY_READ, readDetector, inf);
  vm->addMemAccessCB(QBDI::MEMORY_WRITE, writeDetector, inf);

  vm->addMnemonicCB("MOV*", QBDI::PREINST, movDetector, inf);

  return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction retSourceDetector(QBDI::VM *vm, QBDI::GPRState *gprState,
                                 QBDI::FPRState *fprState, void *data) {

  Memory::Info *inf = static_cast<Memory::Info *>(data);
  Memory::Info::FuncType &type = inf->type_;

  if (type == Memory::Info::FuncType::SOURCE) {

    type = Memory::Info::FuncType::NEUTRAL;
    QBDI::rword rax = gprState->rax;

    inf->shadowMem_.emplace(rax, inf->size_);
    inf->shadowReg_.emplace(0);
  }
  return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction enterSinkDetector(QBDI::VM *vm, QBDI::GPRState *gprState,
                                 QBDI::FPRState *fprState, void *data) {

  Memory::Info *info = static_cast<Memory::Info *>(data);

  auto argIt = info->shadowReg_.find(5);

  if (argIt != info->shadowReg_.cend())
    std::cout << "\t\t--- Leaking detected!!!" << std::endl;

  return QBDI::VMAction::CONTINUE;
}

std::ostream &operator<<(std::ostream &out, const Memory::Memory &vec) {

  out << vec.address_;
  return out;
}

Memory::Info leakDetector(DBI::DTA &dta) {

  Memory::Info inf{};

  dta.vm_.addCodeAddrCB(reinterpret_cast<QBDI::rword>(source), QBDI::PREINST,
                        enterSourceDetector, &inf);
  dta.vm_.addCodeAddrCB(reinterpret_cast<QBDI::rword>(sink), QBDI::PREINST,
                        enterSinkDetector, &inf);

  QBDI::rword retValue;

#ifdef DISASM
  uint32_t cid = dta.vm_.addCodeCB(QBDI::PREINST, showInstruction, nullptr);
  assert(cid != QBDI::INVALID_EVENTID);
#endif

  dta.vm_.addMnemonicCB("RET*", QBDI::PREINST, retSourceDetector, &inf);

  bool res = dta.vm_.addInstrumentedModuleFromAddr(
      reinterpret_cast<QBDI::rword>(testSink));
  assert(res == true);

  res = dta.vm_.call(&retValue, reinterpret_cast<QBDI::rword>(testSink));
  assert(res == true);

  return inf;
}
