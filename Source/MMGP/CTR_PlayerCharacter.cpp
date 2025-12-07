#include "CTR_PlayerCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Blueprint/UserWidget.h"

ACTR_PlayerCharacter::ACTR_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	MoveDistance = 100.0f;
	CurrentScore = 0;
	MaxForwardDistance = 10.0f;  // 이름은 ForwardDistance지만, 이제 Y축(오른쪽) 거리를 저장하는 용도로 씁니다.
	MaxSideDistance = 500.0f;    // 도로 폭 (X축 방향 제한)

	TimeLeft = 60.0f;

	GetCharacterMovement()->MaxStepHeight = 0.0f;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	bCanMove = true;
	bIsOnLog = false;
	bIsDead = false;
	LogSpeed = 150.0f;
	MinSwipeDistance = 50.0f;
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

		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;
		PlayerController->bEnableTouchEvents = true;
	}

	if (TimerWidgetClass)
	{
		TimerWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), TimerWidgetClass);
		if (TimerWidgetInstance)
		{
			TimerWidgetInstance->AddToViewport(10);
		}
	}
}

void ACTR_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

	CheckFloor();

	if (TimeLeft > 0.0f)
	{
		TimeLeft -= DeltaTime;
		UpdateTimerUI(TimeLeft);

		if (TimeLeft <= 0.0f)
		{
			TimeLeft = 0.0f;
			GameOver();
		}
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
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACTR_PlayerCharacter::OnTouchPressed);
}

// [입력 방향 유지] 요청하신 대로 입력 로직은 그대로 둡니다.
void ACTR_PlayerCharacter::OnTouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (bIsDead) return;
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	int32 ScreenX, ScreenY;
	PC->GetViewportSize(ScreenX, ScreenY);
	float TouchX, TouchY;
	bool bIsPressed;
	PC->GetInputTouchState(FingerIndex, TouchX, TouchY, bIsPressed);

	FVector2D Center(ScreenX / 2.0f, ScreenY / 2.0f);
	FVector2D TouchLoc(TouchX, TouchY);
	FVector2D Direction = TouchLoc - Center;

	if (FMath::Abs(Direction.X) > FMath::Abs(Direction.Y))
	{
		// 가로 터치
		if (Direction.X > 0) MoveBackward(FInputActionValue());
		else MoveForward(FInputActionValue());
	}
	else
	{
		// 세로 터치
		// 화면 위를 누르면 MoveRight가 호출됩니다. -> 아래 MoveRight 함수에서 점수를 올립니다.
		if (Direction.Y < 0) MoveRight(FInputActionValue());
		else MoveLeft(FInputActionValue());
	}
}

// [수정됨] MoveRight가 이제 '전진(점수획득)' 역할을 합니다.
void ACTR_PlayerCharacter::MoveRight(const FInputActionValue& Value)
{
	// Y축(Right)으로 이동 시도
	if (TryMove(FVector::RightVector))
	{
		// Y좌표 기준으로 점수 계산 (오른쪽으로 갈수록 Y값이 커짐)
		float CurrentY = GetActorLocation().Y;

		// 기존 MaxForwardDistance 변수를 재활용하여 Y축 기록을 저장
		if (CurrentY > MaxForwardDistance)
		{
			CurrentScore++;
			MaxForwardDistance = CurrentY; // 최고 기록 갱신
			RequestSpawnTile(); // 맵 생성 요청
		}
	}
}

// 나머지 이동 함수는 점수 로직 없이 이동만 수행
void ACTR_PlayerCharacter::MoveForward(const FInputActionValue& Value) { TryMove(FVector::ForwardVector); }
void ACTR_PlayerCharacter::MoveLeft(const FInputActionValue& Value) { TryMove(FVector::LeftVector); }
void ACTR_PlayerCharacter::MoveBackward(const FInputActionValue& Value) { TryMove(FVector::BackwardVector); }

bool ACTR_PlayerCharacter::TryMove(FVector Direction)
{
	if (!bCanMove || bIsDead) return false;

	SetActorRotation(Direction.Rotation());

	FVector StartLocation = GetActorLocation();
	FVector EndLocationXY = StartLocation + (Direction * MoveDistance);

	// [★수정됨] 사망 체크 기준 변경
	// 게임이 오른쪽(Y축)으로 진행되므로, 낭떠러지는 앞/뒤(X축)를 검사해야 합니다.
	if (FMath::Abs(EndLocationXY.X) > MaxSideDistance)
	{
		TargetLocation = EndLocationXY;
		SetActorLocation(TargetLocation);
		GameOver(); // 도로 밖으로 나감 (낙사)
		return false;
	}

	FVector TraceStart = FVector(EndLocationXY.X, EndLocationXY.Y, StartLocation.Z + 500.0f);
	FVector TraceEnd = FVector(EndLocationXY.X, EndLocationXY.Y, StartLocation.Z - 500.0f);
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

	if (bHit)
	{
		float NewZ = HitResult.ImpactPoint.Z + GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		TargetLocation = FVector(EndLocationXY.X, EndLocationXY.Y, NewZ + 2.0f);
	}
	else
	{
		TargetLocation = EndLocationXY;
	}

	SetActorLocation(TargetLocation);

	bCanMove = false;
	GetWorld()->GetTimerManager().SetTimer(MoveTimerHandle, this, &ACTR_PlayerCharacter::ResetMove, 0.15f, false);

	return true;
}

void ACTR_PlayerCharacter::ResetMove() { bCanMove = true; }

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
		if (HitActor)
		{
			if (HitActor->ActorHasTag("Log")) bIsOnLog = true;
			else if (HitActor->ActorHasTag("Water")) { bIsOnLog = false; GameOver(); }
			else bIsOnLog = false;
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

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetCinematicMode(true, false, false, true, true);
	}
}