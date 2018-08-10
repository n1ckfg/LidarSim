// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"

#include <unreal_ros_node_wrapper/unreal_ros_node_wrapper.hpp>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <exception>
#include <cmath>

class FLidarSimModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};