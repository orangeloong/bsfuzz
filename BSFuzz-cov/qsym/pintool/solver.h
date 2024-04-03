#ifndef QSYM_SOLVER_H_
#define QSYM_SOLVER_H_


#include <fstream>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pin.H"

#include "afl_trace_map.h"

namespace qsym {

class Solver {
public:

  Solver(
      const std::string bitmap);

  void push();
  void reset();
  void pop();

  void addJcc(bool, ADDRINT, ADDRINT);

protected:
  AflTraceMap           trace_;
  ADDRINT               last_pc_;

};

extern Solver* g_solver;

} // namespace qsym

#endif // QSYM_SOLVER_H_

