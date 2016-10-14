#include <Nefry.h>
void setup() {
  
  // put your setup code here, to run once:
Nefry.getWebServer()->on("/welcome", [&]() {
   String content = F(
      "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
      "<title>Nefry Welcome</title>"
      "</head><body><div><h1>Nefry Welcome!</h1>"
      "<a href=\"/\">Back to top</a></div></body></html>");
    Nefry.getWebServer()->send(200, "text/html", content);
  });
}

void loop() {
  // put your main code here, to run repeatedly:

}
