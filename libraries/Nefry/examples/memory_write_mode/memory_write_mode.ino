#include <Nefry.h>
void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 8; i++) 
    Nefry.memory_write_mode(i*10, i);
}

void loop() {
  // put your main code here, to run repeatedly:

}
