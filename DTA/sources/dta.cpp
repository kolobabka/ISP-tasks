#include "dta.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

#include "test.hpp"
//----------------------------------------
//----------------------------------------
namespace {
    void PrintNodes (
        std::ofstream &dumpOut,
        const std::unordered_map<
            QBDI::rword,
            std::set<Memory::Data, decltype (Memory::Info::cmpData)>>
            &graph)
    {
        for (auto v : graph) {
            dumpOut << "\"" << std::hex << v.first << "\""
                    << " "
                    << "[shape = doublecircle lable = \"" << v.first << "\"]"
                    << std::endl;
        }

        for (auto v : graph)
            for (auto u : v.second)
                dumpOut << "\"" << v.first << "\""
                        << " -> "
                        << "\"" << u << "\""
                        << ";" << std::endl;
    }

    QBDI::VMAction showInstruction (QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data)
    {
        const QBDI::InstAnalysis *instAnalysis = vm->getInstAnalysis ();

        std::cout << std::setbase (16) << instAnalysis->address << ": "
                  << instAnalysis->disassembly << std::endl
                  << std::setbase (10);

        return QBDI::VMAction::CONTINUE;
    }

    QBDI::rword getReg (QBDI::GPRState *state, QBDI::rword id)
    {
        if (id > 18)
            throw std::runtime_error ("Unknown id");

        QBDI::rword *regPtr = (QBDI::rword *)state;
        return *(regPtr + id);
    }

    inline bool isPadding (
        std::set<Memory::Memory, decltype (Memory::Info::cmpVert)>::iterator toFind,
        std::set<Memory::Memory, decltype (Memory::Info::cmpVert)>::iterator end)
    {
        return toFind != end ? true : false;
    }
}  // namespace
//----------------------------------------
//----------------------------------------
namespace Memory {
    void Info::dumpGraph (const std::string &fileName) const
    {
        std::ofstream dumpOut (fileName, dumpOut.out | dumpOut.trunc);

        dumpOut << "digraph nodes {\n";

        PrintNodes (dumpOut, graph_);

        dumpOut << "}";
    }

    std::set<Memory, decltype (Info::cmpVert)>::iterator
    Info::find (const QBDI::rword address) const
    {
        if (pointers_.empty ())
            return pointers_.end ();

        Memory find{address, sizeof (Memory)};
        auto toFind = pointers_.lower_bound (find);

        if (toFind != pointers_.end ()) {
            if (toFind->address_ == address)
                return toFind;

            if (toFind == pointers_.begin ())
                return pointers_.end ();

            --toFind;
            if (toFind->address_ + toFind->size_ >= address)
                return toFind;

            return pointers_.end ();
        }

        toFind = std::prev (toFind);
        if (toFind->address_ + toFind->size_ >= address)
            return toFind;

        return pointers_.end ();
    }

    void Info::eracePointer (const QBDI::rword address)
    {
        pointers_.erase (Memory{address, 42});

        auto toDelete = graph_.find (address);
        graph_.erase (toDelete);
    }

    void Info::graphUnwinding (const QBDI::rword address)
    {
        auto pred = [address] (auto x) { return x.address_ == address; };

        for (auto &v : graph_) {
            auto &graphNodes = v.second;

            for (auto curIt = graphNodes.begin (), endIt = graphNodes.end ();
                 curIt != endIt;)
                if (pred (*curIt))
                    curIt = graphNodes.erase (curIt);
                else
                    ++curIt;
        }
    }

