#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "OnlineHelper.generated.h"   // ★ 항상 마지막 include

UCLASS()
class MMGP_API UOnlineHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 블루프린트에서 바로 쓸 수 있게
    UFUNCTION(BlueprintCallable, Category = "Online|Google")
    static FString GetGoogleAuthToken(bool& bSuccess, int32 LocalUserNum = 0);
};
