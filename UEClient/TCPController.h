#pragma once

#include "CoreMinimal.h"
#include "Common/TcpListener.h"
#include "GameFramework/Actor.h"
#include "TCPController.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API ATCPController: public AActor
{
	GENERATED_BODY()
private:
	
public:
	ATCPController();

protected:
	virtual void BeginPlay() override;
	
	// delegate callback for newly connected clients when Unreal itself is the TCP server.
	// bool ClientConnected(FSocket* Socket, const FIPv4Endpoint& FIPV4Endpoint);

public:

	UFUNCTION(BlueprintCallable)
	void Connect(FString ServerHostingIP);
	
	UFUNCTION(BlueprintCallable)
	void Disconnect();

	UFUNCTION(BlueprintCallable)
	FString GetClientConnectionStatus();
	
	UFUNCTION(BlueprintCallable)
	void sendMessage(FString Message);
	
	virtual void Tick(float DeltaTime) override;

	FSocket* ClientSocket; // Socket pointer acts solely on the client, used for receiving data

	// TUniquePtr<FTcpListener> ServerListener; // this runs asynchronously, is only needed if this is going to be used as a listening server and not a lonely client

	const int32 BufferSize = 4096; // Size of the pending data being sent over the client socket
	TSharedPtr<FInternetAddr> CurrentAddress;
	
	
	
};

// Documentation page for continuing client https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Sockets/FSocket

