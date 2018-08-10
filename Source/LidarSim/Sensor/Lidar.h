// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LidarSim.h"

#include "Sensor/Sensor.h"
#include "Sensor/LidarMeasurement.h"
#include "Settings/LidarDescription.h"

#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "StaticMeshResources.h"

#include "Lidar.generated.h"

/// A ray-trace based Lidar sensor.
UCLASS()
class LIDARSIM_API ALidar : public ASensor
{
  GENERATED_BODY()

public:

  ALidar(const FObjectInitializer &ObjectInitializer);

  void Set(const ULidarDescription &LidarDescription);

  void BeginPlay();
  
  bool debugging = false;
  bool drawRays = true;
  bool writeTestFile = true;
  int pointsMax = 200000;
  int pointsCounter = 0;
  std::ofstream pointsFile;

protected:

  virtual void Tick(float DeltaTime) override;

private:

  /// Creates a Laser for each channel.
  void CreateLasers();

  /// Updates LidarMeasurement with the points read in DeltaTime.
  void ReadPoints(float DeltaTime);

  /// Shoot a laser ray-trace, return whether the laser hit something.
  bool ShootLaser(uint32 Channel, float HorizontalAngle, FVector &Point) const;

  UPROPERTY(Category = "Lidar", VisibleAnywhere)
  const ULidarDescription *Description = nullptr;

  TArray<float> LaserAngles;

  FLidarMeasurement LidarMeasurement;

};
