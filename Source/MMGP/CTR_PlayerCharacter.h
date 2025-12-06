#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
<<<<<<< HEAD
=======
#include "Blueprint/UserWidget.h" // 위젯 사용을 위해 추가
>>>>>>> choiseik
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

<<<<<<< HEAD
	// --- [입력 관련 변수] ---
=======
	// --- [입력 관련 (Enhanced Input)] ---
>>>>>>> choiseik
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

<<<<<<< HEAD
	// --- [이동 관련 변수] ---
=======
	// --- [터치 조작 관련] ---
	FVector2D TouchStartLocation;
	UPROPERTY(EditAnywhere, Category = "Input|Touch")
	float MinSwipeDistance;

	// --- [타이머 시스템 (★추가됨)] ---

	// 1. 남은 시간 (초 단위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	float TimeLeft;

	// 2. 에디터에서 설정할 위젯 클래스 (WBP_Timer를 여기에 넣으세요)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer")
	TSubclassOf<UUserWidget> TimerWidgetClass;

	// 3. 실제 생성된 위젯 인스턴스
	UPROPERTY()
	UUserWidget* TimerWidgetInstance;

	// 4. 블루프린트로 "UI 업데이트해!"라고 명령 보내는 함수
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTimerUI(float CurrentTime);


	// --- [이동 및 게임플레이 변수] ---
>>>>>>> choiseik
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveDistance;
<<<<<<< HEAD

	// 맵의 좌우 한계 거리 (이보다 멀어지면 사망)
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSideDistance;

	// 연타 방지용
	bool bCanMove;
	FTimerHandle MoveTimerHandle;

	// --- [점수 관련 변수] ---
=======
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSideDistance;

	bool bCanMove;
	FTimerHandle MoveTimerHandle;

>>>>>>> choiseik
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 CurrentScore;
	float MaxForwardDistance;

<<<<<<< HEAD
	// --- [강/통나무 시스템 변수] ---
=======
>>>>>>> choiseik
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

<<<<<<< HEAD
	bool TryMove(FVector Direction);
	void ResetMove();

	void CheckFloor();
	void GameOver();

	// [★ 추가됨] 블루프린트로 "맵 깔아줘!" 신호 보내는 함수
=======
	// 터치 함수들
	void OnTouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location);

	// 공통 로직
	bool TryMove(FVector Direction);
	void ResetMove();
	void CheckFloor();
	void GameOver();

>>>>>>> choiseik
	UFUNCTION(BlueprintImplementableEvent)
	void RequestSpawnTile();
};