#include "OnlineHelper.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"

FString UOnlineHelper::GetGoogleAuthToken(bool& bSuccess, int32 LocalUserNum)
{
    bSuccess = false;

    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (!OSS)
        return TEXT("");

    IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
    if (!Identity.IsValid())
        return TEXT("");

    // 로그인 여부 확인
    if (Identity->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
        return TEXT("");

    FString Token;

    // UserAccount에서 AccessToken 꺼내기
    FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(LocalUserNum);
    if (UserId.IsValid())
    {
        TSharedPtr<FUserOnlineAccount> Account = Identity->GetUserAccount(*UserId);
        if (Account.IsValid())
        {
            Token = Account->GetAccessToken();
        }
    }

    if (!Token.IsEmpty())
        bSuccess = true;

    return Token;
}
