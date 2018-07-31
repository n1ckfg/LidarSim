// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDescription.h"

class AActor;

/// A view over an actor and its properties.
class FActorView
{
public:

  using IdType = uint32;

  FActorView() = default;
  FActorView(const FActorView &) = default;

  bool IsValid() const
  {
    return (TheActor != nullptr) && Description.IsValid();
  }

  IdType GetActorId() const
  {
    return Id;
  }

  AActor *GetActor()
  {
    return TheActor;
  }

  const AActor *GetActor() const
  {
    return TheActor;
  }

  const FActorDescription *GetActorDescription() const
  {
    return Description.Get();
  }

private:

  friend class FActorRegistry;

  FActorView(IdType ActorId, AActor &Actor, FActorDescription Description)
    : Id(ActorId),
      TheActor(&Actor),
      Description(MakeShared<FActorDescription>(std::move(Description))) {
    check(Id != 0u);
  }

  IdType Id = 0u;

  AActor *TheActor = nullptr;

  TSharedPtr<const FActorDescription> Description = nullptr;
};
