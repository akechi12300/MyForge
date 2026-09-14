// RoundedRectTraceComponent.h
// UE5.4
//
// 「X軸・Y軸を別々に伸縮できる角丸四角柱」の形で、通常のCapsuleTraceByChannelと同じ感覚で
// 使えるトレースコンポーネント。
//
// 特徴:
// ・形だけが違うだけで、使い方はCapsuleTraceByChannelとほぼ同じ(TraceChannel/TraceComplex/
//   ActorsToIgnore/DrawDebugType などを揃えている)
// ・Start=Endでスイープする(=その場での重なりチェック)ので、動いていなくても呼べば毎回
//   その時点の状態を返す。BeginOverlapのようなイベント駆動ではなく、好きなタイミング
//   (Tickでも、何かのイベントでもOK)で呼び出して使う
// ・OutHitsで普通のFHitResult配列(ImpactPoint等込み)が取れる
// ・OutActorsで範囲内のActor配列も一緒に取れる
// ・SetShapeSize()でBPからサイズ変更可能。エディタのDetailsパネルからも変更可能

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RoundedRectTraceComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYFORGE_API URoundedRectTraceComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()

public:
	URoundedRectTraceComponent();

	/** X軸方向の半幅(角丸を含む、外形全体の半分の長さ) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoundedRect", meta = (ClampMin = "1.0"))
	float HalfExtentX = 50.f;

	/** Y軸方向の半幅(角丸を含む、外形全体の半分の長さ) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoundedRect", meta = (ClampMin = "1.0"))
	float HalfExtentY = 30.f;

	/** Z軸方向の半分の高さ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoundedRect", meta = (ClampMin = "1.0"))
	float HalfHeight = 88.f;

	/** 角の丸め半径。0にすると角の尖った普通の四角形になる */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoundedRect", meta = (ClampMin = "0.0"))
	float CornerRadius = 20.f;

	/** 角丸1つあたりの分割数(多いほど滑らかだが再生成コストが増える) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoundedRect", meta = (ClampMin = "1", ClampMax = "16"))
	int32 CornerSegments = 4;

	/**
	 * サイズを設定し、コリジョン形状(凸包)を再生成する。
	 * ★実行時にサイズを変える場合は、プロパティに直接代入するのではなく必ずこの関数を
	 *   使うこと(直接代入だけだとコリジョンが更新されない)。
	 * エディタのDetailsパネルで数値を変えた場合は自動で再生成される。
	 */
	UFUNCTION(BlueprintCallable, Category = "RoundedRect")
	void SetShapeSize(float InHalfExtentX, float InHalfExtentY, float InHalfHeight, float InCornerRadius);

	/**
	 * CapsuleTraceByChannelと同じ感覚で使えるトレース関数。
	 * このコンポーネントの現在位置・回転で、角丸四角柱の範囲に何が重なっているかを判定する。
	 * (Start=Endでスイープしているので「止まっていても」呼べば毎回その場の状態を返す)
	 *
	 * @param TraceChannel     判定に使うトレースチャンネル(Visibility, Camera, カスタムチャンネル等)
	 * @param bTraceComplex    コンプレックスコリジョンを使うか(通常のTraceByChannelと同じ意味)
	 * @param ActorsToIgnore   判定から除外するActorリスト
	 * @param bIgnoreSelf      自分(Owner)を判定から除外するか
	 * @param OutHits          ヒットしたFHitResultの配列(ImpactPoint等を含む、通常のトレースと同じ)
	 * @param OutActors        範囲内に重なっているActorの配列(重複なし)
	 * @param DrawDebugType    デバッグ描画の種類(None / ForOneFrame / ForDuration / Persistent)
	 * @param TraceColor       ヒットなし時のデバッグ描画色
	 * @param TraceHitColor    ヒットあり時のデバッグ描画色
	 * @param DrawTime         ForDuration指定時の表示時間
	 * @return ヒットが1件以上あればtrue
	 */
	UFUNCTION(BlueprintCallable, Category = "RoundedRect",
		meta = (AutoCreateRefTerm = "ActorsToIgnore", AdvancedDisplay = "DrawDebugType,TraceColor,TraceHitColor,DrawTime"))
	bool RoundedRectTraceByChannel(
		ETraceTypeQuery TraceChannel,
		bool bTraceComplex,
		const TArray<AActor*>& ActorsToIgnore,
		bool bIgnoreSelf,
		TArray<FHitResult>& OutHits,
		TArray<AActor*>& OutActors,
		EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None,
		FLinearColor TraceColor = FLinearColor::Red,
		FLinearColor TraceHitColor = FLinearColor::Green,
		float DrawTime = 0.f);

protected:
	virtual void OnRegister() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	TArray<FVector> BuildCrossSectionPoints() const;
	void RebuildCollision();
	void DrawDebugShape(bool bHit, EDrawDebugTrace::Type DrawDebugType, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime) const;
};