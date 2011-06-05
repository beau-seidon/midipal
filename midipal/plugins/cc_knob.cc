// Copyright 2011 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// MIDI clock generator plug-in.

#include "midipal/plugins/cc_knob.h"

#include "midi/midi.h"

#include "midipal/resources.h"
#include "midipal/ui.h"

namespace midipal { namespace plugins {

using namespace avrlib;

void CcKnob::OnLoad() {
  for (uint8_t i = 0; i < 6; ++i) {
    SetParameter(i, LoadSetting(SETTING_CC_KNOB_VALUE + i));
  }
  ui.AddPage(STR_RES_VAL, 0, 0, 127);
  ui.AddPage(STR_RES_CHN, 0, 1, 16);
  ui.AddPage(STR_RES_TYP, STR_RES_CC_, 0, 1);
  ui.AddPage(STR_RES_NUM, 0, 0, 255);
  ui.AddPage(STR_RES_MIN, 0, 0, 255);
  ui.AddPage(STR_RES_MAX, 0, 0, 255);
}

void CcKnob::OnRawMidiData(
   uint8_t status,
   uint8_t* data,
   uint8_t data_size,
   uint8_t accepted_channel) {
  Send(status, data, data_size);
}

void CcKnob::SetParameter(uint8_t key, uint8_t value) {
  uint8_t previous_value = value_;
  static_cast<uint8_t*>(&value_)[key] = value;
  if (type_ == 0) {
    // Extended range not supported by CC.
    if (min_ > 127) {
      min_ = 127;
    }
    if (max_ > 127) {
      max_ = 127;
    }
    if (number_ > 127) {
      number_ = 127;
    }
  }
  value_ = Clip(value_, min_, max_);
  if (value_ != previous_value) {
    if (type_ == 0) {
      Send3(0xb0 | (channel_ - 1), number_ & 0x7f, value_ & 0x7f);
    } else {
      Send3(0xb0 | (channel_ - 1), midi::kNrpnMsb, number_ > 127);
      Send3(0xb0 | (channel_ - 1), midi::kNrpnLsb, number_ & 0x7f);
      Send3(0xb0 | (channel_ - 1), midi::kDataEntryMsb, value_ > 127);
      Send3(0xb0 | (channel_ - 1), midi::kDataEntryLsb, value_ & 0x7f);
    }
  }
  SaveSetting(SETTING_CC_KNOB_VALUE + key, value);
}

uint8_t CcKnob::GetParameter(uint8_t key) {
  if (key == 0) {
    return Clip(value_, min_, max_);
  } else {
    return static_cast<uint8_t*>(&value_)[key];
  }
}

} }  // namespace midipal::plugins