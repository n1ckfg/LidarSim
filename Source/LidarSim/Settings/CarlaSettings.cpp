// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CarlaSettings.h"
#include "DynamicWeather.h"
#include "Settings/CameraDescription.h"
#include "Settings/LidarDescription.h"
#include "Util/IniFile.h"
#include "Package.h"
#include "CommandLine.h"
#include "UnrealMathUtility.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInstance.h"

// INI file sections.
#define S_CARLA_SERVER                 TEXT("CARLA/Server")
#define S_CARLA_LEVELSETTINGS          TEXT("CARLA/LevelSettings")
#define S_CARLA_SENSOR                 TEXT("CARLA/Sensor")
#define S_CARLA_QUALITYSETTINGS        TEXT("CARLA/QualitySettings")

// =============================================================================
// -- Static variables & constants ---------------------------------------------
// =============================================================================

const FName UCarlaSettings::CARLA_ROAD_TAG = FName("CARLA_ROAD");
const FName UCarlaSettings::CARLA_SKY_TAG = FName("CARLA_SKY");

// =============================================================================
// -- Static methods -----------------------------------------------------------
// =============================================================================

template <typename T>
static void ForEachSectionInName(const FString &SensorName, T &&Callback)
{
  TArray<FString> SubSections;
  SensorName.ParseIntoArray(SubSections, TEXT("/"), true);
  check(SubSections.Num() > 0);
  FString Section = S_CARLA_SENSOR;
  Callback(Section);
  for (FString &SubSection : SubSections) {
    Section += TEXT("/");
    Section += SubSection;
    Callback(Section);
  }
}

static FString GetSensorType(
    const FIniFile &ConfigFile,
    const FString &SensorName)
{
  FString SensorType;
  ForEachSectionInName(SensorName, [&](const auto &Section){
    ConfigFile.GetString(*Section, TEXT("SensorType"), SensorType);
  });
  return SensorType;
}

static void LoadSensorFromConfig(
    const FIniFile &ConfigFile,
    USensorDescription &Sensor)
{
  ForEachSectionInName(Sensor.Name, [&](const auto &Section){
    Sensor.Load(ConfigFile, Section);
  });
}

template <typename T>
static T *MakeSensor(UObject *Parent, const FString &Name, const FString &Type)
{
  auto *Sensor = NewObject<T>(Parent);
  Sensor->Name = Name;
  Sensor->Type = Type;
  return Sensor;
}

static USensorDescription *MakeSensor(
    const FIniFile &ConfigFile,
    UObject *Parent,
    const FString &SensorName)
{
  const auto SensorType = GetSensorType(ConfigFile, SensorName);
  if (SensorType == TEXT("CAMERA")) {
    return MakeSensor<UCameraDescription>(Parent, SensorName, SensorType);
  } else if (SensorType == TEXT("LIDAR_RAY_CAST")) {
    return MakeSensor<ULidarDescription>(Parent, SensorName, SensorType);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Invalid sensor type '%s'"), *SensorType);
    return nullptr;
  }
}

static void LoadSettingsFromConfig(
    const FIniFile &ConfigFile,
    UCarlaSettings &Settings,
    const bool bLoadCarlaServerSection)
{
  // CarlaServer.
  if (bLoadCarlaServerSection) {
    ConfigFile.GetBool(S_CARLA_SERVER, TEXT("UseNetworking"), Settings.bUseNetworking);
    ConfigFile.GetInt(S_CARLA_SERVER, TEXT("WorldPort"), Settings.WorldPort);
    ConfigFile.GetInt(S_CARLA_SERVER, TEXT("ServerTimeOut"), Settings.ServerTimeOut);
  }
  ConfigFile.GetBool(S_CARLA_SERVER, TEXT("SynchronousMode"), Settings.bSynchronousMode);
  ConfigFile.GetBool(S_CARLA_SERVER, TEXT("SendNonPlayerAgentsInfo"), Settings.bSendNonPlayerAgentsInfo);
  // LevelSettings.
  ConfigFile.GetString(S_CARLA_LEVELSETTINGS, TEXT("PlayerVehicle"), Settings.PlayerVehicle);
  ConfigFile.GetInt(S_CARLA_LEVELSETTINGS, TEXT("NumberOfVehicles"), Settings.NumberOfVehicles);
  ConfigFile.GetInt(S_CARLA_LEVELSETTINGS, TEXT("NumberOfPedestrians"), Settings.NumberOfPedestrians);
  ConfigFile.GetInt(S_CARLA_LEVELSETTINGS, TEXT("WeatherId"), Settings.WeatherId);
  ConfigFile.GetInt(S_CARLA_LEVELSETTINGS, TEXT("SeedVehicles"), Settings.SeedVehicles);
  ConfigFile.GetInt(S_CARLA_LEVELSETTINGS, TEXT("SeedPedestrians"), Settings.SeedPedestrians);
  ConfigFile.GetBool(S_CARLA_LEVELSETTINGS, TEXT("DisableTwoWheeledVehicles"), Settings.bDisableTwoWheeledVehicles);

  // QualitySettings.
  FString sQualityLevel;
  ConfigFile.GetString(S_CARLA_QUALITYSETTINGS, TEXT("QualityLevel"), sQualityLevel);
  Settings.SetQualitySettingsLevel(UQualitySettings::FromString(sQualityLevel));

  // Sensors.
  FString Sensors;
  ConfigFile.GetString(S_CARLA_SENSOR, TEXT("Sensors"), Sensors);
  TArray<FString> SensorNames;
  Sensors.ParseIntoArray(SensorNames, TEXT(","), true);
  for (const FString &Name : SensorNames) {
    auto *Sensor = MakeSensor(ConfigFile, &Settings, Name);
    if (Sensor != nullptr) {
      LoadSensorFromConfig(ConfigFile, *Sensor);
      Sensor->Validate();
      Settings.bSemanticSegmentationEnabled |= Sensor->RequiresSemanticSegmentation();
      Settings.SensorDescriptions.Add(Name, Sensor);
    }
  }
}

