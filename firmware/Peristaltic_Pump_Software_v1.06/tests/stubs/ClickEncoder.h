#ifndef CLICKENCODER_STUB_H
#define CLICKENCODER_STUB_H
// Host signature stub mirrors 0xPIT ClickEncoder Button API; no timing behavior.
class ClickEncoder {
public:
  typedef enum Button_e { Open=0, Closed, Pressed, Held, Released, Clicked, DoubleClicked } Button;
};
#endif
