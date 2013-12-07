#pragma once

#include "g2o_frontend/boss_map/bframe.h"
#include "g2o_frontend/pwn_core/frame.h"
#include "g2o_frontend/boss_map/boss_map_manager.h"
#include "g2o_frontend/pwn_core/pinholepointprojector.h"
#include "g2o_frontend/pwn_core/depthimageconverter.h"
#include "cache.h"

namespace pwn_tracker {
  using namespace cache_ns;
  using namespace pwn;
  using namespace boss_map;

  class PwnCache;
  class PwnTrackerFrame;

  class PwnCacheEntry: public CacheEntry<PwnTrackerFrame, pwn::Frame>{
  public:
    PwnCacheEntry(PwnCache* cache, PwnTrackerFrame* k, pwn::Frame* d=0);
  protected:
    virtual DataType* fetch(KeyType* k);
    PwnCache* _pwnCache;
  };

  class PwnCache: public Cache<PwnCacheEntry>{
  public:
    PwnCache(DepthImageConverter* converter_, int scale_, int minSlots_, int _maxSlots_);

    inline DepthImageConverter* converter() {return _converter;}
    inline void setConverter(DepthImageConverter* converter_) {_converter = converter_;}

    inline int scale() const {return _scale;}
    inline void setScale(int scale_)  {_scale=scale_;}
    pwn::Frame* loadFrame(PwnTrackerFrame* trackerFrame);

  protected:
    virtual Cache<PwnCacheEntry>::EntryType* makeEntry(KeyType* k, DataType* d);
    DepthImageConverter* _converter;
    int _scale;
  };


  class PwnCacheHandler: public MapManagerActionHandler {
  public:
    PwnCacheHandler(MapManager* _manager, PwnCache* cache);
    inline PwnCache* cache() {return _cache;}
    virtual ~PwnCacheHandler();
    void init();

    virtual void nodeAdded(MapNode* n);
    virtual void nodeRemoved(MapNode* n);
    virtual void relationAdded(MapNodeRelation* _r);
    virtual void relationRemoved(MapNodeRelation* r);
  protected:
    PwnCache* _cache;
  };

  
}
