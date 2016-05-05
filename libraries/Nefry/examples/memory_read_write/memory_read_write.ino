#include<Nefry.h>
void setup() {
  Nefry.memory_write("Hello World", 1);
  Nefry.memory_write("welcome to Nefry", 2);
  Nefry.println(Nefry.memory_read(1));
  Nefry.println(Nefry.memory_read(2));
}

void loop() {
}
