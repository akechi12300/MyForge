#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ImageLoaderLibrary.generated.h"

UCLASS()
class MYFORGE_API UImageLoaderLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // ファイル選択ダイアログを開いて、選んだパスを返す
    UFUNCTION(BlueprintCallable, Category = "ImageLoader")
    static bool OpenImageFileDialog(FString& OutFilePath);

    // 画像ファイルパスからUTexture2Dを生成する
    UFUNCTION(BlueprintCallable, Category = "ImageLoader")
    static UTexture2D* LoadTextureFromFile(const FString& FilePath);
};