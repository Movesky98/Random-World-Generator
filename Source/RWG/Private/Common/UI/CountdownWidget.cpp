// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/UI/CountdownWidget.h"

#include "Components/TextBlock.h"

void UCountdownWidget::InitializeCountdown(const FText& InTitle, const FGetRemainingSecondsDelegate& InDelegate)
{
	TitleText->SetText(InTitle);

	RemainingSecondsProvider = InDelegate;
}

void UCountdownWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (RemainingSecondsProvider.IsBound())
	{
		int32 Second = FMath::CeilToInt(RemainingSecondsProvider.Execute());

		if (Second != LastSecond)
		{
			FText SecondText = FText::FromString(FString::FromInt(Second));
			CountdownText->SetText(SecondText);
			LastSecond = Second;
		}
	}
}
