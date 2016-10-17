#include <Nefry.h>
void setup() {
  Nefry.getWebServer()->on("/welcome", [&]() {
    String content = F(
      "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
      "<title>Nefry Welcome</title>"
      "</head><body><div><h1>Nefry Welcome!</h1>"
      "<a href=\"/\">Back to top</a></div></body></html>");
    Nefry.getWebServer()->send(200, "text/html", content);
  });
  Nefry.setIndexLink("welcomePage","/welcome");
}

void loop() {
}
