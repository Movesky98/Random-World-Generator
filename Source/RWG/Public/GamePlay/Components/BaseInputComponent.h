// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GamePlay/Interfaces/InputBindable.h"
#include "BaseInputComponent.generated.h"

class UBaseInputConfig;

const FName InputConfigName = FName("InputConfig");

DECLARE_MULTICAST_DELEGATE_OneParam(FOnConfigLoaded, UBaseInputComponent* InputComponent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API UBaseInputComponent : public UActorComponent, public IInputBindable
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBaseInputComponent();

	FOnConfigLoaded OnConfigLoaded;

protected:
	virtual void InitializeComponent() override;

	// Called when the game starts
	virtual void BeginPlay() override;

	bool TryLoadConfigFromAssetManager();

	void LoadInputConfig();

	void OnLoadedInputConfig();
	
	virtual void BindInputActions(UEnhancedInputComponent* InputComponent) override
		PURE_VIRTUAL(BindInputActions, );

	virtual UBaseInputConfig* GetInputConfig() const override;

	virtual int32 GetIMCPriority() const override;

	virtual UInputMappingContext* GetMappingContext() const override;


	virtual TSubclassOf<UBaseInputConfig> GetConfigClass() PURE_VIRTUAL(GetConfigClass, return nullptr;);

	UPROPERTY(EditAnywhere, Category = "Input")
	UBaseInputConfig* LoadedConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	int32 IMCPriority = 0;
};
