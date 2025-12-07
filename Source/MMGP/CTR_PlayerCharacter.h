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

	// --- [입력 시스템 (Enhanced Input)] ---
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

	// --- [터치 설정] ---
	// * 참고: 앞서 작성한 C++ 코드는 스와이프가 아닌 터치 위치(화면 위/아래/좌/우)를 감지합니다.
	UPROPERTY(EditAnywhere, Category = "Input|Touch")
	float MinSwipeDistance;

	// --- [타이머 시스템] ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Timer")
	float TimeLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game|Timer")
	TSubclassOf<UUserWidget> TimerWidgetClass;

	UPROPERTY()
	UUserWidget* TimerWidgetInstance;

	// C++에서 호출하면 블루프린트 위젯의 시간을 업데이트하는 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Game|Timer")
	void UpdateTimerUI(float CurrentTime);

	// --- [이동 및 게임플레이 변수] ---
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveDistance;     // 한 칸 이동 거리 (예: 100)

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSideDistance;  // 도로 좌우 폭 제한 (예: 500)

	bool bCanMove;
	FTimerHandle MoveTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game|Score")
	int32 CurrentScore;

	float MaxForwardDistance; // 최고 기록(X축 전진 거리) 저장용

	// --- [상태 변수] ---
	bool bIsOnLog;
	float LogSpeed;
	bool bIsDead;

	// --- [함수들] ---
	void MoveForward(const FInputActionValue& Value);
	void MoveLeft(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void MoveBackward(const FInputActionValue& Value);

	void OnTouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location);

	bool TryMove(FVector Direction);
	void ResetMove();
	void CheckFloor();
	void GameOver();

	// 새로운 타일을 생성하라고 블루프린트(MapManager)에 요청하는 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void RequestSpawnTile();
};