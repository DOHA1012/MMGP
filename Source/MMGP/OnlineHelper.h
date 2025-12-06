#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "OnlineHelper.generated.h"   // <= 꼭 마지막 include!

UCLASS()
class MMGP_API UOnlineHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Online|Google")
    static FString GetGoogleAuthToken(bool& bSuccess, int32 LocalUserNum = 0);
};
