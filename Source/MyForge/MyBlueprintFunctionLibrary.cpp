#include "MyBlueprintFunctionLibrary.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"

static TWeakObjectPtr<USceneComponent> GHeldPart;
static TWeakObjectPtr<UPhysicsHandleComponent> GPhysicsHandle;
static FQuat GCurrentQuat;

void UMyBlueprintFunctionLibrary::RotateHeldPartQuaternion(
    USceneComponent* HeldPart,
    UPhysicsHandleComponent* PhysicsHandle,
    float MouseX,
    float MouseY,
    FVector CameraForward,
    FVector CameraRight,
    float RotationSpeed
)
{
    if (!HeldPart || !PhysicsHandle)
    {
        return;
    }

    // ’Í‚ñ‚Å‚¢‚é‘ÎÛ‚ª•Ï‚í‚Á‚½ê‡‚¾‚¯‰Šú‰»
    if (GHeldPart.Get() != HeldPart)
    {
        GHeldPart = HeldPart;
        GPhysicsHandle = PhysicsHandle;
        GCurrentQuat = HeldPart->GetComponentQuat();
    }

    // ƒvƒŒƒCƒ„[Šî€‚Ì‰ñ“]Ž²
    const FVector UpAxis = FVector::UpVector;
    const FVector RightAxis = CameraRight.GetSafeNormal();

    // ƒ}ƒEƒX“ü—Í‚ð‰ñ“]—Ê‚Ö•ÏŠ·
    const float YawRad =
        FMath::DegreesToRadians(MouseX * RotationSpeed);

    const float PitchRad =
        FMath::DegreesToRadians(MouseY * RotationSpeed);

    // Yaw / Pitch ‚Ì‰ñ“]‚ðì¬
    const FQuat YawQuat(UpAxis, YawRad);
    const FQuat PitchQuat(RightAxis, PitchRad);

    // Œ»Ý‚Ì‰ñ“]‚Ö“K—p
    GCurrentQuat =
        YawQuat *
        PitchQuat *
        GCurrentQuat;

    GCurrentQuat.Normalize();

    // Physics Handle ‚Ì–Ú•W‰ñ“]‚ÖÝ’è
    GPhysicsHandle->SetTargetRotation(
        GCurrentQuat.Rotator()
    );
}

void UMyBlueprintFunctionLibrary::EndRotate()
{
    if (GHeldPart.IsValid())
    {
        if (UPrimitiveComponent* Prim =
            Cast<UPrimitiveComponent>(GHeldPart.Get()))
        {
            Prim->SetPhysicsAngularVelocityInDegrees(
                FVector::ZeroVector
            );

            Prim->SetPhysicsLinearVelocity(
                FVector::ZeroVector
            );
        }
    }

    GHeldPart = nullptr;
    GPhysicsHandle = nullptr;
}