#include "Action.h"

Decision Decision::play(int pos) {
  Decision ret;
  ret.type = PLAY;
  ret.pos = pos;
  return ret;
}
Decision Decision::discard(int pos) {
  Decision ret;
  ret.type = DISCARD;
  ret.pos = pos;
  return ret;
}
Decision Decision::hint_colour(int hintee, Colour colour) {
  Decision ret;
  ret.type = HINT_COLOUR;
  ret.hintee = hintee;
  ret.colour = colour;
  return ret;
}
Decision Decision::hint_value(int hintee, Value value) {
  Decision ret;
  ret.type = HINT_VALUE;
  ret.hintee = hintee;
  ret.value = value;
  return ret;
}
