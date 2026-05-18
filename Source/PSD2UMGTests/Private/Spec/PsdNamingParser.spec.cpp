#include "Misc/AutomationTest.h"
#include "Schema/PsdNamingParser.h"

// `using namespace PSD2UMG;` is scoped inside Define() — at file scope it
// would leak into other specs in the unity TU where it can clash with
// engine-side names like EBlendMode.

BEGIN_DEFINE_SPEC(FPsdNamingParserSpec, "PSD2UMG.NamingParser",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FPsdNamingParserSpec)

void FPsdNamingParserSpec::Define()
{
    using namespace PSD2UMG;
    Describe("plain name", [this]()
    {
        It("yields Image type and original name", [this]()
        {
            FParsedLayerName P;
            TestTrue("parse", FPsdNamingParser::Parse(TEXT("Logo"), P));
            TestEqual("name", P.BaseName, FString(TEXT("Logo")));
            TestEqual("type", (int)P.Type, (int)EWidgetType::Image);
        });
    });

    Describe("button states", [this]()
    {
        It("recognizes #button_normal", [this]()
        {
            FParsedLayerName P;
            FPsdNamingParser::Parse(TEXT("PlayBtn#button_normal"), P);
            TestEqual("type",  (int)P.Type, (int)EWidgetType::Button);
            TestEqual("state", (int)P.ButtonState, (int)EButtonState::Normal);
            TestEqual("name",  P.BaseName, FString(TEXT("PlayBtn")));
        });
        It("recognizes #button_hovered", [this]()
        {
            FParsedLayerName P;
            FPsdNamingParser::Parse(TEXT("PlayBtn#button_hovered"), P);
            TestEqual("state", (int)P.ButtonState, (int)EButtonState::Hovered);
        });
        It("recognizes #button_pressed", [this]()
        {
            FParsedLayerName P;
            FPsdNamingParser::Parse(TEXT("PlayBtn#button_pressed"), P);
            TestEqual("state", (int)P.ButtonState, (int)EButtonState::Pressed);
        });
    });

    Describe("nine slice", [this]()
    {
        It("parses margin args (L,R,T,B)", [this]()
        {
            FParsedLayerName P;
            FPsdNamingParser::Parse(TEXT("Panel#9slice(8,8,8,8)"), P);
            TestEqual("type",   (int)P.Type, (int)EWidgetType::Image);
            TestTrue ("nine",   P.bNineSlice);
            TestEqual("L", P.NineSliceMargin.Left,   8.0f);
            TestEqual("R", P.NineSliceMargin.Right,  8.0f);
            TestEqual("T", P.NineSliceMargin.Top,    8.0f);
            TestEqual("B", P.NineSliceMargin.Bottom, 8.0f);
        });
    });

    Describe("linkedpsd", [this]()
    {
        It("captures relative path and sets SubWidget type", [this]()
        {
            FParsedLayerName P;
            FPsdNamingParser::Parse(TEXT("Avatar#linkedpsd(Avatar.psd)"), P);
            TestEqual("type", (int)P.Type, (int)EWidgetType::SubWidget);
            TestEqual("rel",  P.LinkedPsdRelPath, FString(TEXT("Avatar.psd")));
            TestEqual("name", P.BaseName, FString(TEXT("Avatar")));
        });
    });

    Describe("multiple tags", [this]()
    {
        It("respects #vanilla after #button_normal", [this]()
        {
            FParsedLayerName P;
            FPsdNamingParser::Parse(TEXT("PlayBtn#button_normal#vanilla"), P);
            TestEqual("type", (int)P.Type, (int)EWidgetType::Button);
            TestFalse("commonui off", P.bUseCommonUI);
        });
    });

    Describe("unknown tag", [this]()
    {
        It("falls back to default Image and records a warning", [this]()
        {
            FParsedLayerName P;
            FPsdNamingParser::Parse(TEXT("X#buton"), P);
            TestEqual("type", (int)P.Type, (int)EWidgetType::Image);
            TestTrue ("warning recorded", P.Warnings.Num() > 0);
            TestEqual("name", P.BaseName, FString(TEXT("X")));
        });
    });
}
