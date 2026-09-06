// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Interfaces/InputBindable.h"
#include "Gameplay/Interfaces/WidgetBindable.h"
#include "InputComponentBase.generated.h"

class UInputConfigBase;
class UUserWidgetBase;

const FName InputConfigName = FName("InputConfig");

DECLARE_MULTICAST_DELEGATE_OneParam(FOnConfigLoaded, UInputComponentBase* InputComponent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API UInputComponentBase : public UActorComponent, public IInputBindable, public IWidgetBindable
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	// Sets default values for this component's properties
	UInputComponentBase();

protected:
	virtual void InitializeComponent() override;

	// Called when the game starts
	virtual void BeginPlay() override;

/*********************************************************************
*                             입력 설정
*********************************************************************/
public:
	FOnConfigLoaded OnConfigLoaded;

protected:
	bool TryLoadConfigFromAssetManager();

	void LoadInputConfig();

	void OnLoadedInputConfig();

	virtual TSubclassOf<UInputConfigBase> GetConfigClass() PURE_VIRTUAL(GetConfigClass, return nullptr;);

	virtual UInputConfigBase* GetInputConfig() const override;

	virtual UInputMappingContext* GetMappingContext() const override;

	bool IsConfigLoaded() const override;

	void BindOnConfigLoaded(TFunction<void()> Callback) override;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputConfigBase* LoadedConfig;

/*********************************************************************
*                            입력 바인딩
*********************************************************************/
protected:
	virtual void BindInputActions(UEnhancedInputComponent* InputComponent) override
		PURE_VIRTUAL(BindInputActions, );

	virtual int32 GetIMCPriority() const override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	int32 IMCPriority = 0;

/*********************************************************************
*                             위젯 연결
*********************************************************************/
protected:
	virtual TArray<TSubclassOf<UUserWidgetBase>> GetDefaultWidgetClasses() const override;

	virtual void BindComponent(UUserWidgetBase* Widget) override;

	virtual void UnbindComponent(UUserWidgetBase* Widget) override;
};
