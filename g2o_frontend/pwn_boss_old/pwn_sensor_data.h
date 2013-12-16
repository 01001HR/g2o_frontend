#ifndef _PWN_SENSOR_DATA_H_
#define _PWN_SENSOR_DATA_H_

#include "g2o_frontend/pwn_core/frame.h"
#include "g2o_frontend/pwn_core/pinholepointprojector.h"
#include "g2o_frontend/pwn_core/depthimageconverter.h"
#include "g2o_frontend/pwn_core/aligner.h"
#include "g2o_frontend/boss/serializer.h"
#include "g2o_frontend/boss/deserializer.h"

#include "g2o_frontend/boss_map/reference_frame.h"
#include "g2o_frontend/boss_map/reference_frame_relation.h"
#include "g2o_frontend/boss_map/image_sensor.h"

namespace pwn_boss {
  using namespace boss;
  using namespace pwn;
  using namespace boss_map;


  class PWNSensorData : public boss_map::BaseSensorData {
  public:
    PWNSensorData(int id=-1, IdContext* context = 0);
    virtual void serialize(ObjectData& data, IdContext& context);
    virtual void deserialize(ObjectData& data, IdContext& context);
    inline pwn::FrameBLOBReference& blob() { return _blob; }
    inline const pwn::FrameBLOBReference& blob() const { return _blob; }
  protected:
    pwn::FrameBLOBReference _blob;
  };

  class PWNDepthConverterSensor: public BaseSensor{
  public:
    PWNDepthConverterSensor(DepthImageConverter* converter_=0, int id=-1, IdContext* context = 0);
    virtual void serialize(ObjectData& data, IdContext& context);
    virtual void deserialize(ObjectData& data, IdContext& context);
  protected:
    DepthImageConverter* _converter;
  };

  class PWNDepthConvertedData: public PWNSensorData{
  public:
    PWNDepthConvertedData(PWNDepthConverterSensor* sensor=0,  int id=-1, IdContext* context = 0);
    virtual void serialize(ObjectData& data, IdContext& context);
    virtual void deserialize(ObjectData& data, IdContext& context);
    virtual BaseSensor* baseSensor();
    virtual const BaseSensor* baseSensor() const;

    ImageData* _image;
    int _scale;
    PWNDepthConverterSensor* _sensor;
  };

}

#endif
