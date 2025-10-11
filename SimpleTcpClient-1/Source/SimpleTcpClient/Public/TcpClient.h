#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "HAL/Runnable.h"
#include "HAL/PlatformProcess.h"
#include "Async/Async.h"
#include "Serialization/JsonSerializer.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Containers/Queue.h"
#include "TcpClient.generated.h"

UCLASS(BlueprintType)
class SIMPLETCPCLIENT_API UTcpClient: public UObject
{
	GENERATED_BODY()
	
public:
	UTcpClient();

	UFUNCTION(BlueprintCallable, Category="TcpClient",
		  meta=(
		  	
		  	DisplayName="Create TCP Client Object",
		  	Keywords="Socket Connect Network",
		  	ToolTip="Creates a TCP client object you can use to connect to a server")
		  	)
	static UTcpClient* CreateTcpClient(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "TcpClient", meta=(ToolTip="Connect to the open socket"))
	void Connect(FString ServerHostingIP, int ServerPort);
    
	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	void GetClientConnectionStatus();
    
	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	void SendMessage(FString Message, FString Type);

	UFUNCTION(BlueprintPure, Category = "TcpClient")
	TArray<FString> ReadStringFromFile(FString FilePath, bool& bWasReadSuccessful);

	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	void StartPollAsynchronously();
	
	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	void PollSocket();

protected:

public:	// variables
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TcpClient")
	FString ServerMessage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TcpClient")
	FString ClientMessage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TcpClient")
	FString Sender;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TcpClient")
	FString CawfeData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TcpClient")
	FString UnrealEngineMessagingAlias;

	UPROPERTY(BlueprintReadOnly)
	bool bIsConnected;
	
private:
	FSocket* ClientSocket; // Socket pointer acts solely on the client, used for receiving data
	const int32 BufferSize = 4096; // Size of the pending data being sent over the client socket
	// TSharedPtr<FInternetAddr> CurrentAddress;
	// FString PartialJsonMessage;
	TQueue<FString, EQueueMode::Mpsc> QueuedJsonStrings;
	TArray<uint8> PartialJsonBytes;
	
	
	// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Json/Dom
	
	// TUniquePtr<FTcpListener> ServerListener; // this runs asynchronously, is only needed if this is going to be used as a listening server and not a lonely client
    
	
};
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/EAsyncExecution