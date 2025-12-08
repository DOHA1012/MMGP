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
	MaxForwardDistance = 10.0f;
	MaxSideDistance = 250.0f;

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

	// [★수정됨] 터치 입력 바인딩
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACTR_PlayerCharacter::OnTouchPressed);

	// [★추가됨] 마우스 클릭 바인딩 (설정 껐을 때 대비)
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACTR_PlayerCharacter::OnMouseClicked);
}

// [모바일] 터치 좌표를 받아서 처리
void ACTR_PlayerCharacter::OnTouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	ProcessScreenInput(FVector2D(Location.X, Location.Y));
}

// [PC] 마우스 좌표를 찾아서 처리
void ACTR_PlayerCharacter::OnMouseClicked()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		float MouseX, MouseY;
		if (PC->GetMousePosition(MouseX, MouseY))
		{
			ProcessScreenInput(FVector2D(MouseX, MouseY));
		}
	}
}

// [★핵심] 좌표를 기반으로 이동 방향 결정 (터치/마우스 공용)
void ACTR_PlayerCharacter::ProcessScreenInput(const FVector2D InputLocation)
{
	if (bIsDead) return;
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	int32 ScreenX, ScreenY;
	PC->GetViewportSize(ScreenX, ScreenY);

	FVector2D Center(ScreenX / 2.0f, ScreenY / 2.0f);
	FVector2D Direction = InputLocation - Center;

	// 가로(X) vs 세로(Y) 변화량 비교
	if (FMath::Abs(Direction.X) > FMath::Abs(Direction.Y))
	{
		// 가로 터치
		if (Direction.X > 0) MoveBackward(FInputActionValue());
		else MoveForward(FInputActionValue());
	}
	else
	{
		// 세로 터치
		if (Direction.Y < 0) MoveRight(FInputActionValue());
		else MoveLeft(FInputActionValue());
	}
}

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

void ACTR_PlayerCharacter::MoveForward(const FInputActionValue& Value) { TryMove(FVector::ForwardVector); }
void ACTR_PlayerCharacter::MoveLeft(const FInputActionValue& Value) { TryMove(FVector::LeftVector); }
void ACTR_PlayerCharacter::MoveBackward(const FInputActionValue& Value) { TryMove(FVector::BackwardVector); }

bool ACTR_PlayerCharacter::TryMove(FVector Direction)
{
	if (!bCanMove || bIsDead) return false;

	SetActorRotation(Direction.Rotation());

	FVector StartLocation = GetActorLocation();
	FVector EndLocationXY = StartLocation + (Direction * MoveDistance);

	// 격자 정렬 (100단위)
	EndLocationXY.X = FMath::RoundToFloat(EndLocationXY.X / MoveDistance) * MoveDistance;
	EndLocationXY.Y = FMath::RoundToFloat(EndLocationXY.Y / MoveDistance) * MoveDistance;

	if (FMath::Abs(EndLocationXY.X) > MaxSideDistance)
	{
		TargetLocation = EndLocationXY;
		SetActorLocation(TargetLocation);
		GameOver();
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
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->ActorHasTag("Wall"))
		{
			return false;
		}

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
	UE_LOG(LogTemp, Error, TEXT("Game Over triggered! Calling Blueprint Sequence."));

	StartGameOverSequence();
}