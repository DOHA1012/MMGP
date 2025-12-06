#include "DJ_PlayerCharacter.h"
#include "Blueprint/UserWidget.h" 
// ... 기타 필요한 헤더 ...

ADJ_PlayerCharacter::ADJ_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// ... 기타 생성자 코드 ...

	// 타이머 초기값 (60초)
	TimeLeft = 60.0f;
}

void ADJ_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay(); // (BP에서 연결을 끊었더라도, 이곳은 원래 실행되어야 합니다.)

	// [1차 시도] BeginPlay에서 타이머 위젯 생성 및 화면 부착
	if (TimerWidgetClass)
	{
		TimerWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), TimerWidgetClass);
		if (TimerWidgetInstance)
		{
			// ZOrder를 999로 설정하여 모든 UI 위에 무조건 띄움
			TimerWidgetInstance->AddToViewport(999);
		}
	}
}

void ADJ_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// --- [2차 시도: 공격적인 위젯 생성 체크 (FORCE)] ---
	// BeginPlay가 실행되지 않았거나 실패했을 경우, Tick에서 강제로 생성
	if (!TimerWidgetInstance && TimerWidgetClass)
	{
		TimerWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), TimerWidgetClass);
		if (TimerWidgetInstance)
		{
			// ZOrder 999로 설정하여 무조건 최상단에 띄웁니다.
			TimerWidgetInstance->AddToViewport(999);
		}
	}
	// ------------------------------------------------------


	// [시간 감소 로직]
	if (TimeLeft > 0.0f)
	{
		TimeLeft -= DeltaTime;

		// TimerWidgetInstance가 생성된 후에만 UI 업데이트 신호를 보냅니다.
		if (TimerWidgetInstance)
		{
			UpdateTimerUI(TimeLeft);
		}

		if (TimeLeft <= 0.0f)
		{
			TimeLeft = 0.0f;
			// GameOver(); 
		}
	}
}