// RoundedRectTraceComponent.cpp

#include "RoundedRectTraceComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

URoundedRectTraceComponent::URoundedRectTraceComponent()
	: Super(FObjectInitializer::Get())
{
	PrimaryComponentTick.bCanEverTick = false;

	// レンダリングメッシュは作らないため、複雑コリジョン(=レンダリングメッシュ)ではなく
	// 単純コリジョン(=ここで追加する凸包)を判定に使うようにする。
	bUseComplexAsSimpleCollision = false;

	// 見た目は不要(判定専用コンポーネント)
	SetVisibility(false);
	SetHiddenInGame(true);
	SetCastShadow(false);

	// トレース時にRoundedRectTraceByChannel内で毎回TraceChannelに合わせて上書きするので、
	// 初期値は何でもよい
	SetCollisionProfileName(TEXT("BlockAll"));
}


void URoundedRectTraceComponent::OnRegister()
{
	Super::OnRegister();
	RebuildCollision();
}

#if WITH_EDITOR
void URoundedRectTraceComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	// Detailsパネルで数値を変えたら自動でコリジョンを作り直す
	RebuildCollision();
}
#endif

void URoundedRectTraceComponent::SetShapeSize(float InHalfExtentX, float InHalfExtentY, float InHalfHeight, float InCornerRadius)
{
	HalfExtentX = FMath::Max(InHalfExtentX, 1.f);
	HalfExtentY = FMath::Max(InHalfExtentY, 1.f);
	HalfHeight = FMath::Max(InHalfHeight, 1.f);
	CornerRadius = FMath::Clamp(InCornerRadius, 0.f, FMath::Min(HalfExtentX, HalfExtentY));

	RebuildCollision();
}

TArray<FVector> URoundedRectTraceComponent::BuildCrossSectionPoints() const
{
	TArray<FVector> Points;

	const float Cx = HalfExtentX - CornerRadius;
	const float Cy = HalfExtentY - CornerRadius;

	const FVector2D CornerCenters[4] =
	{
		FVector2D(Cx,  Cy),
		FVector2D(-Cx,  Cy),
		FVector2D(-Cx, -Cy),
		FVector2D(Cx, -Cy),
	};
	const float StartAngleDeg[4] = { 0.f, 90.f, 180.f, 270.f };

	if (CornerRadius <= KINDA_SMALL_NUMBER)
	{
		for (const FVector2D& C : CornerCenters)
		{
			Points.Add(FVector(C.X, C.Y, 0.f));
		}
		return Points;
	}

	for (int32 CornerIdx = 0; CornerIdx < 4; ++CornerIdx)
	{
		for (int32 SegIdx = 0; SegIdx <= CornerSegments; ++SegIdx)
		{
			const float AngleDeg = StartAngleDeg[CornerIdx] + (90.f * SegIdx / CornerSegments);
			const float AngleRad = FMath::DegreesToRadians(AngleDeg);
			const FVector2D Offset(FMath::Cos(AngleRad) * CornerRadius, FMath::Sin(AngleRad) * CornerRadius);
			Points.Add(FVector(CornerCenters[CornerIdx].X + Offset.X, CornerCenters[CornerIdx].Y + Offset.Y, 0.f));
		}
	}

	return Points;
}

void URoundedRectTraceComponent::RebuildCollision()
{
	ClearCollisionConvexMeshes();

	const TArray<FVector> CrossSection = BuildCrossSectionPoints();

	TArray<FVector> HullPoints;
	HullPoints.Reserve(CrossSection.Num() * 2);
	for (const FVector& P : CrossSection)
	{
		HullPoints.Add(FVector(P.X, P.Y, HalfHeight));
		HullPoints.Add(FVector(P.X, P.Y, -HalfHeight));
	}

	AddCollisionConvexMesh(HullPoints);
}

