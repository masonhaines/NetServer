#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "HAL/Runnable.h"
#include "HAL/PlatformProcess.h"
// #include "Async/Async.h"
#include "Serialization/JsonSerializer.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Containers/Queue.h"
#include <atomic>

#include "TcpClient.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTaskOutput);


USTRUCT(BlueprintType)
struct FileInformation
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FString SFileName;
	
	UPROPERTY(BlueprintReadWrite)
	FString SData;
};

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
		  	ToolTip="Creates a TCP client object you can use to connect to a server, pho singleton")
		  	)
	static UTcpClient* CreateTcpClient(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "TcpClient", meta=(ToolTip="Connect to the open socket"))
	void Connect(FString ServerHostingIP, int ServerPort);
    
	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	void GetClientConnectionStatus();
    
	UFUNCTION(BlueprintCallable, Category = "TcpClient",
		meta=(ToolTip="Types: request, chat, updateName. UpdateName should only be used after connect. If type request, message should be filename")
		)
	void SendMessage(FString Message, FString Type, FString RequestID);

	UFUNCTION(BlueprintPure, Category = "TcpClient")
	TArray<FString> ReadStringFromFile(FString FilePath, bool& bWasReadSuccessful);

	UFUNCTION()
	void AddFileEntry(const FString& Filename, const FString& Data);

	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	bool GetFileData(const FString& TargetFileName, FString& OutData);

	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	void StartPollAsynchronously();
	
	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	void PollSocket();

	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	void RequestData();

	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	int GetReceivedFileNames(TArray<FString>& OutFileNames);

	UPROPERTY(BlueprintAssignable)
	FTaskOutput OnAsyncEvent;

	UPROPERTY(BlueprintAssignable)
	FTaskOutput OnAsyncEventCompleted;

	UFUNCTION(BlueprintCallable, Category = "ASyncFunctions")
	void RunAsyncwrapper();

	UFUNCTION(BlueprintCallable, Category = "TcpClient")
	TArray<FString> DataStringToArray(FString Data);




protected:

public:	// variables

	UPROPERTY(BlueprintReadWrite)
	TArray<FileInformation> ArrayOfReceivedFiles;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TcpClient")
	FString FileName;

	UPROPERTY(BlueprintReadOnly)
	bool bIsConnected;
	
private:
	FSocket* ClientSocket; // Socket pointer acts solely on the client, used for receiving data
	const int32 BufferSize = 4096; // Size of the pending data being sent over the client socket
	// TSharedPtr<FInternetAddr> CurrentAddress;
	// FString PartialJsonMessage;
	TQueue<FString, EQueueMode::Mpsc> QueuedJsonStrings;
	TArray<uint8> ReassemblyBuffer;
	std::atomic<bool> PollActive{false}; // this was really cool to learn about https://www.youtube.com/watch?v=ZQFzMfHIxng
	
	
	// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Json/Dom
	
	// TUniquePtr<FTcpListener> ServerListener; // this runs asynchronously, is only needed if this is going to be used as a listening server and not a lonely client
    
	
};
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/EAsyncExecution



// #include "TcpClient.h"
// #include "TcpClientCore.h"
//
// void UTcpClient::Connect(const FString& IP, int32 Port)
// {
// 	if (!ClientCore)
// 		ClientCore = MakeShared<FTcpClientCore>();
// 	ClientCore->Connect(IP, Port);
// }
//
// void UTcpClient::Disconnect()
// {
// 	if (ClientCore)
// 		ClientCore->Disconnect();
// }
//
// void UTcpClient::StartPolling()
// {
// 	if (ClientCore)
// 		ClientCore->PollSocket();
// }