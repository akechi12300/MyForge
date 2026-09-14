#include "ImageLoaderLibrary.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "ImageUtils.h"
#include "Framework/Application/SlateApplication.h"

bool UImageLoaderLibrary::OpenImageFileDialog(FString& OutFilePath)
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (!DesktopPlatform)
    {
        return false;
    }

    // Get window handle so the dialog appears in front
    void* ParentWindowHandle = nullptr;
    if (FSlateApplication::IsInitialized())
    {
        TSharedPtr<SWindow> ActiveWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
        if (ActiveWindow.IsValid())
        {
            ParentWindowHandle = ActiveWindow->GetNativeWindow()->GetOSWindowHandle();
        }
    }

    TArray<FString> OutFiles;
    const bool bOpened = DesktopPlatform->OpenFileDialog(
        ParentWindowHandle,
        TEXT("Select Image File"),
        TEXT(""), // default path
        TEXT(""), // default file name
        TEXT("Image Files (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg"),
        EFileDialogFlags::None,
        OutFiles
    );

    if (bOpened && OutFiles.Num() > 0)
    {
        OutFilePath = OutFiles[0];
        return true;
    }

    return false;
}

UTexture2D* UImageLoaderLibrary::LoadTextureFromFile(const FString& FilePath)
{
    if (!FPaths::FileExists(FilePath))
    {
        return nullptr;
    }

    UTexture2D* LoadedTexture = FImageUtils::ImportFileAsTexture2D(FilePath);
    return LoadedTexture;
}