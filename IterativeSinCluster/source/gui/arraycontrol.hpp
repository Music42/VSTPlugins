// (c) 2019 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/vst/vsttypes.h"
#include "vstgui/vstgui.h"

namespace VSTGUI {

struct ArrayControl : public CView {
  ArrayControl(
    const CRect &size,
    std::vector<Steinberg::Vst::ParamID> id,
    std::vector<double> value,
    std::vector<double> defaultValue)
    : CView(size), id(id), value(value), defaultValue(defaultValue)
  {
  }

  void setValueAt(size_t index, double normalized)
  {
    if (index < value.size())
      value[index] = normalized < 0.0 ? 0.0 : normalized > 1.0 ? 1.0 : normalized;
  }

  std::vector<Steinberg::Vst::ParamID> id;
  std::vector<double> value;
  std::vector<double> defaultValue;
};

} // namespace VSTGUI
