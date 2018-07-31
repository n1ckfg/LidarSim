// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "TrafficLightBase.h"

#include "Vehicle/CarlaWheeledVehicle.h"
#include "Vehicle/WheeledVehicleAIController.h"

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

static bool IsValid(const ACarlaWheeledVehicle *Vehicle)
{
  return ((Vehicle != nullptr) && !Vehicle->IsPendingKill());
}

static ETrafficSignState ToTrafficSignState(ETrafficLightState State) {
  switch (State) {
    case ETrafficLightState::Green:
      return ETrafficSignState::TrafficLightGreen;
    case ETrafficLightState::Yellow:
      return ETrafficSignState::TrafficLightYellow;
    default:
    case ETrafficLightState::Red:
      return ETrafficSignState::TrafficLightRed;
  }
}

// =============================================================================
// -- ATrafficLightBase --------------------------------------------------------
// =============================================================================

ATrafficLightBase::ATrafficLightBase(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATrafficLightBase::OnConstruction(const FTransform &Transform)
{
  Super::OnConstruction(Transform);
  SetTrafficLightState(State);
}

#if WITH_EDITOR
void ATrafficLightBase::PostEditChangeProperty(FPropertyChangedEvent &Event)
{
  Super::PostEditChangeProperty(Event);

  const FName PropertyName = (Event.Property != nullptr ? Event.Property->GetFName() : NAME_None);
  if (PropertyName == GET_MEMBER_NAME_CHECKED(ATrafficLightBase, State)) {
    SetTrafficLightState(State);
  }
}
#endif // WITH_EDITOR

void ATrafficLightBase::SetTrafficLightState(const ETrafficLightState InState)
{
  State = InState;
  SetTrafficSignState(ToTrafficSignState(State));
  for (auto Controller : Vehicles) {
    if (Controller != nullptr) {
      Controller->SetTrafficLightState(State);
    }
  }
  if (State == ETrafficLightState::Green) {
    Vehicles.Empty();
  }
  OnTrafficLightStateChanged(State);
}

void ATrafficLightBase::SwitchTrafficLightState()
{
  switch (State) {
    case ETrafficLightState::Red:
      SetTrafficLightState(ETrafficLightState::Green);
      break;
    case ETrafficLightState::Yellow:
      SetTrafficLightState(ETrafficLightState::Red);
      break;
    case ETrafficLightState::Green:
      SetTrafficLightState(ETrafficLightState::Yellow);
      break;
    default:
      UE_LOG(LogCarla, Error, TEXT("Invalid traffic light state!"));
      SetTrafficLightState(ETrafficLightState::Red);
      break;
  }
}

void ATrafficLightBase::NotifyWheeledVehicle(ACarlaWheeledVehicle *Vehicle)
{
  if (IsValid(Vehicle)) {
    auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (Controller != nullptr) {
      Controller->SetTrafficLightState(State);
      if (State != ETrafficLightState::Green) {
        Vehicles.Add(Controller);
      }
    }
  }
}
