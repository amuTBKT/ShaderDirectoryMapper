// Copyright 2024 Amit Kumar Mehar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ShaderDirectoryMapper.generated.h"

UCLASS(config = Engine, defaultconfig, DisplayName = "Shader Directory Mapper")
class UShaderDirectoryMapperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	virtual void PostInitProperties() override
	{
		Super::PostInitProperties();

#if WITH_EDITOR
		if (IsTemplate())
		{
			ImportConsoleVariableValues();
		}
#endif
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		const FProperty* MemberPropertyThatChanged = PropertyChangedEvent.MemberProperty;
		const FName MemberPropertyName = MemberPropertyThatChanged ? MemberPropertyThatChanged->GetFName() : NAME_None;

		if (PropertyChangedEvent.Property)
		{
			ExportValuesToConsoleVariables(PropertyChangedEvent.Property);
		}
	}
#endif

public:
	// If initiailized, only plugins specified in this list will be registered.
	UPROPERTY(config, EditAnywhere, Category = "Plugin")
	TSet<FString> AllowedPlugins;

	// Plugins to never register.
	UPROPERTY(config, EditAnywhere, Category = "Plugin")
	TSet<FString> DisallowedPlugins;
};
