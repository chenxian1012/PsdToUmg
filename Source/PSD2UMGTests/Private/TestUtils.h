#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

namespace PSD2UMGTest
{
    /** Returns the absolute path to the Tests/Sample/ directory inside this plugin. */
    FString GetSamplesDir();

    /** Returns absolute path to a sample PSD. SampleName is e.g. "Simple". */
    FString GetSamplePsdPath(const FString& SampleName);

    /** Loads and parses the matching expected.json for a sample. Asserts on failure. */
    TSharedPtr<FJsonObject> LoadExpectedJson(const FString& SampleName);
}
