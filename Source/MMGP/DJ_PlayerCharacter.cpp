#include "DJ_PlayerCharacter.h"
#include "Blueprint/UserWidget.h"

ADJ_PlayerCharacter::ADJ_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 타이머 초기값 (60초)
	TimeLeft = 60.0f;
}

void ADJ_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	// 여기선 아무것도 안 합니다. (위젯 생성은 블루프린트가 할 예정)
}

void ADJ_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// [시간 감소 로직]
	// 시간이 남아있을 때만 작동
	if (TimeLeft > 0.0f)
	{
		TimeLeft -= DeltaTime;

		// 매 프레임마다 블루프린트에게 시간을 업데이트하라고 신호를 보냅니다.
		UpdateTimerUI(TimeLeft);

		// 시간이 0 이하로 떨어졌다면?
		if (TimeLeft <= 0.0f)
		{
			TimeLeft = 0.0f;

			// [추가됨] 여기서 블루프린트의 OnGameOver 이벤트를 발동시킵니다!
			OnGameOver();
		}
	}
}