static bool GetSettingsFilePathFromCommandLine(FString &Value)
{
  if (FParse::Value(FCommandLine::Get(), TEXT("-carla-settings="), Value)) {
    if (FPaths::IsRelative(Value)) {
      Value = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir(), Value);
      return true;
    }
  }
  return false;
}

// =============================================================================
// -- UCarlaSettings -----------------------------------------------------------
// =============================================================================

EQualitySettingsLevel UQualitySettings::FromString(const FString& SQualitySettingsLevel)
{
	if(SQualitySettingsLevel.Equals("Low")) return EQualitySettingsLevel::Low;
	if(SQualitySettingsLevel.Equals("Medium")) return EQualitySettingsLevel::Medium;
	if(SQualitySettingsLevel.Equals("High")) return EQualitySettingsLevel::High;
	if(SQualitySettingsLevel.Equals("Epic")) return EQualitySettingsLevel::Epic;

	return EQualitySettingsLevel::None;
}

FString UQualitySettings::ToString(EQualitySettingsLevel QualitySettingsLevel)
{
  const UEnum* ptr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EQualitySettingsLevel"), true);
  if(!ptr)
    return FString("Invalid");
  return ptr->GetNameStringByIndex(static_cast<int32>(QualitySettingsLevel));
}

void UCarlaSettings::SetQualitySettingsLevel(EQualitySettingsLevel newQualityLevel)
{
	QualitySettingsLevel = newQualityLevel;
}

void UCarlaSettings::LoadSettings()
{
  CurrentFileName = TEXT("");
  // Load settings from project Config folder if present.
  LoadSettingsFromFile(FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("CarlaSettings.ini")), false);
  // Load settings given by command-line arg if provided.
  {
    FString FilePath;
    if (GetSettingsFilePathFromCommandLine(FilePath)) {
      LoadSettingsFromFile(FilePath, true);
    }
  }
  // Override settings from command-line.
  {
    if (FParse::Param(FCommandLine::Get(), TEXT("carla-server"))) {
      bUseNetworking = true;
    }
    uint32 Value;
    if (FParse::Value(FCommandLine::Get(), TEXT("-world-port="), Value) ||
        FParse::Value(FCommandLine::Get(), TEXT("-carla-port="), Value) ||
        FParse::Value(FCommandLine::Get(), TEXT("-carla-world-port="), Value)) {
      WorldPort = Value;
      bUseNetworking = true;
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("carla-no-networking"))) {
      bUseNetworking = false;
    }
  }
}

void UCarlaSettings::LoadSettingsFromString(const FString &INIFileContents)
{
  UE_LOG(LogCarla, Log, TEXT("Loading CARLA settings from string"));
  ResetSensorDescriptions();
  FIniFile ConfigFile;
  ConfigFile.ProcessInputFileContents(INIFileContents);
  constexpr bool bLoadCarlaServerSection = false;
  LoadSettingsFromConfig(ConfigFile, *this, bLoadCarlaServerSection);
  CurrentFileName = TEXT("<string-provided-by-client>");
}

void UCarlaSettings::LoadWeatherDescriptions()
{
  WeatherDescriptions.Empty();
  ADynamicWeather::LoadWeatherDescriptionsFromFile(MapName, WeatherDescriptions);
  check(WeatherDescriptions.Num() > 0);
}

void UCarlaSettings::ValidateWeatherId()
{
  if (WeatherId >= WeatherDescriptions.Num()) {
    UE_LOG(LogCarla, Error, TEXT("Provided weather id %d cannot be found"), WeatherId);
    WeatherId = -1;
  }
}

