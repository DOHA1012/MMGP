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
#include "Blueprint/UserWidget.h" // 위젯 헤더 필수!

ACTR_PlayerCharacter::ACTR_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	MoveDistance = 100.0f;
	CurrentScore = 0;
	MaxForwardDistance = 10.0f;
	MaxSideDistance = 500.0f;

	// 타이머 초기값 (60초)
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

		// 마우스 터치 활성화 (테스트용)
		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;
		PlayerController->bEnableTouchEvents = true;
	}

	// [★핵심] C++에서 타이머 위젯 생성 및 화면 부착
	if (TimerWidgetClass)
	{
		TimerWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), TimerWidgetClass);
		if (TimerWidgetInstance)
		{
			// ZOrder 10으로 설정하여 점수판보다 위에 보이게 함
			TimerWidgetInstance->AddToViewport(10);
		}
	}
}

void ACTR_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

	CheckFloor();

	// [★핵심] 타이머 감소 로직
	if (TimeLeft > 0.0f)
	{
		TimeLeft -= DeltaTime;

		// 블루프린트로 현재 시간 값을 보내줌 (UI 갱신용)
		UpdateTimerUI(TimeLeft);

		// 시간이 다 되면 게임 오버
		if (TimeLeft <= 0.0f)
		{
			TimeLeft = 0.0f;
			GameOver();
		}
	}

	if (bIsOnLog)
	{
		// 통나무 이동 로직 (필요시 활성화)
		// AddActorWorldOffset(FVector(0.0f, LogSpeed * DeltaTime, 0.0f));
	}
}

// ---------------------- 입력 및 이동 로직 ----------------------

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
	// 터치 바인딩
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACTR_PlayerCharacter::OnTouchPressed);
}

// 터치 이동 로직
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
		if (Direction.X > 0) MoveRight(FInputActionValue());
		else MoveLeft(FInputActionValue());
	}
	else
	{
		if (Direction.Y < 0) MoveForward(FInputActionValue());
		else MoveBackward(FInputActionValue());
	}
}

// 이동 함수들
void ACTR_PlayerCharacter::MoveForward(const FInputActionValue& Value) { TryMove(FVector::ForwardVector); }
void ACTR_PlayerCharacter::MoveLeft(const FInputActionValue& Value) { TryMove(FVector::LeftVector); }
void ACTR_PlayerCharacter::MoveRight(const FInputActionValue& Value)
{
	if (TryMove(FVector::RightVector))
	{
		float CurrentY = GetActorLocation().Y;
		if (CurrentY > MaxForwardDistance)
		{
			CurrentScore++;
			MaxForwardDistance = CurrentY;
			RequestSpawnTile();
		}
	}
}
void ACTR_PlayerCharacter::MoveBackward(const FInputActionValue& Value) { TryMove(FVector::BackwardVector); }

bool ACTR_PlayerCharacter::TryMove(FVector Direction)
{
	if (!bCanMove || bIsDead) return false;
	SetActorRotation(Direction.Rotation());

	FVector StartLocation = GetActorLocation();
	FVector EndLocationXY = StartLocation + (Direction * MoveDistance);

	if (FMath::Abs(EndLocationXY.X) > MaxSideDistance)
	{
		TargetLocation = EndLocationXY;
		SetActorLocation(TargetLocation);
		GameOver();
		return false;
	}

	// 높이 보정 로직
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
		if (HitActor && HitActor->ActorHasTag("Log")) bIsOnLog = true;
		else if (HitActor && HitActor->ActorHasTag("Water")) { bIsOnLog = false; GameOver(); }
		else bIsOnLog = false;
	}
	else bIsOnLog = false;
}

void ACTR_PlayerCharacter::GameOver()
{
	if (bIsDead) return;
	bIsDead = true;
	UE_LOG(LogTemp, Error, TEXT("Game Over!"));
}