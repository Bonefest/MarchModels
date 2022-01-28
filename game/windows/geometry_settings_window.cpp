#include "utils.h"
#include "ui_utils.h"
#include "ui_styles.h"
#include "editor_utils.h"
#include "geometry_settings_window.h"

using std::string;

struct GeometrySettingsWindowData
{
  Scene* scene;
  AssetPtr geometry;
  
  bool8 positionRelativeToParent = TRUE;
  bool8 orientationRelativeToParent = TRUE;
};

static bool8 geometrySettingsWindowInitialize(Window*);
static void geometrySettingsWindowShutdown(Window*);
static void geometrySettingsWindowUpdate(Window* window, float64 delta);
static void geometrySettingsWindowDraw(Window* window, float64 delta);
static void geometrySettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender);

bool8 createGeometrySettingsWindow(Scene* scene, AssetPtr geometry, Window** outWindow)
{
  WindowInterface interface = {};
  interface.initialize = geometrySettingsWindowInitialize;
  interface.shutdown = geometrySettingsWindowShutdown;
  interface.update = geometrySettingsWindowUpdate;
  interface.draw = geometrySettingsWindowDraw;
  interface.processInput = geometrySettingsWindowProcessInput;

  if(allocateWindow(interface, geometrySettingsWindowIdentifier(geometry), outWindow) == FALSE)
  {
    return FALSE;
  }

  GeometrySettingsWindowData* data = engineAllocObject<GeometrySettingsWindowData>(MEMORY_TYPE_GENERAL);
  data->scene = scene;
  data->geometry = geometry;

  windowSetInternalData(*outWindow, data);
  
  return TRUE;
}

string geometrySettingsWindowIdentifier(Asset* geometry)
{
  char identifier[255];
  sprintf(identifier, "%s settings##%p", assetGetName(geometry).c_str(), geometry);

  return identifier;
}

bool8 geometrySettingsWindowInitialize(Window* window)
{
  return TRUE;
}