void UCarlaSettings::LogSettings() const
{
  auto EnabledDisabled = [](bool bValue) { return (bValue ? TEXT("Enabled") : TEXT("Disabled")); };
  UE_LOG(LogCarla, Log, TEXT("== CARLA Settings =============================================================="));
  UE_LOG(LogCarla, Log, TEXT("Last settings file loaded: %s"), *CurrentFileName);
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_SERVER);
  UE_LOG(LogCarla, Log, TEXT("Networking = %s"), EnabledDisabled(bUseNetworking));
  UE_LOG(LogCarla, Log, TEXT("World Port = %d"), WorldPort);
  UE_LOG(LogCarla, Log, TEXT("Server Time-out = %d ms"), ServerTimeOut);
  UE_LOG(LogCarla, Log, TEXT("Synchronous Mode = %s"), EnabledDisabled(bSynchronousMode));
  UE_LOG(LogCarla, Log, TEXT("Send Non-Player Agents Info = %s"), EnabledDisabled(bSendNonPlayerAgentsInfo));
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_LEVELSETTINGS);
  UE_LOG(LogCarla, Log, TEXT("Player Vehicle        = %s"), (PlayerVehicle.IsEmpty() ? TEXT("Default") : *PlayerVehicle));
  UE_LOG(LogCarla, Log, TEXT("Number Of Vehicles    = %d"), NumberOfVehicles);
  UE_LOG(LogCarla, Log, TEXT("Number Of Pedestrians = %d"), NumberOfPedestrians);
  UE_LOG(LogCarla, Log, TEXT("Weather Id = %d"), WeatherId);
  UE_LOG(LogCarla, Log, TEXT("Seed Vehicle Spawner = %d"), SeedVehicles);
  UE_LOG(LogCarla, Log, TEXT("Seed Pedestrian Spawner = %d"), SeedPedestrians);
  UE_LOG(LogCarla, Log, TEXT("Two-Wheeled Vehicles = %s"), EnabledDisabled(!bDisableTwoWheeledVehicles));
  UE_LOG(LogCarla, Log, TEXT("Found %d available weather settings."), WeatherDescriptions.Num());
  for (auto i = 0; i < WeatherDescriptions.Num(); ++i)
  {
    UE_LOG(LogCarla, Log, TEXT("  * %d - %s"), i, *WeatherDescriptions[i].Name);
  }
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_QUALITYSETTINGS);
  UE_LOG(LogCarla, Log, TEXT("Quality Settings = %s"), *UQualitySettings::ToString(QualitySettingsLevel));

  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_SENSOR);
  UE_LOG(LogCarla, Log, TEXT("Added %d sensors."), SensorDescriptions.Num());
  UE_LOG(LogCarla, Log, TEXT("Semantic Segmentation = %s"), EnabledDisabled(bSemanticSegmentationEnabled));
  for (auto &&Sensor : SensorDescriptions)
  {
    check(Sensor.Value != nullptr);
    Sensor.Value->Log();
  }
  UE_LOG(LogCarla, Log, TEXT("================================================================================"));
}

#undef S_CARLA_SERVER
#undef S_CARLA_LEVELSETTINGS
#undef S_CARLA_SENSOR

void UCarlaSettings::GetActiveWeatherDescription(
    bool &bWeatherWasChanged,
    FWeatherDescription &WeatherDescription) const
{
  auto WeatherPtr = GetActiveWeatherDescription();
  if (WeatherPtr != nullptr) {
    WeatherDescription = *WeatherPtr;
    bWeatherWasChanged = true;
  } else {
    bWeatherWasChanged = false;
  }
}

const FWeatherDescription &UCarlaSettings::GetWeatherDescriptionByIndex(int32 Index)
{
  check(WeatherDescriptions.Num() > 0);
  FMath::Clamp(Index, 0, WeatherDescriptions.Num());
  return WeatherDescriptions[Index];
}

void UCarlaSettings::ResetSensorDescriptions()
{
  SensorDescriptions.Empty();
  bSemanticSegmentationEnabled = false;
}

void UCarlaSettings::LoadSettingsFromFile(const FString &FilePath, const bool bLogOnFailure)
{
  if (FPaths::FileExists(FilePath)) {
    UE_LOG(LogCarla, Log, TEXT("Loading CARLA settings from \"%s\""), *FilePath);
    ResetSensorDescriptions();
    const FIniFile ConfigFile(FilePath);
    constexpr bool bLoadCarlaServerSection = true;
    LoadSettingsFromConfig(ConfigFile, *this, bLoadCarlaServerSection);
    CurrentFileName = FilePath;
  } else if (bLogOnFailure) {
    UE_LOG(LogCarla, Error, TEXT("Unable to find settings file \"%s\""), *FilePath);
  }
}
