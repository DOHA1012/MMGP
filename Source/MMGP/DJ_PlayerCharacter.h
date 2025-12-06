#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h" // 위젯 사용 필수
#include "DJ_PlayerCharacter.generated.h"

UCLASS()
class MMGP_API ADJ_PlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// 생성자
	ADJ_PlayerCharacter();

	// 매 프레임 실행 (시간 깎기용)
	virtual void Tick(float DeltaTime) override;

protected:
	// 게임 시작 시 실행 (위젯 생성용)
	virtual void BeginPlay() override;

	// --- [타이머 시스템] ---

	// 1. 남은 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	float TimeLeft;

	// 2. 위젯 클래스 (에디터에서 WBP_Timer 넣을 곳)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer")
	TSubclassOf<UUserWidget> TimerWidgetClass;

	// 3. 실제 생성된 위젯
	UPROPERTY()
	UUserWidget* TimerWidgetInstance;

	// 4. 블루프린트로 신호 보내는 함수
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTimerUI(float CurrentTime);
};