#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyBlueprintFunctionLibrary.generated.h"

class UPhysicsHandleComponent;

/**
 * MyForgeóp âÒì]èàóùFunction Library
 */
UCLASS()
class MYFORGE_API UMyBlueprintFunctionLibrary
    : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(
        BlueprintCallable,
        Category = "MyForge|Quaternion"
    )
    static void EndRotate();

    UFUNCTION(
        BlueprintCallable,
        Category = "MyForge|Quaternion"
    )
    static void RotateHeldPartQuaternion(
        USceneComponent* HeldPart,
        UPhysicsHandleComponent* PhysicsHandle,
        float MouseX,
        float MouseY,
        FVector CameraForward,
        FVector CameraRight,
        float RotationSpeed
    );
};