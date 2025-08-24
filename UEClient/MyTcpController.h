// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/TcpListener.h"
#include "GameFramework/Actor.h"
#include "MyTcpController.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AMyTcpController : public AActor
{
	GENERATED_BODY()
	
public:	// methods
	// Sets default values for this actor's properties
	AMyTcpController();
	
	UFUNCTION(BlueprintCallable, Category = "TCP_Controller")
	void Connect(FString ServerHostingIP);
    
	UFUNCTION(BlueprintCallable, Category = "TCP_Controller")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "TCP_Controller")
	void GetClientConnectionStatus();
    
	UFUNCTION(BlueprintCallable, Category = "TCP_Controller")
	void sendMessage(FString Message);
    
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// delegate callback for newly connected clients when Unreal itself is the TCP server.
    // bool ClientConnected(FSocket* Socket, const FIPv4Endpoint& FIPV4Endpoint);

public:	// variables
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TCP_Controller")
	FString ServerMessage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TCP_Controller")
    FString ClientMessage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TCP_Controller")
	FString sender;
	
	
    FSocket* ClientSocket; // Socket pointer acts solely on the client, used for receiving data
    const int32 BufferSize = 4096; // Size of the pending data being sent over the client socket
    TSharedPtr<FInternetAddr> CurrentAddress;
    // TUniquePtr<FTcpListener> ServerListener; // this runs asynchronously, is only needed if this is going to be used as a listening server and not a lonely client
    
 
};


    // Documentation page for continuing client https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Sockets/FSocket
