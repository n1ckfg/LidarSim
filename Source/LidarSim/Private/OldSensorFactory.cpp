// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "OldSensorFactory.h"

#include "Sensor/Lidar.h"
#include "Sensor/SceneCaptureCamera.h"
#include "Settings/CameraDescription.h"
#include "Settings/LidarDescription.h"

template <typename T, typename D>
static T *SpawnSensor(const D &Description, UWorld &World)
{
  FActorSpawnParameters Params;
  Params.Name = FName(*Description.Name);
  return World.SpawnActor<T>(Description.Position, Description.Rotation, Params);
}

ASensor *FSensorFactory::Make(
    const USensorDescription &Description,
    UWorld &World)
{
  FSensorFactory Visitor(World);
  Description.AcceptVisitor(Visitor);
  check(Visitor.Sensor != nullptr);
  return Visitor.Sensor;
}

FSensorFactory::FSensorFactory(UWorld &World) : World(World) {}

void FSensorFactory::Visit(const UCameraDescription &Description)
{
  auto Camera = SpawnSensor<ASceneCaptureCamera>(Description, World);
  Camera->Set(Description);
  UE_LOG(
      LogCarla,
      Log,
      TEXT("Created Capture Camera %d with postprocess \"%s\""),
      Camera->GetId(),
      *PostProcessEffect::ToString(Camera->GetPostProcessEffect()));
  Sensor = Camera;
}

void FSensorFactory::Visit(const ULidarDescription &Description)
{
  auto Lidar = SpawnSensor<ALidar>(Description, World);
  Lidar->Set(Description);
  UE_LOG(
      LogCarla,
      Log,
      TEXT("Created Lidar %d"),
      Lidar->GetId());
  Sensor = Lidar;
}
