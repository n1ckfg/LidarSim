// Fill out your copyright notice in the Description page of Project Settings.

#include "LidarDescription.h"

ULidarDescription::ULidarDescription(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{ 
  //
}

void ULidarDescription::Load(const FIniFile &Config, const FString &Section)
{
  Super::Load(Config, Section);
  Config.GetInt(*Section, TEXT("Channels"), Channels);
  Config.GetFloat(*Section, TEXT("Range"), Range, 1e2);
  Config.GetInt(*Section, TEXT("PointsPerSecond"), PointsPerSecond);
  Config.GetFloat(*Section, TEXT("RotationFrequency"), RotationFrequency);
  Config.GetFloat(*Section, TEXT("UpperFovLimit"), UpperFovLimit);
  Config.GetFloat(*Section, TEXT("LowerFovLimit"), LowerFovLimit);
  Config.GetBool(*Section, TEXT("ShowDebugPoints"), ShowDebugPoints);
}

void ULidarDescription::Validate()
{
  Channels = (Channels == 0u ? 32u : Channels);
  FMath::Clamp(Range, 0.10f, 50000.0f);
  PointsPerSecond = (PointsPerSecond == 0u ? 56000u : PointsPerSecond);
  FMath::Clamp(RotationFrequency, 0.001f, 50000.0f);
  FMath::Clamp(UpperFovLimit, -89.9f, 90.0f);
  FMath::Clamp(LowerFovLimit, -90.0f, UpperFovLimit);
}

void ULidarDescription::Log() const
{
  Super::Log();
  std::cout << "Channels = %d" << Channels << "\n";
  std::cout << "Range = %f" << Range << "\n";
  std::cout << "PointsPerSecond = %d" << PointsPerSecond << "\n";
  std::cout << "RotationFrequency = %f" << RotationFrequency << "\n";
  std::cout << "UpperFovLimit = %f" << UpperFovLimit << "\n";
  std::cout << "LowerFovLimit = %f" << LowerFovLimit << "\n";
}
