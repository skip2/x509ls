// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_BASE_COLOURS_H_
#define X509LS_CLI_BASE_COLOURS_H_

#include <map>

#include "x509ls/base/types.h"

using std::map;

namespace x509ls {
// Singleton class to manage terminal colour pairs.
//
// Detects if the terminal can support the required number of colours. Then
// provides access to one of two palettes: The full colour version, and a
// simplified, non-colour version.
//
// Provides an accessor to obtain each colour by name. Colours returned may have
// additional attributes set, such as bold.
//
// Normal usage is simply like:
// int colour_pair = Colours::Get(Colours::kColourInfoBar);
class Colours {
 public:
  // The available colour palette.
  // kColourDefault is the default terminal colour pair.
  enum ColourType {
    kColourDefault,
    kColourHighlighted,
    kColourInfoBar,
    kColourWarning,
    kColourRed,
    kColourRedHighlighted,
    kColourYellow,
    kColourYellowHighlighted,
    kColourGreen,
    kColourGreenHighlighted,
    kColourPurple,
    kColourPurpleHighlighted
  };

  // Return the colour pair assigned to |colour_type|.
  // The most commonly used method.
  static int Get(ColourType colour_type);

  // Return the singleton instance of Colours.
  static const Colours& Instance();

  // Return the colour pair assigned to |colour_type|.
  int GetColour(ColourType colour_type) const;

 private:
  NO_COPY_AND_ASSIGN(Colours)

  // Private constructor.
  Colours();
  void Init();

  // Map |colour_type| to have ncurses |foreground|, |background| colours, and
  // set |flags| additional display attributes (such as bold).
  void AddColour(ColourType colour_type,
      int foreground, int background, int flags = 0);

  // Map of |colour_type| to combined ncurses colour & display attributes.
  map<ColourType, int> colour_map_;
};
}  // namespace x509ls

#endif  // X509LS_CLI_BASE_COLOURS_H_

