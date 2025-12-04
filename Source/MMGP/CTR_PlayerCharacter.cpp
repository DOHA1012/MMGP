#include "CTR_PlayerCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

ACTR_PlayerCharacter::ACTR_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	MoveDistance = 100.0f;
	CurrentScore = 0;
	MaxForwardDistance = 10.0f;

	// 맵 좌우 한계 (기본 5칸 정도 여유)
	MaxSideDistance = 500.0f;

	// 낮은 장애물을 밟고 올라가지 못하게 설정
	GetCharacterMovement()->MaxStepHeight = 0.0f;

	// 회전 관련 설정 강제 적용
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	bCanMove = true;
	bIsOnLog = false;
	bIsDead = false;
	LogSpeed = 150.0f;
}

void ACTR_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ACTR_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

	// 1. 발밑 검사
	CheckFloor();

	// 2. 통나무 위에 있다면 같이 이동
	if (bIsOnLog)
	{
		// 통나무 방향에 따라 부호(+/-) 조절 필요
		// AddActorWorldOffset(FVector(0.0f, LogSpeed * DeltaTime, 0.0f));
	}
}

void ACTR_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Started, this, &ACTR_PlayerCharacter::MoveForward);
		EnhancedInputComponent->BindAction(MoveLeftAction, ETriggerEvent::Started, this, &ACTR_PlayerCharacter::MoveLeft);
		EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Started, this, &ACTR_PlayerCharacter::MoveRight);
		EnhancedInputComponent->BindAction(MoveBackwardAction, ETriggerEvent::Started, this, &ACTR_PlayerCharacter::MoveBackward);
	}
}

// 1. 'W' 키 -> 화면상 왼쪽 (월드 X축)
void ACTR_PlayerCharacter::MoveForward(const FInputActionValue& Value)
{
	TryMove(FVector::ForwardVector);
}

// 2. 'A' 키 -> 화면상 뒤로 (월드 -Y축)
void ACTR_PlayerCharacter::MoveLeft(const FInputActionValue& Value)
{
	TryMove(FVector::LeftVector);
}

// 3. 'D' 키 -> 화면상 전진 (월드 Y축) -> 점수 획득
void ACTR_PlayerCharacter::MoveRight(const FInputActionValue& Value)
{
	if (TryMove(FVector::RightVector))
	{
		// 전진 방향(Y축) 좌표로 점수 계산
		float CurrentY = GetActorLocation().Y;

		if (CurrentY > MaxForwardDistance)
		{
			CurrentScore++;
			MaxForwardDistance = CurrentY;
			// UE_LOG(LogTemp, Warning, TEXT("Score: %d"), CurrentScore);

			// [★ 추가됨] 점수가 올랐다는 건 앞으로 갔다는 뜻! 맵을 더 깔아라!
			RequestSpawnTile();
		}
	}
}

// 4. 'S' 키 -> 화면상 오른쪽 (월드 -X축)
void ACTR_PlayerCharacter::MoveBackward(const FInputActionValue& Value)
{
	TryMove(FVector::BackwardVector);
}

// [핵심 이동 로직]
bool ACTR_PlayerCharacter::TryMove(FVector Direction)
{
	if (!bCanMove || bIsDead) return false;

	// 1. 캐릭터 회전
	SetActorRotation(Direction.Rotation());

	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation + (Direction * MoveDistance);

	// 2. 맵 밖으로 나갔는지(낭떠러지) 검사 - X축 기준 (W, S 방향)
	if (FMath::Abs(EndLocation.X) > MaxSideDistance)
	{
		UE_LOG(LogTemp, Error, TEXT("Fall out of map!"));
		TargetLocation = EndLocation; // 허공으로 이동
		SetActorLocation(TargetLocation);

		GameOver(); // 사망 처리
		return false;
	}

	// 3. 장애물 검사 (LineTrace)
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);

	if (bHit) return false; // 막힘

	// 4. 이동 수행
	TargetLocation = EndLocation;
	SetActorLocation(TargetLocation);
	Jump();

	bCanMove = false;
	GetWorld()->GetTimerManager().SetTimer(MoveTimerHandle, this, &ACTR_PlayerCharacter::ResetMove, 0.15f, false);

	return true;
}

void ACTR_PlayerCharacter::ResetMove()
{
	bCanMove = true;
}

void ACTR_PlayerCharacter::CheckFloor()
{
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 150.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();

		if (HitActor && HitActor->ActorHasTag("Log"))
		{
			bIsOnLog = true;
		}
		else if (HitActor && HitActor->ActorHasTag("Water"))
		{
			bIsOnLog = false;
			GameOver();
		}
		else
		{
			bIsOnLog = false;
		}
	}
	else
	{
		bIsOnLog = false;
	}
}

void ACTR_PlayerCharacter::GameOver()
{
	if (bIsDead) return;

	bIsDead = true;
	UE_LOG(LogTemp, Error, TEXT("Game Over!"));

	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}