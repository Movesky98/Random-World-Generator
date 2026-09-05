// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/Convict/Animation/ConvictAnimInstance.h"
#include "Gameplay/Characters/Convict/Convict.h"

void UConvictAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	AConvict* Convict = Cast<AConvict>(OwnerCharacter);
	if (!Convict) return;

	const FRotator Rotation = Convict->GetActorRotation();
	const FRotator AimRotation = Convict->GetBaseAimRotation();
	const FRotator AimDelta = (AimRotation - Rotation).GetNormalized();

	// Pitch는 엔진이 RemoteViewPitch16으로 복제해줘서 시뮬레이티드 프록시에서도 값이 들어온다.
	AimPitch = AimDelta.Pitch;

	// 시뮬레이티드 프록시는 그 플레이어의 컨트롤러가 없어 GetBaseAimRotation()이 액터 회전을 돌려준다.
	// 그래서 AimDelta.Yaw 대신, 복제받은 조준 방향에서 몸 각도를 빼 각도 차를 만든다.
	if (Convict->GetLocalRole() == ROLE_SimulatedProxy)
	{
		// 메시의 월드 회전에서 기본 상대 회전을 걷어내 캡슐과 같은 기준의 몸 각도를 얻는다.
		const FQuat MeshQuat = Convict->GetMesh()->GetComponentQuat() * Convict->GetBaseRotationOffset().Inverse();

		AimYaw = FRotator::NormalizeAxis(Convict->GetRemoteViewYaw() - MeshQuat.Rotator().Yaw);
	}
	else
	{
		AimYaw = AimDelta.Yaw;
	}
}
