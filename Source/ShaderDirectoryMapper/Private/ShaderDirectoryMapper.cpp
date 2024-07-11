// Copyright 2024 Amit Kumar Mehar. All Rights Reserved.

#include "ShaderDirectoryMapper.h"

#include "Misc/App.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "Interfaces/IPluginManager.h"

class FShaderDirectoryMapperModule : public IModuleInterface
{
	// workaround to read data from config file as UObjects are not initialized during PostConfigInit
	struct FSettings
	{
		FSettings(const FConfigSection* Section)
		{
			if (!Section)
			{
				return;
			}

			for (FConfigSectionMap::TConstIterator It(*Section); It; ++It)
			{
				const FName KeyName = FName(It.Key().GetPlainNameString());

				if (KeyName == GET_MEMBER_NAME_CHECKED(UShaderDirectoryMapperSettings, PluginsToRegister) ||
					KeyName == GET_MEMBER_NAME_CHECKED(UShaderDirectoryMapperSettings, PluginsToNeverRegister))
				{
					TSet<FString>& DstSet = (KeyName == GET_MEMBER_NAME_CHECKED(UShaderDirectoryMapperSettings, PluginsToRegister)) ?
											PluginsToRegister : PluginsToNeverRegister;
					
					FString ValueString = It.Value().GetValue();
					if (ValueString.StartsWith("(\"") && ValueString.EndsWith("\")")) //Key=("Value")
					{
						TArray<FString> Values;
						ValueString.LeftChop(2).RightChop(2).ParseIntoArrayWS(Values, TEXT(","), true);
						for (const FString& Value : Values)
						{
							DstSet.Add(Value);
						}
					}
					else if (ValueString.Len() > 2) //Key=(Value)
					{
						DstSet.Add(ValueString);
					}
				}
			}
		}

		FORCEINLINE bool IsPluginAllowed(const FString& PluginName) const
		{
			if (!PluginsToRegister.IsEmpty())
			{
				return PluginsToRegister.Contains(PluginName);
			}

			if (!PluginsToNeverRegister.IsEmpty())
			{
				return !PluginsToNeverRegister.Contains(PluginName);
			}

			return true;
		}

	private:
		TSet<FString> PluginsToRegister;
		TSet<FString> PluginsToNeverRegister;
	};

	virtual void StartupModule() override
	{
		// wait for PostConfigInit loading phase to register directories
		// this is required to make sure we don't add duplicates for plugins registering shaders manually (in StartupModule)
		IPluginManager::Get().OnLoadingPhaseComplete().AddRaw(this, &FShaderDirectoryMapperModule::OnPluginLoadingPhaseComplete);
	}

	void OnPluginLoadingPhaseComplete(ELoadingPhase::Type LoadingPhase, bool bSuccess)
	{
		if (!bSuccess)
		{
			return;
		}
		if (LoadingPhase != ELoadingPhase::PostConfigInit)
		{
			return;
		}

		const FConfigSection* Section = GConfig->GetSection(TEXT("/Script/ShaderDirectoryMapper.ShaderDirectoryMapperettings"), false, *GEngineIni);
		const FShaderDirectoryMapperModule::FSettings Settings{ Section };

		TArray<FString> ExistingShaderDirectories;
		for (const auto& Itr : AllShaderSourceDirectoryMappings())
		{
			// skip autogenerated directories (uproject always addds "ProjDir/Intermediate/ShaderAutogen")
			if (!Itr.Value.Contains(TEXT("/Intermediate")) &&
				!Itr.Value.Contains(TEXT("/ShaderAutogen")))
			{
				ExistingShaderDirectories.Add(Itr.Value);
			}
		}

		// returns false if any subdirectory of "BaseDirectory" is registered (assumes plugins only register one directory)
		auto IsPluginDirectoryAlreadyRegistered = [&](const FString& PluginBaseDirectory) -> bool
		{
			const bool bAlreadyAdded = ExistingShaderDirectories.ContainsByPredicate(
				[&](const FString& Directory)
				{
					return Directory.StartsWith(PluginBaseDirectory, ESearchCase::IgnoreCase);
				});
			return bAlreadyAdded;
		};

		// returns false only if it encounters exact path (otherwise returns false positives for ProjDir/Plugin/Shaders)
		auto IsProjectDirectoryAlreadyRegistered = [&](const FString& ProjectBaseDirectory) -> bool
		{
			const bool bAlreadyAdded = ExistingShaderDirectories.ContainsByPredicate(
				[&](const FString& Directory)
				{
					return Directory.Equals(ProjectBaseDirectory, ESearchCase::IgnoreCase);
				});
			return bAlreadyAdded;
		};

		auto TryAddingShaderDirectoryMapping = [](const FString& VirtualShaderDirectory, const FString& RealShaderDirectory)
		{
			if (IPlatformFile::GetPlatformPhysical().DirectoryExists(*RealShaderDirectory))
			{				
				AddShaderSourceDirectoryMapping(VirtualShaderDirectory, RealShaderDirectory);

				UE_LOG(LogTemp, Log, TEXT("[ShaderDirectoryMapper] Adding \"%s\" shader directory as \"%s\""), *RealShaderDirectory, *VirtualShaderDirectory);
			}
		};

		// add mapping for plugins
		for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPlugins())
		{
			const FString& PluginName = Plugin->GetName();
			if (!Settings.IsPluginAllowed(PluginName))
			{
				continue;
			}

			const FString PluginBaseDirectory = Plugin->GetBaseDir();			
			if (!IsPluginDirectoryAlreadyRegistered(PluginBaseDirectory))
			{
				const FString PluginShaderDirectory = FPaths::Combine(PluginBaseDirectory, TEXT("Shaders"));
				const FString VirtualPath = FString::Printf(TEXT("/%s"), *PluginName);
				TryAddingShaderDirectoryMapping(VirtualPath, PluginShaderDirectory);
			}
		}

		// add mapping for uproject
		const FString ProjectBaseDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		if (!IsProjectDirectoryAlreadyRegistered(ProjectBaseDirectory))
		{
			const FString ProjectName = FString(FApp::GetProjectName());
			const FString ProjectShaderDirectory = FPaths::Combine(ProjectBaseDirectory, TEXT("Shaders"));
			const FString VirtualPath = FString::Printf(TEXT("/%s"), *ProjectName);
			TryAddingShaderDirectoryMapping(VirtualPath, ProjectShaderDirectory);
		}
	}
};

IMPLEMENT_MODULE(FShaderDirectoryMapperModule, ShaderDirectoryMapper)