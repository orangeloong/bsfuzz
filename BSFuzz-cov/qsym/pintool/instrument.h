#ifndef QSYM_INSTRUMENT_H_
#define QSYM_INSTRUMENT_H_

#include "pin.H"

namespace qsym {

void PIN_FAST_ANALYSIS_CALL
instrumentJcc(
    ADDRINT pc,
    bool taken,
    ADDRINT target);

} // namespace qsym

#endif // QSYM_INSTRUMENT_H_

