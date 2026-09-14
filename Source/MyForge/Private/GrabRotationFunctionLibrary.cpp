#include "GrabRotationFunctionLibrary.h"

#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"


//==================================================
// 現在の状態
//==================================================

static TWeakObjectPtr<USceneComponent> GHeldPart;
static TWeakObjectPtr<UPhysicsHandleComponent> GPhysicsHandle;

static FQuat GCurrentQuat;


//==================================================
// マウス入力の蓄積
//==================================================

static float GAccumulatedYawInput = 0.0f;
static float GAccumulatedPitchInput = 0.0f;


//==================================================
// 回転角度
//==================================================

static constexpr float StepDeg = 90.0f;


//==================================================
// 回転処理
//==================================================

void UGrabRotationFunctionLibrary::RotateHeldPartQuaternion(
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


	//==================================================
	// 新しく掴んだ場合
	//==================================================

	if (GHeldPart.Get() != HeldPart)
	{
		GHeldPart = HeldPart;
		GPhysicsHandle = PhysicsHandle;


		// 掴んだ瞬間の回転を取得
		FRotator InitialRotation =
			HeldPart->GetComponentRotation();


		//==================================================
		// 各軸を90度単位へスナップ
		//==================================================

		InitialRotation.Roll =
			FMath::GridSnap(
				InitialRotation.Roll,
				StepDeg
			);

		InitialRotation.Pitch =
			FMath::GridSnap(
				InitialRotation.Pitch,
				StepDeg
			);

		InitialRotation.Yaw =
			FMath::GridSnap(
				InitialRotation.Yaw,
				StepDeg
			);


		// 現在の回転を保存
		GCurrentQuat =
			InitialRotation.Quaternion();


		// 入力蓄積をリセット
		GAccumulatedYawInput = 0.0f;
		GAccumulatedPitchInput = 0.0f;
	}


	//==================================================
	// マウス入力
	//==================================================

	const float AbsX = FMath::Abs(MouseX);
	const float AbsY = FMath::Abs(MouseY);


	//==================================================
	// 左右入力のほうが強い
	//==================================================

	if (AbsX >= AbsY)
	{
		GAccumulatedYawInput +=
			MouseX * RotationSpeed;

		// Pitch入力をリセット
		GAccumulatedPitchInput = 0.0f;


		// 90度分たまった
		if (FMath::Abs(GAccumulatedYawInput) >= StepDeg)
		{
			const float Direction =
				FMath::Sign(GAccumulatedYawInput);


			// プレイヤーの上方向
			const FVector UpAxis =
				FVector::UpVector;


			// 90度回転
			const FQuat YawQuat(
				UpAxis,
				FMath::DegreesToRadians(
					StepDeg * Direction
				)
			);


			// Yawだけ90度回転
			GCurrentQuat =
				(YawQuat * GCurrentQuat).GetNormalized();


			// 入力をリセット
			GAccumulatedYawInput = 0.0f;
		}
	}


	//==================================================
	// 上下入力のほうが強い
	//==================================================

	else
	{
		GAccumulatedPitchInput +=
			MouseY * RotationSpeed;

		// Yaw入力をリセット
		GAccumulatedYawInput = 0.0f;


		// 90度分たまった
		if (FMath::Abs(GAccumulatedPitchInput) >= StepDeg)
		{
			const float Direction =
				FMath::Sign(GAccumulatedPitchInput);


			// プレイヤーの右方向
			const FVector RightAxis =
				CameraRight.GetSafeNormal();


			// 90度回転
			const FQuat PitchQuat(
				RightAxis,
				FMath::DegreesToRadians(
					StepDeg * Direction
				)
			);


			// Pitchだけ90度回転
			GCurrentQuat =
				(PitchQuat * GCurrentQuat).GetNormalized();


			// 入力をリセット
			GAccumulatedPitchInput = 0.0f;
		}
	}


	//==================================================
	// Physics Handleへ送る
	//==================================================

	if (GPhysicsHandle.IsValid())
	{
		GPhysicsHandle->SetTargetRotation(
			GCurrentQuat.Rotator()
		);
	}
}


//==================================================
// 回転終了
//==================================================

void UGrabRotationFunctionLibrary::EndRotate()
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

	GAccumulatedYawInput = 0.0f;
	GAccumulatedPitchInput = 0.0f;

	GCurrentQuat = FQuat::Identity;
}