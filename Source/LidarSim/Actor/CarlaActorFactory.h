// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "LidarSim/Actor/ActorDefinition.h"
#include "LidarSim/Actor/ActorDescription.h"
#include "LidarSim/Actor/ActorSpawnResult.h"

#include "Containers/Array.h"
#include "GameFramework/Actor.h"

#include "CarlaActorFactory.generated.h"

/// Base class for Carla actor factories.
UCLASS(Abstract)
class LIDARSIM_API ACarlaActorFactory : public AActor
{
  GENERATED_BODY()

public:

  ACarlaActorFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
  {
    PrimaryActorTick.bCanEverTick = false;
  }

  /// Retrieve the list of actor definitions that this class is able to spawn.
  virtual TArray<FActorDefinition> GetDefinitions() {
    unimplemented();
    return {};
  }

  /// Spawn an actor based on @a ActorDescription and @a Transform.
  ///
  /// @pre ActorDescription is expected to be derived from one of the
  /// definitions retrieved with GetDefinitions.
  virtual FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) {
    unimplemented();
    return {};
  }
};
