#include "CTR_PlayerCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
<<<<<<< HEAD
=======
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h" 
#include "Blueprint/UserWidget.h" // 위젯 헤더 필수!
>>>>>>> choiseik

ACTR_PlayerCharacter::ACTR_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	MoveDistance = 100.0f;
	CurrentScore = 0;
	MaxForwardDistance = 10.0f;
<<<<<<< HEAD

	// 맵 좌우 한계 (기본 5칸 정도 여유)
	MaxSideDistance = 500.0f;

	// 낮은 장애물을 밟고 올라가지 못하게 설정
	GetCharacterMovement()->MaxStepHeight = 0.0f;

	// 회전 관련 설정 강제 적용
=======
	MaxSideDistance = 500.0f;

	// 타이머 초기값 (60초)
	TimeLeft = 60.0f;

	GetCharacterMovement()->MaxStepHeight = 0.0f;
>>>>>>> choiseik
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	bCanMove = true;
	bIsOnLog = false;
	bIsDead = false;
	LogSpeed = 150.0f;
<<<<<<< HEAD
=======
	MinSwipeDistance = 50.0f;
>>>>>>> choiseik
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
<<<<<<< HEAD
=======

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
>>>>>>> choiseik
	}
}

void ACTR_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

<<<<<<< HEAD
	// 1. 발밑 검사
	CheckFloor();

	// 2. 통나무 위에 있다면 같이 이동
	if (bIsOnLog)
	{
		// 통나무 방향에 따라 부호(+/-) 조절 필요
=======
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
>>>>>>> choiseik
		// AddActorWorldOffset(FVector(0.0f, LogSpeed * DeltaTime, 0.0f));
	}
}

<<<<<<< HEAD
=======
// ---------------------- 입력 및 이동 로직 ----------------------

>>>>>>> choiseik
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
<<<<<<< HEAD
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
=======
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
>>>>>>> choiseik
void ACTR_PlayerCharacter::MoveRight(const FInputActionValue& Value)
{
	if (TryMove(FVector::RightVector))
	{
<<<<<<< HEAD
		// 전진 방향(Y축) 좌표로 점수 계산
		float CurrentY = GetActorLocation().Y;

=======
		float CurrentY = GetActorLocation().Y;
>>>>>>> choiseik
		if (CurrentY > MaxForwardDistance)
		{
			CurrentScore++;
			MaxForwardDistance = CurrentY;
<<<<<<< HEAD
			// UE_LOG(LogTemp, Warning, TEXT("Score: %d"), CurrentScore);

			// [★ 추가됨] 점수가 올랐다는 건 앞으로 갔다는 뜻! 맵을 더 깔아라!
=======
>>>>>>> choiseik
			RequestSpawnTile();
		}
	}
}
<<<<<<< HEAD

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
=======
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
>>>>>>> choiseik

void ACTR_PlayerCharacter::CheckFloor()
{
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 150.0f);
<<<<<<< HEAD

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

=======
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
>>>>>>> choiseik
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
<<<<<<< HEAD

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
=======
		if (HitActor && HitActor->ActorHasTag("Log")) bIsOnLog = true;
		else if (HitActor && HitActor->ActorHasTag("Water")) { bIsOnLog = false; GameOver(); }
		else bIsOnLog = false;
	}
	else bIsOnLog = false;
>>>>>>> choiseik
}

void ACTR_PlayerCharacter::GameOver()
{
	if (bIsDead) return;
<<<<<<< HEAD

	bIsDead = true;
	UE_LOG(LogTemp, Error, TEXT("Game Over!"));

	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
=======
	bIsDead = true;
	UE_LOG(LogTemp, Error, TEXT("Game Over!"));
>>>>>>> choiseik
}