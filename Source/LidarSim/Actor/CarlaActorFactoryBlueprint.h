// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "LidarSim/Actor/ActorSpawnResult.h"
#include "LidarSim/Actor/CarlaActorFactory.h"

#include "GameFramework/Actor.h"

#include "CarlaActorFactoryBlueprint.generated.h"

/// Base class for Blueprints implementing ACarlaActorFactory interface.
///
/// Blueprints deriving from this class are expected to override
/// GetDefinitions and SpawnActor functions.
UCLASS(Abstract, BlueprintType, Blueprintable)
class LIDARSIM_API ACarlaActorFactoryBlueprint : public ACarlaActorFactory
{
  GENERATED_BODY()

public:

  TArray<FActorDefinition> GetDefinitions() final
  {
    return GenerateDefinitions();
  }

  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final
  {
    FActorSpawnResult Result;
    SpawnActor(SpawnAtTransform, ActorDescription, Result);
    return Result;
  }

protected:

  UFUNCTION(BlueprintImplementableEvent)
  TArray<FActorDefinition> GenerateDefinitions();

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription,
      FActorSpawnResult &SpawnResult);
};