void geometrySettingsWindowShutdown(Window* window)
{
  GeometrySettingsWindowData* data = (GeometrySettingsWindowData*)windowGetInternalData(window);
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

void geometrySettingsWindowUpdate(Window* window, float64 delta)
{

}

void geometrySettingsWindowDraw(Window* window, float64 delta)
{
  GeometrySettingsWindowData* data = (GeometrySettingsWindowData*)windowGetInternalData(window);

  const char* geometryTypeLabel = geometryIsRoot(data->geometry)   ? "root" :
                                  geometryIsBranch(data->geometry) ? "branch" :
                                                                     "leaf";

  pushIconSmallButtonStyle();
  ImGui::PushStyleColor(ImGuiCol_Text, (float4)WarningClr);
    ImGui::SmallButton("[?]");
    if(ImGui::IsItemHovered())
    {
      ImGui::BeginTooltip();
      ImGui::TextColored("_<C>%010x</C>_[ID %u] _<C>%#010x</C>_[%s] _<C>%#010x</C>_'%s'_<C>0x1</C>_ was created on_<C>%#010x</C>_ 12.12.2021",
                         revbytes((uint32)InfoClr),
                         geometryGetID(data->geometry),
                         revbytes((uint32)PrimaryClr),
                         geometryTypeLabel,
                         revbytes((uint32)SecondaryClr),
                         assetGetName(data->geometry).c_str(),
                         revbytes((uint32)SecondaryClr));

      ImGui::EndTooltip();
    }
    
    ImGui::SameLine();
  ImGui::PopStyleColor();

  ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);
    ImGui::SmallButton("[Rename]");
    ImGui::SameLine();
    ImGui::SmallButton("[Export]");
    ImGui::SameLine();
    ImGui::SmallButton("[Import]");
    ImGui::SameLine();
    ImGui::SmallButton("[Load]");
    ImGui::SameLine();
    ImGui::SmallButton("[Save as]");
    ImGui::SameLine();
  ImGui::PopStyleColor();
  
  ImGui::PushStyleColor(ImGuiCol_Text, (float4)BrightDeleteClr);  
    if(ImGui::SmallButton("[Delete]"))
    {
      if(geometryHasParent(data->geometry))
      {
        geometryRemoveChild(geometryGetParent(data->geometry), data->geometry);
      }
      else
      {
        sceneRemoveGeometry(data->scene, data->geometry);
      }
      
      windowClose(window);
    }
  ImGui::PopStyleColor();
  
  popIconSmallButtonStyle();
  
  ImGui::Separator();
  
  // Position input
  float3 geometryPosition = geometryGetPosition(data->geometry);
  
  const char* relModePositionIcon = data->positionRelativeToParent == TRUE ?
    ICON_KI_USER"##PositionRelMode" : ICON_KI_USERS"##PositionRelMode";

  pushIconButtonStyle();
    if(ImGui::Button(relModePositionIcon))
    {
      data->positionRelativeToParent = !data->positionRelativeToParent;
    }

    ImGui::SameLine();

    if(ImGui::Button(ICON_KI_RELOAD_INVERSE"##ReloadPosition"))
    {
      geometryPosition = float3();
    }
  popIconButtonStyle();
    
  ImGui::SameLine();


  if(data->positionRelativeToParent == FALSE)
  {
    geometryPosition = geometryTransformToWorld(geometryGetParent(data->geometry), geometryPosition);
  }

  ImGui::SliderFloat3("Position##Geometry", &geometryPosition.x, -10.0, 10.0);

  if(data->positionRelativeToParent == FALSE)
  {
    geometryPosition = geometryTransformToLocal(geometryGetParent(data->geometry), geometryPosition);
  }
  geometrySetPosition(data->geometry, geometryPosition);

  // Orientation input
  quat geometryOrientation = geometryGetOrientation(data->geometry);
  
  const char* relModeOrientationIcon = data->orientationRelativeToParent == TRUE ?
    ICON_KI_USER"##OrientationRelMode" : ICON_KI_USERS"##OrientationRelMode";

  pushIconButtonStyle();  
    if(ImGui::Button(relModeOrientationIcon))
    {
      data->orientationRelativeToParent = !data->orientationRelativeToParent;
    }

    ImGui::SameLine();


    if(ImGui::Button(ICON_KI_RELOAD_INVERSE"##ReloadOrientation"))
    {
      geometryOrientation = quat(0.0f, 0.0f, 0.0f, 1.0f);
    }
  popIconButtonStyle();

  ImGui::SameLine();
  

  const static float32 orientationMinRange[] = {-1.0, -1.0, -1.0, -1.0};
  const static float32 orientationMaxRange[] = { 1.0,  1.0,  1.0, 1.0};

  if(data->orientationRelativeToParent == FALSE)
  {
    float3x3 qAsMat = qmat(geometryOrientation);
    float3x3 worldMat = rotor(geometryGetGeoWorldMat(geometryGetParent(data->geometry)));
    
    geometryOrientation = rotation_quat(mul(worldMat, qAsMat));
  }
  
  ImGui::SliderScalarN("Orientation",
                       ImGuiDataType_Float,
                       &geometryOrientation,
                       4,
                       orientationMinRange,
                       orientationMaxRange,
                       "%.2f",
                       ImGuiSliderFlags_MultiRange);

  if(data->orientationRelativeToParent == FALSE)
  {
    float3x3 qAsMat = qmat(geometryOrientation);
    float3x3 localMat = rotor(geometryGetWorldGeoMat(geometryGetParent(data->geometry)));
    
    geometryOrientation = rotation_quat(mul(localMat, qAsMat));
  }
  
  geometrySetOrientation(data->geometry, normalize(geometryOrientation));

  // AABB Settings
  if(ImGui::TreeNode("AABB settings"))
  {
    bool automatic = geometryAABBIsAutomaticallyCalculated(data->geometry);
    bool bounded = geometryIsBounded(data->geometry);
    AABB nativeAABB = geometryGetNativeAABB(data->geometry);
    AABB dynamicAABB = geometryGetDynamicAABB(data->geometry);
    AABB finalAABB = geometryGetFinalAABB(data->geometry);        
    
    if(ImGui::Checkbox("Automatic", &automatic))
    {
      geometrySetAABBAutomaticallyCalculated(data->geometry, automatic);
    }

    ImGui::BeginDisabled(automatic);

    ImGui::SameLine();
    if(ImGui::Checkbox("Bounded", &bounded))
    {
      geometrySetBounded(data->geometry, bounded);
    }

    ImGui::EndDisabled();
    
    ImGui::SameLine();
    if(ImGui::Button("Force recalculation"))
    {
      geometryMarkAsNeedAABBRecalculation(data->geometry);
    }

    ImGui::BeginDisabled(automatic);
    
    ImGui::Text("Native AABB");
    ImGui::SliderFloat3("Minimal##Native", &nativeAABB.min[0], -20.0, 20.0);
    ImGui::SliderFloat3("Maximal##Native", &nativeAABB.max[0], -20.0, 20.0);

    geometrySetNativeAABB(data->geometry, nativeAABB);

    ImGui::EndDisabled();

    ImGui::BeginDisabled(true);
    
    ImGui::Text("Dynamic AABB");
    ImGui::SliderFloat3("Minimal##Dynamic", &dynamicAABB.min[0], -20.0, 20.0);
    ImGui::SliderFloat3("Maximal##Dynamic", &dynamicAABB.max[0], -20.0, 20.0);

    ImGui::Text("Final AABB");
    ImGui::SliderFloat3("Minimal##Final", &finalAABB.min[0], -20.0, 20.0);
    ImGui::SliderFloat3("Maximal##Final", &finalAABB.max[0], -20.0, 20.0);
    
    ImGui::EndDisabled();
    
    ImGui::TreePop();
  }
  
  // Script functions list
  if(ImGui::TreeNode("Attached script functions"))
  {
    pushIconSmallButtonStyle();
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);
      if(geometryIsLeaf(data->geometry) && geometryHasSDF(data->geometry) == FALSE)
      {
        if(ImGui::SmallButton("[New SDF]"))
        {
          geometryAddFunction(data->geometry, createDefaultSDF());
        }

        ImGui::SameLine();
      }

      if(ImGui::SmallButton("[New IDF]"))
      {
        geometryAddFunction(data->geometry, createDefaultIDF());
      }
      ImGui::SameLine();

      if(ImGui::SmallButton("[New ODF]"))
      {
        geometryAddFunction(data->geometry, createDefaultODF());
      }

    ImGui::PopStyleColor();
    popIconSmallButtonStyle();
      
    std::vector<AssetPtr> functions = geometryGetScriptFunctions(data->geometry);
    for(AssetPtr asset: functions)
    {
      drawScriptFunctionItem(data->geometry, asset);
    }

    ImGui::TreePop();
  }
  
  // Children list switch
  bool showChildren = ImGui::TreeNode("Children");
  ImGui::SameLine();

  // Combination function
  pushIconSmallButtonStyle();

  ImGui::Text("( Combination type");
  ImGui::SameLine();
  
  ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);

    static const char* combFuncIcon[] =
    {
      ICON_KI_STAR_HALF,
      ICON_KI_STAR,
      ICON_KI_STAR_O
    };
    
    CombinationFunction combinationFunc = geometryGetCombinationFunction(data->geometry);
  
    char combFuncLabel[32];
    sprintf(combFuncLabel, "%s %s",
            combinationFunctionLabel(combinationFunc),
            combFuncIcon[(uint32)combinationFunc]);
    

    if(ImGui::SmallButton(combFuncLabel))
    {
      combinationFunc = (CombinationFunction)(((uint32)combinationFunc + 1) % (uint32)COMBINATION_COUNT);
      geometrySetCombinationFunction(data->geometry, combinationFunc);
    }

  ImGui::PopStyleColor();
  popIconSmallButtonStyle();

  ImGui::SameLine();
  ImGui::Text(")");

  // List of children  
  if(showChildren)
  {
    pushIconSmallButtonStyle();
    ImGui::PushStyleColor(ImGuiCol_Text, (float4)NewClr);
      if(ImGui::SmallButton("[New child]"))
      {
        geometryAddChild(data->geometry, createNewGeometry());
      }
    ImGui::PopStyleColor();
    popIconSmallButtonStyle();
    
    std::vector<AssetPtr>& children = geometryGetChildren(data->geometry);
    for(uint32 idx = 0; idx < children.size();)
    {
      AssetPtr child = children[idx];

      ImGui::PushID(child);
      pushIconSmallButtonStyle();

        float4 downButtonClr = (float4)SecondaryClr;
        downButtonClr.w = idx > 0 ? 1.0f : 0.1f;
        ImGui::PushStyleColor(ImGuiCol_Text, downButtonClr);

          if(ImGui::SmallButton(ICON_KI_CARET_TOP) && idx > 0)
          {
            AssetPtr prevChild = children[idx - 1];
            children[idx - 1] = child;
            children[idx] = prevChild;
          }

          ImGui::SameLine();
        
        ImGui::PopStyleColor();

        float4 topButtonClr = (float4)SecondaryClr;
        topButtonClr.w = children.size() - idx > 1 ? 1.0f : 0.1f;
        ImGui::PushStyleColor(ImGuiCol_Text, topButtonClr);

          if(ImGui::SmallButton(ICON_KI_CARET_BOTTOM) && children.size() - idx > 1)
          {
            AssetPtr nextChild = children[idx + 1];
            children[idx + 1] = child;
            children[idx] = nextChild;
          }

          ImGui::SameLine();
          
        ImGui::PopStyleColor();
          
      popIconSmallButtonStyle();

        ImGui::Text("%s", assetGetName(child).c_str());
        ImGui::SameLine();

        bool8 removeRequested = drawGeometryItemActionButtons(data->scene, child);

        if(removeRequested == TRUE)
        {
          children.erase(children.begin() + idx);
        }
        else
        {
          idx++;
        }
      ImGui::PopID();
    }

    ImGui::TreePop();
  }

  // new child
  // new sdf, new odf, new idf
}

void geometrySettingsWindowProcessInput(Window* window, const EventData& eventData, void* sender)
{

}
