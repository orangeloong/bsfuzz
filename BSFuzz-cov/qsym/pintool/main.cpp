#include <cstdio>
#include <errno.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <set>

#include "solver.h"
#include "analysis.h"

namespace qsym {
  const int     kExitFailure = -1;
  const char*   kDlibSuffix = ".so";


  REG           g_thread_context_reg;
  Solver        *g_solver;
}

#define SYS_SOCKET 1

using namespace qsym;

static KNOB<size_t> g_opt_stdin(KNOB_MODE_WRITEONCE, "pintool",
    "s", "0", "track stdin");
static KNOB<size_t> g_opt_fs(KNOB_MODE_WRITEONCE, "pintool",
    "f", "0", "tracking file name");
static KNOB<size_t> g_opt_net(KNOB_MODE_WRITEONCE, "pintool",
    "n", "0", "track net");
static KNOB<std::string> g_opt_input(KNOB_MODE_WRITEONCE, "pintool",
    "i", "", "input");
static KNOB<string> g_opt_outdir(KNOB_MODE_WRITEONCE, "pintool",
    "o", "", "output directory");
static KNOB<string> g_opt_bitmap(KNOB_MODE_WRITEONCE, "pintool",
    "b", "", "bitmap file");
static KNOB<int> g_opt_linearization(KNOB_MODE_WRITEONCE, "pintool",
    "l", "0", "turn on linearization");

namespace {

bool checkOpt() {
  bool b1 = g_opt_stdin.Value() != 0;
  bool b2 = g_opt_fs.Value() != 0;
  bool b3 = g_opt_net.Value() != 0;

  if (g_opt_input.Value().empty()) {
    LOG_INFO("No input is specified\n");
    return false;
  }

  // one of them should be true
  if (!b1 && !b2 && !b3) {
    LOG_INFO("No option is specified: use stdin\n");
    g_opt_stdin.AddValue("1");
    return true;
  }

  // three of them cannot be true at the same time
  if (b1 && b2 && b3)
    goto multiple_opt;

  // if two of them are true, then false
  // else one of them are true, then true
  if (b1 ^ b2 ^ b3)
    return true;

multiple_opt:
  LOG_INFO("More than one exclusive options are specified\n");
  return false;
}

void initializeGlobalContext(
    const std::string bitmap) {
  g_solver = new Solver(bitmap);

}


} // anonymous namespace

int main(int argc, char** argv) {
  PIN_InitSymbols();

  if (PIN_Init(argc, argv))
    goto err;

  if (!checkOpt())
    goto err;

  initializeGlobalContext(
      g_opt_bitmap.Value());

  TRACE_AddInstrumentFunction(analyzeTrace, NULL);
  
  PIN_StartProgram();

err:
  PIN_ERROR(KNOB_BASE::StringKnobSummary() + "\n");
  return kExitFailure;
}

