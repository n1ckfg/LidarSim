// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SensorDescription.h"

void USensorDescription::AcceptVisitor(ISensorDescriptionVisitor &Visitor) const
{
  unimplemented();
}

void USensorDescription::Load(const FIniFile &Config, const FString &Section)
{
  constexpr float TO_CENTIMETERS = 1e2;
  Config.GetFloat(*Section, TEXT("PositionX"), Position.X, TO_CENTIMETERS);
  Config.GetFloat(*Section, TEXT("PositionY"), Position.Y, TO_CENTIMETERS);
  Config.GetFloat(*Section, TEXT("PositionZ"), Position.Z, TO_CENTIMETERS);
  Config.GetFloat(*Section, TEXT("RotationPitch"), Rotation.Pitch);
  Config.GetFloat(*Section, TEXT("RotationYaw"), Rotation.Yaw);
  Config.GetFloat(*Section, TEXT("RotationRoll"), Rotation.Roll);
}

void USensorDescription::Log() const
{
  std::cout << "[%s/%s] LidarSim/Sensor" << *Name << "\n";
  std::cout << "Id = %d" << GetId() << "\n";
  std::cout << "Type = %s" << *Type << "\n";
  std::cout << "Position = (%s)" << *Position.ToString() << "\n";
  std::cout << "Rotation = (%s)" << *Rotation.ToString() << "\n";
}
