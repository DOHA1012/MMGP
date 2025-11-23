#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "CTR_PlayerCharacter.generated.h"

UCLASS()
class MMGP_API ACTR_PlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACTR_PlayerCharacter();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// --- [입력 관련 변수] ---
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

	// --- [이동 관련 변수] ---
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveDistance;

	// 맵의 좌우 한계 거리 (이보다 멀어지면 사망)
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSideDistance;

	// 연타 방지용
	bool bCanMove;
	FTimerHandle MoveTimerHandle;

	// --- [점수 관련 변수] ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 CurrentScore;
	float MaxForwardDistance;

	// --- [강/통나무 시스템 변수] ---
	bool bIsOnLog;
	float LogSpeed;
	bool bIsDead;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// 이동 함수들
	void MoveForward(const FInputActionValue& Value);
	void MoveLeft(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void MoveBackward(const FInputActionValue& Value);

	bool TryMove(FVector Direction);
	void ResetMove();

	void CheckFloor();
	void GameOver();

	// [★ 추가됨] 블루프린트로 "맵 깔아줘!" 신호 보내는 함수
	UFUNCTION(BlueprintImplementableEvent)
	void RequestSpawnTile();
};