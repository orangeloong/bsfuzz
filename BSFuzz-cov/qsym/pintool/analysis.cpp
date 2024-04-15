#include <iostream>
#include "analysis.h"
#include "analysis_instruction.h"

namespace qsym
{
  void
  analyzeTrace(TRACE trace, VOID *v)
  {
    IMG img = IMG_FindByAddress(TRACE_Address(trace));

    //if (!IMG_Valid(img) || !IMG_IsMainExecutable(img))
    if (!IMG_Valid(img))
      return;
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
      //analyzeBBL(bbl);
      for (INS ins = BBL_InsHead(bbl);
           INS_Valid(ins);
           ins = INS_Next(ins))
      {
        analyzeInstruction(ins);
      }
    }
  }

  void
  analyzeInstruction(INS ins)
  {
    // use XED to decode the instruction and extract its opcode
    xed_iclass_enum_t ins_indx = (xed_iclass_enum_t)INS_Opcode(ins);

    if (ins_indx <= XED_ICLASS_INVALID ||
        ins_indx >= XED_ICLASS_LAST)
    {
      LOG_WARN("unknown opcode (opcode=" + decstr(ins_indx) + ")" + "\n");
      return;
    }

    switch (ins_indx)
    {
    case XED_ICLASS_JB:
      analyzeJcc(ins, false);
      break;
    case XED_ICLASS_JBE:
      analyzeJcc(ins, false);
      break;
    case XED_ICLASS_JL:
      analyzeJcc(ins, false);
      break;
    case XED_ICLASS_JLE:
      analyzeJcc(ins, false);
      break;
  
    case XED_ICLASS_JNB:
      analyzeJcc(ins, true);
      break;
    case XED_ICLASS_JNBE:
      analyzeJcc(ins, true);
      break;
    case XED_ICLASS_JNL:
      analyzeJcc(ins, true);
      break;
    case XED_ICLASS_JNLE:
      analyzeJcc(ins, true);
      break;
    case XED_ICLASS_JNO:
      analyzeJcc(ins, true);
      break;
    case XED_ICLASS_JNP:
      analyzeJcc(ins, true);
      break;
    case XED_ICLASS_JNS:
      analyzeJcc(ins, true);
      break;
    case XED_ICLASS_JNZ:
      analyzeJcc(ins, true);
      break;
    case XED_ICLASS_JO:
      analyzeJcc(ins, false);
      break;
    case XED_ICLASS_JP:
      analyzeJcc(ins, false);
      break;
    // XED_ICLASS_JRCXZ,
    case XED_ICLASS_JS:
      analyzeJcc(ins, false);
      break;
    case XED_ICLASS_JZ:
      analyzeJcc(ins, false);
      break;

    default:
      break;
    }
    return;
  }

} // namespace qsym

