#include <Nefry.h>
void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 8; i++) {
    Nefry.println(i);
    Nefry.println(Nefry.getConfValue(i));
    Nefry.println(Nefry.getConfStr(i));
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
