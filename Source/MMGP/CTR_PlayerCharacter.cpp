#include "CTR_PlayerCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h" // [추가] 레벨 재시작(사망 처리)용

ACTR_PlayerCharacter::ACTR_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	MoveDistance = 100.0f;
	CurrentScore = 0;
	MaxForwardDistance = 10.0f;
	GetCharacterMovement()->MaxStepHeight = 0.0f;
	bCanMove = true;

	// [★ 초기화]
	bIsOnLog = false;
	bIsDead = false;
	LogSpeed = 150.0f; // 블루프린트의 통나무 속도와 똑같이 맞추세요!
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

// [★ 매 프레임 실행]
void ACTR_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return; // 죽었으면 아무것도 안 함

	// 1. 발밑 검사
	CheckFloor();

	// 2. 통나무 위에 있다면? -> 통나무 속도만큼 같이 이동!
	if (bIsOnLog)
	{
		// Y축(옆)으로 이동 (통나무 방향에 따라 부호 +/- 확인 필요)
		// 만약 통나무가 왼쪽으로 가면 -LogSpeed 로 바꾸세요.
		//AddActorWorldOffset(FVector(0.0f, LogSpeed * DeltaTime, 0.0f));
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

void ACTR_PlayerCharacter::MoveForward(const FInputActionValue& Value) { TryMove(GetActorForwardVector()); }
void ACTR_PlayerCharacter::MoveLeft(const FInputActionValue& Value) { TryMove(-GetActorRightVector()); }

void ACTR_PlayerCharacter::MoveRight(const FInputActionValue& Value)
{
	if (TryMove(GetActorRightVector()))
	{
		float CurrentY = GetActorLocation().Y;
		if (CurrentY > MaxForwardDistance)
		{
			CurrentScore++;
			MaxForwardDistance = CurrentY;
		}
	}
}

void ACTR_PlayerCharacter::MoveBackward(const FInputActionValue& Value) { TryMove(-GetActorForwardVector()); }

bool ACTR_PlayerCharacter::TryMove(FVector Direction)
{
	if (!bCanMove || bIsDead) return false;

	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation + (Direction * MoveDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);

	if (bHit) return false;

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

// [★ 발밑 검사 함수 구현]
void ACTR_PlayerCharacter::CheckFloor()
{
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 150.0f); // 발 아래로 넉넉하게 레이저 발사

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();

		// 1. 통나무("Log")를 밟았을 때
		if (HitActor && HitActor->ActorHasTag("Log"))
		{
			bIsOnLog = true;
			// UE_LOG(LogTemp, Warning, TEXT("통나무 탑승!"));
		}
		// 2. 물("Water")을 밟았을 때 (BP_RiverTile 태그 설정 필수!)
		else if (HitActor && HitActor->ActorHasTag("Water"))
		{
			bIsOnLog = false;
			GameOver(); // 사망!
		}
		// 3. 그냥 땅(도로, 풀밭)일 때
		else
		{
			bIsOnLog = false;
		}
	}
	else
	{
		// 공중에 떠 있거나 바닥이 없을 때
		bIsOnLog = false;
	}
}

// [★ 사망 처리 함수]
void ACTR_PlayerCharacter::GameOver()
{
	if (bIsDead) return; // 이미 죽었으면 중복 실행 방지

	bIsDead = true;
	UE_LOG(LogTemp, Error, TEXT("으악! 물에 빠졌다! Game Over"));

	// 여기서 "Game Over" 위젯을 띄우거나 레벨을 재시작하면 됩니다.
	// 지금은 바로 레벨 재시작 (죽으면 처음부터)
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}