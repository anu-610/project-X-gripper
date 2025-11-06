

#include <ESP8266WiFi.h>
const char* ssid = "projectx";
const char* password = "12345678";
WiFiServer server(80);


#define ENA_PIN D5  
#define LEFT_IN1 D1 
#define LEFT_IN2 D2  
// Right Motor
#define ENB_PIN D6 
#define RIGHT_IN3 D3 
#define RIGHT_IN4 D4 

void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting");

  // Set all motor pins as OUTPUT
  pinMode(ENA_PIN, OUTPUT);
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  pinMode(RIGHT_IN3, OUTPUT);
  pinMode(RIGHT_IN4, OUTPUT);

  // Stop motors at startup
  stopMotors();

  // Start the Wi-Fi Access Point
  Serial.print("Setting up AP: ");
  Serial.println(ssid);
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Start the web server
  server.begin();
  Serial.println("Server started. Ready for a client.");
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.println("New client connected!");
  while (client.connected() && !client.available()) {
    delay(1);
  }

  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Check the request
  if (request.indexOf("GET /F") >= 0) {
    goForward();
  } else if (request.indexOf("GET /B") >= 0) {
    goBackward();
  } else if (request.indexOf("GET /L") >= 0) {
    turnLeft();
  } else if (request.indexOf("GET /R") >= 0) {
    turnRight();
  } else if (request.indexOf("GET /S") >= 0) {
    stopMotors();
  }

 
  
  // HTTP headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  
  client.println("<!DOCTYPE html>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>NodeMCU RC Car</title>");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">");
  client.println("<style>");
  client.println("body { text-align: center; font-family: Arial, sans-serif; background-color: #f0f0f0; }");
  client.println("h1 { color: #333; }");
  client.println(".controller { display: grid; grid-template-columns: 1fr 1fr 1fr; grid-template-rows: 1fr 1fr 1fr; width: 300px; height: 300px; margin: 20px auto; }");
  client.println("button { width: 95px; height: 95px; border: none; border-radius: 15px; background-color: #4CAF50; color: white; font-size: 1.5em; font-weight: bold; cursor: pointer; user-select: none; box-shadow: 0 4px #999; }");
  client.println("button:active { background-color: #45a049; box-shadow: 0 2px #666; transform: translateY(2px); }");
  client.println(".forward { grid-column: 2; grid-row: 1; }");
  client.println(".left { grid-column: 1; grid-row: 2; }");
  client.println(".stop { grid-column: 2; grid-row: 2; background-color: #f44336; }");
  client.println(".stop:active { background-color: #d32f2f; }");
  client.println(".right { grid-column: 3; grid-row: 2; }");
  client.println(".backward { grid-column: 2; grid-row: 3; }");
  client.println(".empty { visibility: hidden; }");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<h1>RC Car Controller</h1>");
  client.println("<div class='controller'>");
  client.println("  <div class='empty'></div>");
  client.println("  <button class='forward' onmousedown=\"sendCommand('/F')\" onmouseup=\"sendCommand('/S')\" ontouchstart=\"sendCommand('/F')\" ontouchend=\"sendCommand('/S')\">&#8593;</button>");
  client.println("  <div class='empty'></div>");
  client.println("  <button class='left' onmousedown=\"sendCommand('/L')\" onmouseup=\"sendCommand('/S')\" ontouchstart=\"sendCommand('/L')\" ontouchend=\"sendCommand('/S')\">&#8592;</button>");
  client.println("  <button class='stop' onmousedown=\"sendCommand('/S')\" ontouchstart=\"sendCommand('/S')\">STOP</button>");
  client.println("  <button class='right' onmousedown=\"sendCommand('/R')\" onmouseup=\"sendCommand('/S')\" ontouchstart=\"sendCommand('/R')\" ontouchend=\"sendCommand('/S')\">&#8594;</button>");
  client.println("  <div class='empty'></div>");
  client.println("  <button class='backward' onmousedown=\"sendCommand('/B')\" onmouseup=\"sendCommand('/S')\" ontouchstart=\"sendCommand('/B')\" ontouchend=\"sendCommand('/S')\">&#8595;</button>");
  client.println("  <div class='empty'></div>");
  client.println("</div>");
  client.println("<script>");
  client.println("  function sendCommand(command) { fetch(command); }");
  client.println("</script>");
  client.println("</body>");
  client.println("</html>");

  delay(1);
  Serial.println("Client disconnected.");
}



void goForward() {
  Serial.println("Moving Forward");
  // Set direction
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, HIGH);
  digitalWrite(RIGHT_IN4, LOW);
  // Enable motors
  digitalWrite(ENA_PIN, HIGH);
  digitalWrite(ENB_PIN, HIGH);
}

void goBackward() {
  Serial.println("Moving Backward");
  // Set direction
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, HIGH);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, HIGH);
  // Enable motors
  digitalWrite(ENA_PIN, HIGH);
  digitalWrite(ENB_PIN, HIGH);
}

void turnLeft() {
  Serial.println("Turning Left");
  // Set direction (Left Back, Right Fwd)
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, HIGH);
  digitalWrite(RIGHT_IN3, HIGH);
  digitalWrite(RIGHT_IN4, LOW);
  // Enable motors
  digitalWrite(ENA_PIN, HIGH);
  digitalWrite(ENB_PIN, HIGH);
}

void turnRight() {
  Serial.println("Turning Right");
  // Set direction (Left Fwd, Right Back)
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, HIGH);
  // Enable motors
  digitalWrite(ENA_PIN, HIGH);
  digitalWrite(ENB_PIN, HIGH);
}

void stopMotors() {
  Serial.println("STOP");
  // Disable both motors.
  digitalWrite(ENA_PIN, LOW);
  digitalWrite(ENB_PIN, LOW);
  // set inputs to LOW
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, LOW);
}
