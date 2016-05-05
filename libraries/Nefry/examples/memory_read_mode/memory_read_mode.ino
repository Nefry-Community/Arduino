#include <Nefry.h>
void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 8; i++) {
    Nefry.println(i);
    Nefry.println(Nefry.memory_read_mode(i));
    Nefry.println(Nefry.memory_read(i + 1));
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
