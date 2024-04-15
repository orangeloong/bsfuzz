#include <set>
#include <byteswap.h>
#include "solver.h"

namespace qsym
{

	Solver::Solver(
		const std::string bitmap)
		: trace_(bitmap), last_pc_(0)
	{
	}


	void Solver::addJcc(bool taken, ADDRINT pc, ADDRINT target)
	{
		// Save the last instruction pointer for debugging
		last_pc_ = pc;
		if (pc != 0)
			trace_.isInterestingBranch(pc, target, taken);

	}
} // namespace qsym

