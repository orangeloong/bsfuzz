#include <cassert>
#include "analysis_instruction.h"
#include "instrument.h"

namespace qsym {

  void analyzeJcc(INS ins, bool inv) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentJcc,
        IARG_FAST_ANALYSIS_CALL,
        IARG_INST_PTR,
        IARG_BRANCH_TAKEN,
        IARG_BRANCH_TARGET_ADDR,
        IARG_END);
  }


} // namespace qsym