    void Info::emplaceIntoGraph (const QBDI::rword lhsAddress,
                                 const QBDI::rword rhsAddress,
                                 const QBDI::rword offset)
    {
        auto findInGraph = graph_.find (lhsAddress);
        auto &graphNodes = findInGraph->second;

        if (graphNodes.empty ()) {
            graphNodes.emplace (rhsAddress, offset);
            return;
        }

        auto findData = graphNodes.find ({rhsAddress, offset});

        if (findData != graphNodes.end ()) {
            graphNodes.erase (findData);
            graphNodes.emplace (rhsAddress, offset);
        }
        else {
            graphNodes.emplace (rhsAddress, offset);
        }
    }
}  // namespace Memory
//----------------------------------------
//----------------------------------------
QBDI::VMAction enterSourceDetector (QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data)
{
    Memory::Info *inf = static_cast<Memory::Info *> (data);

    inf->type_ = Memory::Info::FuncType::CTOR;
    return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction enterDestructorDetector (QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data)
{
    Memory::Info *inf = static_cast<Memory::Info *> (data);

    inf->type_ = Memory::Info::FuncType::DTOR;
    return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction retMallocDetector (QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data)
{
    Memory::Info *inf = static_cast<Memory::Info *> (data);
    Memory::Info::FuncType &type = inf->type_;

    if (type == Memory::Info::FuncType::CTOR) {
        type = Memory::Info::FuncType::NEUTRAL;
        QBDI::rword rax = gprState->rax;

        inf->pointers_.emplace (rax, gprState->rdi);
        inf->graph_.insert (
            {rax, std::set<Memory::Data, decltype (Memory::Info::cmpData)>{}});

        inf->dumpGraph ("out" + std::to_string (inf->numOfLog++) + ".dot");
    }
    return QBDI::VMAction::CONTINUE;
}

QBDI::VMAction callDestructDetector (QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data)
{
    Memory::Info *inf = static_cast<Memory::Info *> (data);
    Memory::Info::FuncType &type = inf->type_;

    if (type == Memory::Info::FuncType::DTOR) {
        type = Memory::Info::FuncType::NEUTRAL;

        QBDI::rword address = gprState->rdi;

        inf->eracePointer (address);
        inf->graphUnwinding (address);

        inf->dumpGraph ("out" + std::to_string (inf->numOfLog++) + ".dot");
    }
    return QBDI::VMAction::CONTINUE;
}

std::ostream &operator<< (std::ostream &out, const Memory::Data &vec)
{
    out << vec.address_;
    return out;
}

QBDI::VMAction movDetector (QBDI::VM *vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data)
{
    Memory::Info *info = static_cast<Memory::Info *> (data);
    const QBDI::InstAnalysis *inst = vm->getInstAnalysis (
        QBDI::ANALYSIS_INSTRUCTION | QBDI::ANALYSIS_DISASSEMBLY |
        QBDI::ANALYSIS_OPERANDS | QBDI::ANALYSIS_SYMBOL);

    if (inst->numOperands == 6 && inst->operands[5].type == QBDI::OPERAND_GPR) {
        QBDI::rword rhs = getReg (gprState, inst->operands[5].regCtxIdx);
        QBDI::rword lhs =
            static_cast<long long> (getReg (gprState, inst->operands[0].regCtxIdx)) +
            static_cast<long long> (inst->operands[3].value);

        auto findLhs = info->find (lhs);
        auto findRhs = info->find (rhs);
        auto end = info->pointers_.end ();

        if (!(isPadding (findLhs, end)) || !(isPadding (findRhs, end)))
            return QBDI::VMAction::CONTINUE;

        QBDI::rword offset = lhs - findLhs->address_;
        info->emplaceIntoGraph (findLhs->address_, findRhs->address_, offset);

        info->dumpGraph ("out" + std::to_string (info->numOfLog++) + ".dot");
    }

    return QBDI::VMAction::CONTINUE;
}

Memory::Info monitorDependences (DBI::DTA &dta)
{
    Memory::Info inf{};

    dta.vm_.addCodeAddrCB (reinterpret_cast<QBDI::rword> (source), QBDI::PREINST, enterSourceDetector, &inf);
    dta.vm_.addCodeAddrCB (reinterpret_cast<QBDI::rword> (destructor),
                           QBDI::PREINST,
                           enterDestructorDetector,
                           &inf);
    QBDI::rword retValue;

#ifdef DISASM
    uint32_t cid = dta.vm_.addCodeCB (QBDI::PREINST, showInstruction, nullptr);
    assert (cid != QBDI::INVALID_EVENTID);
#endif

    dta.vm_.addMnemonicCB ("RET*", QBDI::PREINST, retMallocDetector, &inf);
    dta.vm_.addMnemonicCB ("CALL*", QBDI::PREINST, callDestructDetector, &inf);
    dta.vm_.addMnemonicCB ("MOV*", QBDI::PREINST, movDetector, &inf);

    bool res = dta.vm_.addInstrumentedModuleFromAddr (
        reinterpret_cast<QBDI::rword> (testSource));
    assert (res == true);

    res = dta.vm_.call (&retValue, reinterpret_cast<QBDI::rword> (testSource));
    assert (res == true);

    return inf;
}
