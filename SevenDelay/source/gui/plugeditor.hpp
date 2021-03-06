// (c) 2019 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "public.sdk/source/vst/vstguieditor.h"

#include "waveview.hpp"

#include <unordered_map>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class PlugEditor : public VSTGUIEditor, public IControlListener, public IMouseObserver {
public:
  PlugEditor(void *controller);

  ~PlugEditor()
  {
    for (auto &ctrl : controlMap)
      if (ctrl.second) ctrl.second->forget();

    if (waveView) waveView->forget();
  }

  bool PLUGIN_API
  open(void *parent, const PlatformType &platformType = kDefaultNative) override;
  void PLUGIN_API close() override;
  void valueChanged(CControl *pControl) override;
  void updateUI(Vst::ParamID id, ParamValue normalized);
  void refreshWaveView(Vst::ParamID id);

  void onMouseEntered(CView *view, CFrame *frame) override {}
  void onMouseExited(CView *view, CFrame *frame) override {}
  CMouseEventResult
  onMouseMoved(CFrame *frame, const CPoint &where, const CButtonState &buttons) override
  {
    return kMouseEventNotHandled;
  }
  CMouseEventResult
  onMouseDown(CFrame *frame, const CPoint &where, const CButtonState &buttons) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

  void addGroupLabel(CCoord left, CCoord top, CCoord width, UTF8String name);
  void addLabel(
    CCoord left, CCoord top, CCoord width, UTF8String name, CFontDesc *font = nullptr);

  void addWaveView(const CRect &size);

  void addSplashScreen(CRect buttonRect, CRect splashRect);

  void addVSlider(
    CCoord left,
    CCoord top,
    CColor valueColor,
    UTF8String name,
    ParamID tag,
    float defaultValue,
    UTF8StringPtr tooltip = "",
    bool drawFromCenter = false);

  void addButton(
    CCoord left,
    CCoord top,
    CCoord width,
    UTF8String title,
    ParamID tag,
    int32_t style = CTextButton::kKickStyle);

  void addCheckbox(
    CCoord left,
    CCoord top,
    UTF8String title,
    ParamID tag,
    int32_t style = CCheckBox::Styles::kAutoSizeToFit);

  void addOptionMenu(
    CCoord left, CCoord top, ParamID tag, const std::vector<UTF8String> &items);

  enum class LabelPosition {
    top,
    left,
    bottom,
    right,
  };

  void addKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CColor highlightColor,
    UTF8String name,
    ParamID tag,
    float defaultValue,
    LabelPosition labelPosition = LabelPosition::bottom,
    UTF8StringPtr tooltip = "");

protected:
  void addToControlMap(Vst::ParamID id, CControl *control)
  {
    auto iter = controlMap.find(id);
    if (iter != controlMap.end()) iter->second->forget();
    control->remember();
    controlMap.insert({id, control});
  }

  std::unordered_map<Vst::ParamID, CControl *> controlMap;
  WaveView *waveView = nullptr;

  ViewRect viewRect{0, 0, 960, 330};

  CCoord fontSize = 14.0;

  CCoord frameWidth = 1.0;

  CColor colorBlack{0, 0, 0, 255};
  CColor colorWhite{255, 255, 255, 255};
  CColor colorBlue{11, 164, 241, 255};
  CColor colorOrange{252, 192, 79, 255};
  CColor colorGreen{19, 193, 54, 255};
  CColor colorFaintGray{237, 237, 237, 255};

  const CRect WaveViewSize{760.0, 170.0, 940.0, 280.0};
  const CPoint WaveViewPos{WaveViewSize.left + 1.0, WaveViewSize.top + 1.0};

  ParamValue getPlainValue(ParamID tag);
};

} // namespace Vst
} // namespace Steinberg
