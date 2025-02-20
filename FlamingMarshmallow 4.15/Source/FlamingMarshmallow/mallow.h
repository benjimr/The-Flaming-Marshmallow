// Fill out your copyright notice in the Description page of Project Settings.
//stamina for dashing
//face where camera is looking, maybe only for when locked on
//limit camera movement so you don't glitch through mallow
#pragma once

#include "GameFramework/Character.h"
#include "mallow.generated.h"

UCLASS()
class FLAMINGMARSHMALLOW_API Amallow : public ACharacter
{
	GENERATED_BODY()

	// Sets default values for this character's properties

	Amallow();
	
	//camera components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FollowCamera;

	//visual components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Visual, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* MallowVisual;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Visual, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* LeftEyeVis;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Visual, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* RightEyeVis;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Visual, meta = (AllowPrivateAccess = "true"))
		UParticleSystemComponent* Flames;
		
	UTextRenderComponent* Speed;


public:
	UPROPERTY(VisibleAnywhere, Category = Movement)
		float TurnRate;
	UPROPERTY(VisibleAnywhere, Category = Movement)
		float LookRate;
	UPROPERTY(VisibleAnywhere, Category = Movement)
		float MaxSpeed;
	UPROPERTY(VisibleAnywhere, Category = Movement)
		float JumpVelocity;

	bool midJump;

	bool right;
	bool left;
	bool forward;
	bool back;

	//functions for movement
	void MoveForward();
	void MoveBack();
	void MoveLeft();
	void MoveRight();
	void StopForward();
	void StopBack();
	void StopLeft();
	void StopRight();
	void IncFTime();
	void IncBTime();
	void IncRTime();
	void IncLTime();
	void ToggleFire();

	FTimerHandle ForwardTimer;
	float forwardTime;
	FTimerHandle BackTimer;
	float backTime;
	FTimerHandle LeftTimer;
	float leftTime;
	FTimerHandle RightTimer;
	float rightTime;
	float originalTime;

	FVector CheckDirection(FString Axis);
	void movementControl();
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
	void StartRun();
	void StopRun();
	void jump();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
