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

#include "dspcore.hpp"

namespace Steinberg {
namespace SevenDelay {

float clamp(float value, float min, float max)
{
  return (value < min) ? min : (value > max) ? max : value;
}

void DSPCore::setup(double sampleRate)
{
  for (size_t i = 0; i < delay.size(); ++i)
    delay[i] = std::make_unique<DelayTypeName>(sampleRate, 1.0f, maxDelayTime);

  for (size_t i = 0; i < filter.size(); ++i)
    filter[i] = std::make_unique<FilterTypeName>(
      sampleRate, SomeDSP::SVFType::allpass, maxToneFrequency, 0.9, -3.0);

  lfoPhaseTick = 2.0 * pi / sampleRate;

  startup();
}

void DSPCore::reset()
{
  delay[0]->reset();
  delay[1]->reset();
  filter[0]->reset();
  filter[1]->reset();
  startup();
}

void DSPCore::startup()
{
  delayOut[0] = 0.0f;
  delayOut[1] = 0.0f;
  lfoPhase = param.lfoInitialPhase;
}

void DSPCore::setParameters(double tempo)
{
  auto smoothness = GlobalParameter::scaleSmoothness.map(param.smoothness);
  LinearSmoother<float>::setTime(smoothness);

  // This won't work if sync is on and tempo < 15. Up to 8 sec or 8/16 beat.
  // 15.0 is come from (60 sec per minute) * (4 beat) / (16 beat).
  Vst::ParamValue time = GlobalParameter::scaleTime.map(param.time);
  if (param.tempoSync) {
    if (time < 1.0)
      time *= 15.0 / tempo;
    else
      time = floor(2.0 * time) * 7.5 / tempo;
  }

  Vst::ParamValue offset = GlobalParameter::scaleOffset.map(param.offset);
  if (offset < 0.0) {
    interpTime[0].push(time * (1.0 + offset));
    interpTime[1].push(time);
  } else if (offset > 0.0) {
    interpTime[0].push(time);
    interpTime[1].push(time * (1.0 - offset));
  } else {
    interpTime[0].push(time);
    interpTime[1].push(time);
  }

  interpWetMix.push(param.wetMix);
  interpDryMix.push(param.dryMix);
  interpFeedback.push(param.negativeFeedback ? -param.feedback : param.feedback);
  interpLfoAmount.push(GlobalParameter::scaleLfoAmount.map(param.lfoAmount));
  interpLfoFrequency.push(GlobalParameter::scaleLfoFrequency.map(param.lfoFrequency));
  interpLfoShape.push(GlobalParameter::scaleLfoShape.map(param.lfoShape));

  float inPan = 2 * param.inPan;
  float panInL = clamp(inPan + param.inSpread - 1.0, 0.0, 1.0);
  float panInR = clamp(inPan - param.inSpread, 0.0, 1.0);
  interpPanIn[0].push(panInL);
  interpPanIn[1].push(panInR);

  float outPan = 2 * param.outPan;
  float panOutL = clamp(outPan + param.outSpread - 1.0, 0.0, 1.0);
  float panOutR = clamp(outPan - param.outSpread, 0.0, 1.0);
  interpPanOut[0].push(panOutL);
  interpPanOut[1].push(panOutR);

  interpTone.push(GlobalParameter::scaleTone.map(param.tone));
  interpToneMix.push(GlobalParameter::scaleToneMix.map(param.tone));
}

void DSPCore::process(
  const size_t length, float *in0, float *in1, float *out0, float *out1)
{
  for (int32_t i = 0; i < length; ++i) {
    auto sign = (pi < lfoPhase) - (lfoPhase < pi);
    float lfo = sign * powf(abs(sin(lfoPhase)), interpLfoShape.process());
    lfo = interpLfoAmount.process() * (lfo + 1.0f);

    delay[0]->setTime(interpTime[0].process() + lfo);
    delay[1]->setTime(interpTime[1].process() + lfo);

    const float feedback = interpFeedback.process();
    const float inL = in0[i] + feedback * delayOut[0];
    const float inR = in1[i] + feedback * delayOut[1];
    delayOut[0] = delay[0]->process(inL + interpPanIn[0].process() * (inR - inL));
    delayOut[1] = delay[1]->process(inL + interpPanIn[1].process() * (inR - inL));

    const float tone = interpTone.process();
    const float toneMix = interpToneMix.process();
    filter[0]->setCutoff(tone);
    filter[1]->setCutoff(tone);
    float filterOutL = filter[0]->process(delayOut[0]);
    float filterOutR = filter[1]->process(delayOut[1]);
    if (!isfinite(filterOutL)) {
      filterOutL = 0.0f;
      filter[0].reset();
    }
    if (!isfinite(filterOutR)) {
      filterOutR = 0.0f;
      filter[1].reset();
    }
    delayOut[0] = filterOutL + toneMix * (delayOut[0] - filterOutL);
    delayOut[1] = filterOutR + toneMix * (delayOut[1] - filterOutR);

    const float wet = interpWetMix.process();
    const float dry = interpDryMix.process();
    const float outL = wet * delayOut[0];
    const float outR = wet * delayOut[1];
    out0[i] = dry * in0[i] + outL + interpPanOut[0].process() * (outR - outL);
    out1[i] = dry * in1[i] + outL + interpPanOut[1].process() * (outR - outL);

    if (!param.lfoHold) {
      lfoPhase += interpLfoFrequency.process() * lfoPhaseTick;
      if (lfoPhase > 2.0 * pi) lfoPhase -= pi;
    }
  }
}

} // namespace SevenDelay
} // namespace Steinberg
