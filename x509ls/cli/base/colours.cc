// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/cli/base/colours.h"

#include <ncurses.h>

namespace x509ls {

Colours::Colours() {
  Init();
}

// static
const Colours& Colours::Instance() {
  static Colours singleton_;
  return singleton_;
}

// static
int Colours::Get(ColourType colour_type) {
  return Instance().GetColour(colour_type);
}

int Colours::GetColour(ColourType colour_type) const {
  map<ColourType, int>::const_iterator it = colour_map_.find(colour_type);
  if (it != colour_map_.end()) {
    return it->second;
  }

  int flags = 0;
  switch (colour_type) {
  case kColourHighlighted:
  case kColourInfoBar:
  case kColourWarning:
  case kColourRed:
  case kColourRedHighlighted:
  case kColourYellowHighlighted:
  case kColourGreenHighlighted:
  case kColourPurpleHighlighted:
    flags = A_REVERSE | A_BOLD;
    break;
  default:
    break;
  };

  return COLOR_PAIR(0) | flags;
}

void Colours::Init() {
  if (COLOR_PAIRS >= 16) {
    AddColour(kColourHighlighted, COLOR_WHITE, COLOR_BLUE);
    AddColour(kColourInfoBar, COLOR_WHITE, COLOR_BLUE, A_BOLD);
    AddColour(kColourWarning, COLOR_WHITE, COLOR_RED, A_BOLD);

    AddColour(kColourRed, COLOR_RED, COLOR_BLACK, A_BOLD);
    AddColour(kColourYellow, COLOR_YELLOW, COLOR_BLACK, A_BOLD);
    AddColour(kColourGreen, COLOR_GREEN, COLOR_BLACK, A_BOLD);
    AddColour(kColourPurple, COLOR_MAGENTA, COLOR_BLACK, A_BOLD);

    AddColour(kColourRedHighlighted, COLOR_RED, COLOR_BLUE, A_BOLD);
    AddColour(kColourYellowHighlighted, COLOR_YELLOW, COLOR_BLUE, A_BOLD);
    AddColour(kColourGreenHighlighted, COLOR_GREEN, COLOR_BLUE, A_BOLD);
    AddColour(kColourPurpleHighlighted, COLOR_MAGENTA, COLOR_BLUE, A_BOLD);
  }
}

void Colours::AddColour(ColourType colour_type,
  int foreground, int background, int flags) {
  static int colour_pair = 1;

  init_pair(colour_pair, foreground, background);

  colour_map_[colour_type] = COLOR_PAIR(colour_pair) | flags;

  colour_pair++;
}
}  // namespace x509ls

