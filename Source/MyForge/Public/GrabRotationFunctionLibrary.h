#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GrabRotationFunctionLibrary.generated.h"

class UPhysicsHandleComponent;
class USceneComponent;

UCLASS()
class MYFORGE_API UGrabRotationFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "MyForge|Grab Rotation")
	static void RotateHeldPartQuaternion(
		USceneComponent* HeldPart,
		UPhysicsHandleComponent* PhysicsHandle,
		float MouseX,
		float MouseY,
		FVector CameraForward,
		FVector CameraRight,
		float RotationSpeed
	);

	UFUNCTION(BlueprintCallable, Category = "MyForge|Grab Rotation")
	static void EndRotate();
};