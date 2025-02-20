// Fill out your copyright notice in the Description page of Project Settings.

#include "FlamingMarshmallow.h"
#include "mallow.h"

// Sets default values

Amallow::Amallow()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//size of capsule
	GetCapsuleComponent()->InitCapsuleSize(35.0f, 35.0f);
	SetActorLocation(FVector(0,0,100));

	//creating static mesh
	MallowVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MallowVisual"));
	MallowVisual->SetupAttachment(RootComponent);

	//find the asset we want to use
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MallowVisualAsset(TEXT("/Game/marshmallowV5_Marshmallow_Body"));

	//if it found the asset position it correctly
	if (MallowVisualAsset.Succeeded())
	{
		MallowVisual->SetStaticMesh(MallowVisualAsset.Object);
		MallowVisual->SetRelativeLocation(FVector(5.0f, 0.0f, -12.0f));
	}
	
	LeftEyeVis = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftEyeVis"));
	RightEyeVis = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightEyeVis"));

	LeftEyeVis->SetupAttachment(MallowVisual);
	RightEyeVis->SetupAttachment(MallowVisual);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> LeftEyeAsset(TEXT("/Game/marshmallowV5_left_eye"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RightEyeAsset(TEXT("/Game/marshmallowV5_right_eye"));

	if (LeftEyeAsset.Succeeded())
	{
		LeftEyeVis->SetStaticMesh(LeftEyeAsset.Object);
	}

	if (RightEyeAsset.Succeeded())
	{
		RightEyeVis->SetStaticMesh(RightEyeAsset.Object);
	}
	

	//input rates
	TurnRate = 60.0f;
	LookRate = 60.0f;
	MaxSpeed = 400.0f;
	JumpVelocity = 400.0f;

	midJump = false;

	//so it doesn't rotate when the controller rotates
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	
	//move character in direction of input
	GetCharacterMovement()->bOrientRotationToMovement = true;

	//setting rotation rate
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	//setting jump velocity
	GetCharacterMovement()->JumpZVelocity = JumpVelocity;

	//setting max walking speed
	GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;

	//setting force in air
	GetCharacterMovement()->AirControl = 0.1f;


	//CAMERA
	//Init camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	//Attach camera to the root component
	CameraBoom->SetupAttachment(RootComponent);

	//Set camera boom length
	CameraBoom->TargetArmLength = 300.0f;

	//make camera rotate based on the controller
	CameraBoom->bUsePawnControlRotation = true;

	//init actual camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	//attach follow camera to socket on boom
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	//stop camera rotating with controller, so it just rotates with boom
	FollowCamera->bUsePawnControlRotation = false;

	//Particle System
	Flames = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Flames"));
	Flames->SetupAttachment(MallowVisual);
	Flames->bAutoActivate = false;
	Flames->SetRelativeLocation(FVector(0, 0, 0));
	
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FlamesAsset(TEXT("/Game/StarterContent/Particles/P_Fire.P_Fire"));

	if (FlamesAsset.Succeeded())
	{
		Flames->SetTemplate(FlamesAsset.Object);
	}

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoReceiveInput = EAutoReceiveInput::Player0;

	right = false;
	left = false;
	forward = false;
	back = false;

	originalTime = 0.05;
}

// Called when the game starts or when spawned
void Amallow::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void Amallow::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	movementControl();

}

// Called to bind functionality to input
void Amallow::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	//jumping
	PlayerInputComponent->BindAction("Jump",IE_Pressed, this,&Amallow::jump );

	//start running/stop running
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &Amallow::StartRun);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &Amallow::StopRun);


	//moving forward and right
	PlayerInputComponent->BindAction("MoveForward", IE_Pressed, this, &Amallow::MoveForward);
	PlayerInputComponent->BindAction("MoveBack", IE_Pressed, this, &Amallow::MoveBack);


	PlayerInputComponent->BindAction("MoveRight", IE_Pressed, this, &Amallow::MoveRight);
	PlayerInputComponent->BindAction("MoveLeft", IE_Pressed, this, &Amallow::MoveLeft);

	PlayerInputComponent->BindAction("MoveForward", IE_Released, this, &Amallow::StopForward);
	PlayerInputComponent->BindAction("MoveBack", IE_Released, this, &Amallow::StopBack);


	PlayerInputComponent->BindAction("MoveRight", IE_Released, this, &Amallow::StopRight);
	PlayerInputComponent->BindAction("MoveLeft", IE_Released, this, &Amallow::StopLeft);


	//turning and camera control
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &Amallow::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &Amallow::LookUpAtRate);

	PlayerInputComponent->BindAction("ToggleFire", IE_Pressed, this, &Amallow::ToggleFire);
}

void Amallow::ToggleFire()
{
	Flames->ToggleActive();
}

