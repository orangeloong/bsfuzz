#include "afl_trace_map.h"

namespace qsym
{

  const int kMapSize = 65536;    
  int avg = 65535;
  
  static KNOB<bool> g_opt_context_sensitive(KNOB_MODE_WRITEONCE, "pintool",
                                            "context_sensitive", "1", "Generate testcases by awaring of contexts");

  namespace
  {

    inline bool isPowerOfTwoOrZero(ADDRINT x)
    {
      return ((x & (x - 1)) == 0);
    }

    XXH32_hash_t hashPc(ADDRINT pc, bool taken)
    {
      // mimic afl's branch map
      PIN_LockClient();
      IMG img = IMG_FindByAddress(pc);
      PIN_UnlockClient();

      // hopefully IMG_Id is same for every execution
      if (!IMG_Valid(img))
        LOG_FATAL("invalid image");

      pc -= IMG_LowAddress(img);

      UINT32 img_id = IMG_Id(img);
      XXH32_state_t state;
      XXH32_reset(&state, 0); // seed = 0
      XXH32_update(&state, &pc, sizeof(pc));
      XXH32_update(&state, &img_id, sizeof(img_id));
      XXH32_update(&state, &taken, sizeof(taken));
      return XXH32_digest(&state) % kMapSize;
    }

  } // namespace

  void AflTraceMap::allocMap()
  {
    //trace_map_ = (UINT8 *)safeMalloc(kMapSize);
    virgin_map_ = (UINT8 *)safeMalloc(kMapSize);
    context_map_ = (UINT8 *)safeMalloc(kMapSize);
    aflcov_map_ = (UINT16 *)safeMalloc(2*kMapSize);
    // unsolved_ = (UINT16 *)safeMalloc(2);
    // memset(unsolved_, 65535, 2);
    memset(virgin_map_, 0, kMapSize);
  }

  void AflTraceMap::setDefault()
  {
    //memset(trace_map_, 0, kMapSize);
    memset(context_map_, 0, kMapSize);
    memset(aflcov_map_, 0, 2*kMapSize);
  }

  void AflTraceMap::import(const std::string path)
  {
    ifstream ifs;
    ifs.open(path, ios::binary);
    if (ifs.fail())
    {
      LOG_DEBUG("cannot read a file, so use a default trace map\n");
      setDefault();
      return;
    }
    //ifs.read((char *)trace_map_, kMapSize);
    ifs.read((char *)context_map_, kMapSize);
    ifs.read(reinterpret_cast<char *>(aflcov_map_), 2*kMapSize);
    if (!ifs)
      setDefault();
    else {
      // average
      int sum = 0;
      avg = 0;
      for(int i = 0; i < kMapSize; i++){
        if(aflcov_map_[i] != 0 && aflcov_map_[i] < 65534) {
          avg += aflcov_map_[i];
          sum++;
        }
      }
      avg /= sum;
    }
    ifs.close();


  }

  ADDRINT AflTraceMap::getIndex(ADDRINT h)
  {
    return ((prev_loc_ >> 1) ^ h) % kMapSize;
  }


  bool AflTraceMap::isInterestingContext(ADDRINT h, ADDRINT bits)
  {
    if (!g_opt_context_sensitive.Value())
      return false;

    bool interesting = false;

    // only care power of two
    if (!isPowerOfTwoOrZero(bits))
      return false;

    for (auto it = visited_.begin();
         it != visited_.end();
         it++)
    {
      ADDRINT prev_h = *it;

      // Calculate hash(prev_h || h)
      XXH32_state_t state;
      XXH32_reset(&state, 0);
      XXH32_update(&state, &prev_h, sizeof(prev_h));
      XXH32_update(&state, &h, sizeof(h));

      ADDRINT hash = XXH32_digest(&state) % (kMapSize * CHAR_BIT);
      ADDRINT idx = hash / CHAR_BIT;
      ADDRINT mask = 1 << (hash % CHAR_BIT);

      if ((context_map_[idx] & mask) == 0)
      {
        context_map_[idx] |= mask;
        interesting = true;
      }
    }

    if (bits == 0)
      visited_.insert(h);

    return interesting;
  }

  void AflTraceMap::commit()
  {
    if (!path_.empty())
    {
      fstream fs;
      fs.open(path_, ios::binary | ios::in | ios::out);
      if (fs.fail())
      {
        LOG_DEBUG("cannot read a file, so use a default trace map\n");

        ofstream fs;
        fs.open(path_, ios::binary);
        if (fs.fail())
          LOG_FATAL("Unable to open a bitmap to commit");
        //fs.write((char *)trace_map_, kMapSize);
        fs.write((char *)context_map_, kMapSize);
        fs.write(reinterpret_cast<char *>(aflcov_map_), 2*kMapSize);
        fs.close();
      }
      else
      {
        //fs.write((char *)trace_map_, kMapSize);
        fs.write((char *)context_map_, kMapSize);
        //fs.write((char *)aflcov_map_, kMapSize);
        fs.close();
      }
    }
  }
  void AflTraceMap::commit_unsolved(ADDRINT afl_inv_idx)
  {
    if (!path_.empty())
    {
      fstream fs;
      fs.open(path_, ios::binary | ios::in | ios::out);
      if (fs.fail())
      {
        LOG_DEBUG("cannot read a file, so use a default trace map\n");

        ofstream fs;
        fs.open(path_, ios::binary);
        if (fs.fail())
          LOG_FATAL("Unable to open a bitmap to commit");
      }
      fs.seekp(1 * kMapSize + 2 * afl_inv_idx, ios::beg);
      fs.write(reinterpret_cast<char *>(&aflcov_map_[afl_inv_idx]), 2);
      fs.close();
    }
  }
  AflTraceMap::AflTraceMap(const std::string path)
      : path_(path),
        prev_loc_(0),
        visited_()
  {
    allocMap();
    if (path.empty())
      setDefault();
    else
      import(path);
  }

  bool AflTraceMap::isInterestingBranch(ADDRINT pc, ADDRINT target, bool taken, ADDRINT &afl_inv_idx, ADDRINT &afl_idx, ADDRINT prev_jcc_)
  {
    ADDRINT h = hashPc(pc, taken);
    ADDRINT inv_h = hashPc(pc, !taken);

    ADDRINT idx = getIndex(h);
    // ADDRINT inv_idx = getIndex(inv_h);

    afl_idx = ((prev_jcc_ >> 1) ^ h) % kMapSize;;
    afl_inv_idx = ((prev_jcc_ >> 1) ^ inv_h) % kMapSize;

    
    bool new_context = isInterestingContext(h, virgin_map_[idx]);
    bool ret = true;

    virgin_map_[idx]++;
    
    if ((new_context && aflcov_map_[afl_inv_idx] < 65534) || aflcov_map_[afl_inv_idx] <= avg)
    {
      ret = true;
      commit();
    }
    else
      ret = false;
 
    prev_loc_ = h;
    return ret;

  }

  void AflTraceMap::isUnsolvedBranch(ADDRINT &afl_inv_idx)
  {
    aflcov_map_[afl_inv_idx] |= 65534;
    // memset(unsolved_, aflcov_map_[afl_inv_idx], 2);
    commit_unsolved(afl_inv_idx);
  }

} // namespace qsym


