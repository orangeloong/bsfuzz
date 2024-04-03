#ifndef QSYM_ANALYSIS_INSTRUCTION_H_
#define QSYM_ANALYSIS_INSTRUCTION_H_

#include "pin.H"
#include "compiler.h"

namespace qsym {
enum {
  OP_0 = 0,
  OP_1 = 1,
  OP_2 = 2,
  OP_3 = 3,
  OP_4 = 4,
  OP_5 = 5,
};

void analyzeJcc(INS ins, bool inv);

} // namespace qsym

#endif // QSYM_ANALYZE_INSTRUCTION_H_