void Amallow::movementControl()
{
	//input functions change boolean variable and handle dashing/dodging, this functions controls normal movement
	if (left == true)
	{
		FVector Direction = CheckDirection("Y");

		AddMovementInput(Direction, -1);
	}

	if (right == true)
	{
		FVector Direction = CheckDirection("Y");

		AddMovementInput(Direction, 1);
	}

	if (forward == true)
	{
		FVector Direction = CheckDirection("X");

		AddMovementInput(Direction, 1);
	}

	if (back == true)
	{
		FVector Direction = CheckDirection("X");

		AddMovementInput(Direction, -1);
	}
}

//WILL PROBABLY MAKE THIS IN TO A NEW CLASS AND MAKE INSTANCES FOR EACH DIRECTION SO I DON"T HAVE TO REPEAT ALL THESE FUNCTIONS

//the move functions check if timer is active, if so, increase max speed/acc for dashing/dodging
void Amallow::MoveForward()
{
	if (GetWorldTimerManager().IsTimerActive(ForwardTimer) == true)
	{
		GetCharacterMovement()->MaxAcceleration = 20000000000;
		GetCharacterMovement()->MaxWalkSpeed = 5000;
	}

	forward = true;
}

//the timers are started when the button is released, after release you have 0.05 seconds to press it again to initiate dash
void Amallow::StopForward()
{
	GetWorldTimerManager().SetTimer(ForwardTimer, this, &Amallow::IncFTime, 0.05f, true);
	forward = false;
}

void Amallow::IncFTime()
{
	forwardTime -= 0.05;

	if (forwardTime < 0)
	{
		GetWorldTimerManager().ClearTimer(ForwardTimer);
		forwardTime = originalTime;
		GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;
		GetCharacterMovement()->MaxAcceleration = 2048;
	}
}

void Amallow::MoveBack()
{
	if (GetWorldTimerManager().IsTimerActive(BackTimer) == true)
	{
		GetCharacterMovement()->MaxAcceleration = 20000000000;
		GetCharacterMovement()->MaxWalkSpeed = 2500;	
	}

	back = true;
}

void Amallow::StopBack()
{
	GetWorldTimerManager().SetTimer(BackTimer, this, &Amallow::IncBTime, 0.05f, true);
	back = false;
}

void Amallow::IncBTime()
{
	backTime -= 0.05;

	if (backTime < 0)
	{
		GetWorldTimerManager().ClearTimer(BackTimer);
		backTime = originalTime;
		GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;
		GetCharacterMovement()->MaxAcceleration = 2048;
	}
}

void Amallow::MoveLeft()
{
	if (GetWorldTimerManager().IsTimerActive(LeftTimer) == true)
	{
		GetCharacterMovement()->MaxAcceleration = 20000000000;
		GetCharacterMovement()->MaxWalkSpeed = 2500;	
	}

	left = true;
}

void Amallow::StopLeft()
{
	GetWorldTimerManager().SetTimer(LeftTimer, this, &Amallow::IncLTime, 0.05f, true);
	left = false;
}

void Amallow::IncLTime()
{
	leftTime -= 0.05;

	if (leftTime < 0)
	{
		GetWorldTimerManager().ClearTimer(LeftTimer);
		leftTime = originalTime;
		GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;
		GetCharacterMovement()->MaxAcceleration = 2048;
	}
}

void Amallow::MoveRight()
{
	if (GetWorldTimerManager().IsTimerActive(RightTimer) == true)
	{
		GetCharacterMovement()->MaxAcceleration = 20000000000;
		GetCharacterMovement()->MaxWalkSpeed = 2500;
	}

	right = true;
}

void Amallow::StopRight()
{
	GetWorldTimerManager().SetTimer(RightTimer, this, &Amallow::IncRTime, 0.05f, true);
	right = false;
}

void Amallow::IncRTime()
{
	rightTime -= 0.05;

	if (rightTime < 0)
	{
		GetWorldTimerManager().ClearTimer(RightTimer);
		rightTime = originalTime;
		GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;
		GetCharacterMovement()->MaxAcceleration = 2048;
	}
}

void Amallow::jump()
{
	float Z = GetCharacterMovement()->Velocity.Z;
	float changeZ = 500;//make this relative to current Z so it always has a similar increase(currently double jumping at different levels of normal jump changes overall jump height quite a lot)

	if (midJump == true)
	{
		GetCharacterMovement()->Velocity += FVector(0, 0, changeZ);//maybe adding straight to velocity isn't the best way to do this
		midJump = false;
	}

	if (Z == 0)
	{
		Jump();
		midJump = true;
	}
}

void Amallow::StartRun()
{
	GetCharacterMovement()->MaxWalkSpeed = MaxSpeed*1.5;
}

void Amallow::StopRun()
{
	GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;
}

void Amallow::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * LookRate * GetWorld()->GetDeltaSeconds());
}

void Amallow::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * TurnRate * GetWorld()->GetDeltaSeconds());
}

FVector Amallow::CheckDirection(FString Axis)
{
	FVector Direction;

	FRotator Yaw(0, Controller->GetControlRotation().Yaw, 0);

	if (Axis == "X")
	{
		Direction = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X);
	}
	else if (Axis == "Y")
	{
		Direction = FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y);
	}

	return Direction;
}