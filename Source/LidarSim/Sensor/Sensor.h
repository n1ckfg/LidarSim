// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "LidarSim.h"

#include "Sensor/SensorDataSink.h"
#include "Settings/SensorDescription.h"

#include "GameFramework/Actor.h"

#include "Sensor.generated.h"

/// Base class for sensors.
UCLASS(Abstract, hidecategories=(Collision, Attachment, Actor))
class LIDARSIM_API ASensor : public AActor
{
  GENERATED_BODY()

public:

  ASensor(const FObjectInitializer& ObjectInitializer);

  uint32 GetId() const
  {
    return Id;
  }

  void AttachToActor(AActor *Actor);

  void SetSensorDataSink(TSharedPtr<ISensorDataSink> InSensorDataSink)
  {
    SensorDataSink = InSensorDataSink;
  }

protected:

  void Set(const USensorDescription &SensorDescription)
  {
    Id = SensorDescription.GetId();
  }

  void WriteSensorData(const FSensorDataView &SensorData) const
  {
    if (SensorDataSink.IsValid()) {
      SensorDataSink->Write(SensorData);
    } else {
      //std::cout << "Sensor %d has no data sink." << Id << "\n";
    }
  }

private:

  UPROPERTY(VisibleAnywhere)
  uint32 Id;

  TSharedPtr<ISensorDataSink> SensorDataSink = nullptr;
};
