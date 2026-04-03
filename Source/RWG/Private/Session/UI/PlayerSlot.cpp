// Fill out your copyright notice in the Description page of Project Settings.


#include "Session/UI/PlayerSlot.h"

#include "Components/CheckBox.h"
#include "Components/TextBlock.h"

DEFINE_LOG_CATEGORY(LogPlayerSlot);

void UPlayerSlot::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ensure(PlayerNameText != nullptr)) return;
	if (!ensure(ReadyCheckBox != nullptr)) return;
}

void UPlayerSlot::NativeDestruct()
{
	if(LinkedPlayerState.IsValid())
		LinkedPlayerState->OnReadyChanged.RemoveAll(this);

	Super::NativeDestruct();
}
void UPlayerSlot::BindPlayerState(ALobbyPlayerState* LobbyPS)
{
	LinkedPlayerState = LobbyPS;
	if (LinkedPlayerState.IsValid())
	{
		LobbyPS->OnReadyChanged.AddUObject(this, &ThisClass::HandleReadyChanged);
		HandleReadyChanged(LobbyPS->IsReady());
	}
}

bool UPlayerSlot::IsReady() const
{
	ECheckBoxState CheckBoxState = ReadyCheckBox->GetCheckedState();

	return CheckBoxState == ECheckBoxState::Checked ? true : false;
}

void UPlayerSlot::HandleReadyChanged(bool IsReady)
{
	IsReady ? ReadyCheckBox->SetCheckedState(ECheckBoxState::Checked) 
		: ReadyCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
}