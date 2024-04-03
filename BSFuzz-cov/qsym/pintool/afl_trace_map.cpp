#include "afl_trace_map.h"

namespace qsym
{

  const int kMapSize = 65536;
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
    //context_map_ = (UINT8 *)safeMalloc(kMapSize);
    aflcov_map_ = (UINT16 *)safeMalloc(2*kMapSize);
    memset(virgin_map_, 0, kMapSize);
  }

  void AflTraceMap::setDefault()
  {
    //memset(trace_map_, 0, kMapSize);
    //memset(context_map_, 0, kMapSize);
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
    ifs.seekg(1 * kMapSize, ios::beg);
    ifs.read(reinterpret_cast<char *>(aflcov_map_), 2*kMapSize);
    if (!ifs)
      setDefault();
    ifs.close();
  }

  ADDRINT AflTraceMap::getIndex(ADDRINT h)
  {
    return ((prev_jcc_ >> 1) ^ h) % kMapSize;
    // return (h) % kMapSize;
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
      }
      fs.seekp(1 * kMapSize, ios::beg);
      fs.write(reinterpret_cast<char *>(aflcov_map_), 2*kMapSize);
      fs.close();
    }
  }

  AflTraceMap::AflTraceMap(const std::string path)
      : path_(path),
        prev_jcc_(0),
        visited_()
  {
    allocMap();
    if (path.empty())
      setDefault();
    else
      import(path);
  }

  bool AflTraceMap::isInterestingBranch(ADDRINT pc, ADDRINT target, bool taken)
  {
    ADDRINT h = hashPc(pc, taken);
    ADDRINT idx = getIndex(h);
    // bool new_context = isInterestingContext(h, virgin_map_[idx]);
    bool ret = true;


    if(virgin_map_[idx] == 0){
      virgin_map_[idx] = 1;
      if(aflcov_map_[idx] < 65535){
        aflcov_map_[idx]++; 
        commit();
      }
    }

    // debug
    /*
    if(aflcov_map_[idx] == 1){
	    ofstream fs;
	    int pos = path_.find("bitmap");
	    string bitmap_path = path_;
	    bitmap_path.erase(pos, 6);
	    fs.open(bitmap_path + std::string("../cov_branch.csv"), std::ios::out | std::ios::app);

	    fs << hexstr(pc) << ','
	       << (taken
		       ? "true"
		       : "false")
	       << ','
	       << hexstr(idx) << ','
	       << to_string(aflcov_map_[idx])
	       << std::endl;
	    fs.close();
    }
    */

   
    
    prev_jcc_ = h;
    return ret;

    /*
    virgin_map_[idx]++;

    if ((virgin_map_[idx] | aflcov_map_[idx]) != aflcov_map_[idx])
    {
      aflcov_map_[idx] = virgin_map_[idx] | aflcov_map_[idx];
      commit();
    }
    prev_loc_ = h;
    return ret;
    */
  }

} // namespace qsym


