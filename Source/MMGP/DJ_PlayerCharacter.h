#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "DJ_PlayerCharacter.generated.h"

UCLASS()
class MMGP_API ADJ_PlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ADJ_PlayerCharacter();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// --- [타이머 시스템] ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	float TimeLeft;

	// 1. 블루프린트로 "야, 시간 이걸로 화면 갱신해!" 라고 신호 보내는 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Timer")
	void UpdateTimerUI(float CurrentTime);

	// 2. [추가됨] 블루프린트로 "야, 게임 끝났어! 결과창 띄워!" 라고 신호 보내는 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Game Logic")
	void OnGameOver();
};