bool URoundedRectTraceComponent::RoundedRectTraceByChannel(
	ETraceTypeQuery TraceChannel,
	bool bTraceComplex,
	const TArray<AActor*>& ActorsToIgnore,
	bool bIgnoreSelf,
	TArray<FHitResult>& OutHits,
	TArray<AActor*>& OutActors,
	EDrawDebugTrace::Type DrawDebugType,
	FLinearColor TraceColor,
	FLinearColor TraceHitColor,
	float DrawTime)
{
	OutHits.Reset();
	OutActors.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// ETraceTypeQuery(BP用) -> ECollisionChannel(内部用) に変換
	const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);

	// このコンポーネント自身を「指定チャンネルに対してブロックする物体」として設定する。
	// ComponentSweepMultiは"動かす実体の持つコリジョン設定"を使う仕組みなので、
	// 普通のTraceByChannelノードの「このチャンネルとして判定する」という挙動を
	// ここで再現している。
	SetCollisionObjectType(CollisionChannel);
	SetCollisionResponseToAllChannels(ECR_Block);

	FComponentQueryParams Params(SCENE_QUERY_STAT(RoundedRectTrace));
	Params.bTraceComplex = bTraceComplex;
	Params.AddIgnoredActors(ActorsToIgnore);
	if (bIgnoreSelf && GetOwner())
	{
		Params.AddIgnoredActor(GetOwner());
	}

	const FVector Location = GetComponentLocation();
	const FQuat Rotation = GetComponentQuat();

	// Start = End でスイープ。「移動していない」状態でも、その場の重なりが
	// ヒットとしてちゃんと返ってくる(BeginOverlapのようなイベント頼みではない)。
	const bool bHit = World->ComponentSweepMulti(OutHits, this, Location, Location, Rotation, Params);

	// 重複なしでActorだけの配列も作る
	for (const FHitResult& Hit : OutHits)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			OutActors.AddUnique(HitActor);
		}
	}

	if (DrawDebugType != EDrawDebugTrace::None)
	{
		DrawDebugShape(bHit, DrawDebugType, TraceColor, TraceHitColor, DrawTime);
	}

	return bHit;
}

void URoundedRectTraceComponent::DrawDebugShape(bool bHit, EDrawDebugTrace::Type DrawDebugType,
	FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const bool bPersistent = (DrawDebugType == EDrawDebugTrace::Persistent);
	const float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : (bPersistent ? -1.f : 0.f);
	const FColor DrawColor = (bHit ? TraceHitColor : TraceColor).ToFColor(true);

	const int32 Segments = 8;
	const float InnerX = FMath::Max(HalfExtentX - CornerRadius, 0.f);
	const float InnerY = FMath::Max(HalfExtentY - CornerRadius, 0.f);

	const FVector2D CornerCenters[4] =
	{
		FVector2D(InnerX,  InnerY),
		FVector2D(-InnerX,  InnerY),
		FVector2D(-InnerX, -InnerY),
		FVector2D(InnerX, -InnerY),
	};
	const float StartAngleDeg[4] = { 0.f, 90.f, 180.f, 270.f };

	// 上面・下面それぞれのリングを作って線で結ぶ
	for (float Z : { HalfHeight, -HalfHeight })
	{
		TArray<FVector> Ring;
		for (int32 CornerIdx = 0; CornerIdx < 4; ++CornerIdx)
		{
			for (int32 SegIdx = 0; SegIdx <= Segments; ++SegIdx)
			{
				const float AngleDeg = StartAngleDeg[CornerIdx] + (90.f * SegIdx / Segments);
				const float AngleRad = FMath::DegreesToRadians(AngleDeg);
				const FVector2D Offset(FMath::Cos(AngleRad) * CornerRadius, FMath::Sin(AngleRad) * CornerRadius);
				Ring.Add(GetComponentTransform().TransformPosition(
					FVector(CornerCenters[CornerIdx].X + Offset.X, CornerCenters[CornerIdx].Y + Offset.Y, Z)));
			}
		}

		for (int32 i = 0; i < Ring.Num(); ++i)
		{
			DrawDebugLine(World, Ring[i], Ring[(i + 1) % Ring.Num()], DrawColor, bPersistent, LifeTime, 0, 2.f);
		}
	}
}
