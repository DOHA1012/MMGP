#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Blueprint/UserWidget.h" 
#include "CTR_PlayerCharacter.generated.h"

UCLASS()
class MMGP_API ACTR_PlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACTR_PlayerCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	// --- [입력] ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveForwardAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveLeftAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveRightAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveBackwardAction;

	UPROPERTY(EditAnywhere, Category = "Input|Touch")
	float MinSwipeDistance;

	// --- [타이머] ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Timer")
	float TimeLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game|Timer")
	TSubclassOf<UUserWidget> TimerWidgetClass;

	UPROPERTY()
	UUserWidget* TimerWidgetInstance;

	UFUNCTION(BlueprintImplementableEvent, Category = "Game|Timer")
	void UpdateTimerUI(float CurrentTime);

	// --- [이동 및 점수] ---
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveDistance;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSideDistance;

	bool bCanMove;
	FTimerHandle MoveTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game|Score")
	int32 CurrentScore;

	float MaxForwardDistance;

	// --- [상태 변수] ---
	bool bIsOnLog;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float LogSpeed;

	bool bIsDead;

	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void StartGameOverSequence();

	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void RequestSpawnTile();

	// --- [함수들] ---
	void MoveForward(const FInputActionValue& Value);
	void MoveLeft(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void MoveBackward(const FInputActionValue& Value);

	// [★추가됨] 마우스 클릭 감지 함수
	void OnMouseClicked();

	// [★추가됨] 화면 입력(터치/마우스) 공통 처리 함수
	void ProcessScreenInput(const FVector2D InputLocation);

	void OnTouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location);

	bool TryMove(FVector Direction);
	void ResetMove();
	void CheckFloor();
	void GameOver();
};