// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/DataAssets/WorldThemeConfig.h"


FPrimaryAssetId UWorldThemeConfig::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("WorldThemeConfig", GetFName());
}

void UWorldThemeConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangeEvent)
{
	Super::PostEditChangeProperty(PropertyChangeEvent);

	const FName PropertyName = PropertyChangeEvent.Property ?
		PropertyChangeEvent.GetPropertyName() : NAME_None;

	const FName MemberPropertyName = PropertyChangeEvent.MemberProperty ?
		PropertyChangeEvent.MemberProperty->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(FBuildingAssetEntry, Mesh) ||
		MemberPropertyName == GET_MEMBER_NAME_CHECKED(UWorldThemeConfig, BuildingEntries) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UWorldThemeConfig, CellSize))
	{
		RefreshBuildingFootprints();
	}
}

void UWorldThemeConfig::RefreshBuildingFootprints()
{
	if (CellSize <= 0.0f)
		return;

	for (FBuildingAssetEntry& Entry : BuildingEntries)
	{
		RefreshSingleBuildingFootprint(Entry);
	}

#if WITH_EDITOR
	Modify();
#endif
}

void UWorldThemeConfig::RefreshSingleBuildingFootprint(FBuildingAssetEntry& Entry) const
{
	UStaticMesh* Mesh = Entry.Mesh.LoadSynchronous();
	if (!Mesh || CellSize <= 0.0f)
	{
		Entry.FootprintSizeX = 1;
		Entry.FootprintSizeY = 1;

		return;
	}

	const FBoxSphereBounds Bounds = Mesh->GetBounds();

	const float SizeX = Bounds.BoxExtent.X * 2.0f;
	const float SizeY = Bounds.BoxExtent.Y * 2.0f;

	Entry.FootprintSizeX = FMath::Max(1, FMath::CeilToInt(SizeX / CellSize));
	Entry.FootprintSizeY = FMath::Max(1, FMath::CeilToInt(SizeY / CellSize));
}
