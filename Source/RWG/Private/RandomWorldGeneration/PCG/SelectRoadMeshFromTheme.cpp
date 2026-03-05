// Fill out your copyright notice in the Description page of Project Settings.

#include "RandomWorldGeneration/PCG/SelectRoadMeshFromTheme.h"

#include "PCGContext.h"
#include "Data/PCGPointData.h"
#include "Metadata/PCGMetadata.h"
#include "PCGParamData.h"
#include "PCGComponent.h"

#define LOCTEXT_NAMESPACE "SelectRoadMeshFromThemeElement"

#if WITH_EDITOR
// The label the node is known as internally.
FName USelectRoadMeshFromThemeSettings::GetDefaultNodeName() const
{
	return FName(TEXT("SelectRoadMeshFromTheme"));
}

// Default node name shown in the graph editor. Include spaces.
FText USelectRoadMeshFromThemeSettings::GetDefaultNodeTitle() const
{
	return LOCTEXT("NodeTitle", "SelectRoadMeshFromTheme");
}

// Default tooltip for the node
FText USelectRoadMeshFromThemeSettings::GetNodeTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Add tooltip here.");
}
#endif //WITH_EDITOR

namespace SelectRoadMeshFromTheme
{
	static const FName InPinLabel = PCGPinConstants::DefaultInputLabel;
	static const FName OutPinLabel = PCGPinConstants::DefaultOutputLabel;
	static const FName ConfigPinLabel = FName(TEXT("Config"));
}

// Input/Output pin setup with specific properties, including:
// Pin data type, allowing singular or multiple inputs per pin, and creating multiple in/out pins.
TArray<FPCGPinProperties> USelectRoadMeshFromThemeSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.Emplace(SelectRoadMeshFromTheme::InPinLabel, EPCGDataType::Point);

	FPCGPinProperties ConfigPin;
	ConfigPin.Label = SelectRoadMeshFromTheme::ConfigPinLabel;
	ConfigPin.AllowedTypes = EPCGDataType::Param;
	Pins.Add(ConfigPin);
	
	return Pins;
}

TArray<FPCGPinProperties> USelectRoadMeshFromThemeSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.Emplace(SelectRoadMeshFromTheme::OutPinLabel, EPCGDataType::Point);
	return Pins;
}

// Creates the Element to be used for ExecuteInternal.
FPCGElementPtr USelectRoadMeshFromThemeSettings::CreateElement() const
{
	return MakeShared<FSelectRoadMeshFromThemeElement>();
}

/*
* Processing function for this node. 
* Context holds the InputData, containing the input data collection for this node 
* and the OutputData, the output data collection to write to as output.
* Returns true if the processing is done. 
* Returning false will call back this function at next tick, and will call it until it returns true.
* Settings contains all the setup options for this node, and if a property was marked PCG_Overridable, 
* "Context->GetInputSettings" will contain the overridden value for this property if it is overridden
*/ 

bool FSelectRoadMeshFromThemeElement::ExecuteInternal(FPCGContext* Context) const
{
    const USelectRoadMeshFromThemeSettings* Settings =
    Context ? Context->GetInputSettings<USelectRoadMeshFromThemeSettings>() : nullptr;


    
    if (!Settings)
    {
        return true; // 조용히 통과(원하면 Warning 로그)
    }

    /* ----------- 1. Input Pin으로 들어온 Config 속성의 경로를 이용해 원본 로드 ----------- */
    const TArray<FPCGTaggedData>& InConfigData = Context->InputData.GetInputsByPin(SelectRoadMeshFromTheme::ConfigPinLabel);
    if (InConfigData.IsEmpty())
    {
        return true;
    }

    const UPCGParamData* ConfigParam = Cast<UPCGParamData>(InConfigData[0].Data);
    if (!ConfigParam || !ConfigParam->Metadata)
    {
        return true;
    }

    const FPCGMetadataAttributeBase* AttributeBase = ConfigParam->Metadata->GetConstAttribute(Settings->ThemeConfigAttribute);
    if (!AttributeBase)
    {
        return true;
    }

    const auto* ThemePathAttribute = static_cast<const FPCGMetadataAttribute<FSoftObjectPath>*>(AttributeBase);
    const FSoftObjectPath ThemeConfigPath = ThemePathAttribute->GetValueFromItemKey(0);
    if (!ThemeConfigPath.IsValid())
    {
        return true;
    }

    UObject* LoadedObject = ThemeConfigPath.TryLoad();
    UWorldThemeConfig* ThemeConfig = Cast<UWorldThemeConfig>(LoadedObject);
    if (!ThemeConfig)
    {
        return true;
    }

    /* ----------- 2. 경로를 통해 DA_WorldThemeConfig 로드 → Roads 가져옴 ----------- */
    const TArray<UStaticMesh*>& Roads = ThemeConfig->Roads;
    if(Roads.IsEmpty()) { return true; }

    // 입력 포인트 데이터 가져오기
    const TArray<FPCGTaggedData>& Inputs = Context->InputData.GetInputsByPin(SelectRoadMeshFromTheme::InPinLabel);
    if (Inputs.Num() <= 0)
    {
        return true;
    }

    // 출력 포인트 데이터 준비
    const UPCGPointData* InPointData = Cast<UPCGPointData>(Inputs[0].Data);
    if (!InPointData) return true;

    UPCGPointData* OutPointData = Cast<UPCGPointData>(InPointData->DuplicateData(Context, true));
    TArray<FPCGPoint>& OutPoints = OutPointData->GetMutablePoints();

    // Attribute Writer (SoftObjectPath를 문자열로 저장하는 방식이 가장 덜 꼬임)
    UPCGMetadata* Metadata = OutPointData->MutableMetadata();
    check(Metadata);

    // String attribute 만들기
    FPCGMetadataAttribute<FString>* Attr = Metadata->FindOrCreateAttribute<FString>(
        Settings->OutMeshAttribute,
        FString(),
        false,
        true
    );

    // 랜덤 시드
    const int32 BaseSeed = Context->GetSeed() ^ Settings->ExtraSeed;
    FRandomStream Rng(BaseSeed);

    // 포인트마다 선택 (완전 랜덤)
    for (FPCGPoint& Pt : OutPoints)
    {
        const int32 Idx = Rng.RandRange(0, Roads.Num() - 1);

        const TSoftObjectPtr<UStaticMesh>& Pick = Roads[Idx];
        const FSoftObjectPath Path = Pick.ToSoftObjectPath();

        // 포인트의 메타데이터 엔트리
        const PCGMetadataEntryKey Key = Pt.MetadataEntry;
        Attr->SetValue(Key, Path.ToString());
    }

    // 출력 연결
    FPCGTaggedData& Out = Context->OutputData.TaggedData.Emplace_GetRef();
    Out.Pin = SelectRoadMeshFromTheme::OutPinLabel;
    Out.Data = OutPointData;

    return true;
}

#undef LOCTEXT_NAMESPACE