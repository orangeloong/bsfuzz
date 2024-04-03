#include <cassert>
#include "instrument.h"
#include "solver.h"
#include "third_party/pin/utils.h"

// bit position
namespace qsym {

  void
  instrumentJcc(ADDRINT pc,
                bool taken, ADDRINT target)
  {
    g_solver->addJcc(taken, pc, target);
  }


} // namespace qsym

