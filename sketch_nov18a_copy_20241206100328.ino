  #include <ESP32Servo.h>
  #include <LiquidCrystal_I2C.h>
  #include <WiFi.h>
  #include <ESPAsyncWebServer.h>

  //wifi name and password wpa2 bhes btezbat treka ya ali
  const char* ssid = "esp32";
  const char* password = "123456789";
  AsyncWebServer server(80);

  //auth user and pass
  const char* user_auth = "akram";
  const char* pass_auth = "123";

  //auth website
  const char htmlPage_auth[] = R"rawliteral(
  <!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Login</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f4f4f9;
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
    }
    #inp {
      background-color: white;
      padding: 30px;
      border-radius: 10px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
      width: 300px;
      text-align: center;
    }
    h2 {
      color: #333;
      margin-bottom: 20px;
      font-size: 24px;
    }
    form {
      display: flex;
      flex-direction: column;
      gap: 15px;
    }
    input[type="text"],
    input[type="password"] {
      padding: 10px;
      font-size: 16px;
      border: 1px solid #ddd;
      border-radius: 5px;
      outline: none;
      transition: border-color 0.3s;
    }
    input[type="text"]:focus,
    input[type="password"]:focus {
      border-color: #007BFF;
    }
    button[type="submit"] {
      padding: 10px;
      background-color: #007BFF;
      color: white;
      border: none;
      border-radius: 5px;
      font-size: 16px;
      cursor: pointer;
      transition: background-color 0.3s;
    }
    button[type="submit"]:hover {
      background-color: #0056b3;
    }
    button[type="submit"]:active {
      background-color: #003f8a;
    }
  </style>
</head>
<body>

  <div id="inp">
    <h2>Login</h2>
    <form id="inp_form" method="post" action="/login" name="auth form">
      <input id="user" name="user" type="text" placeholder="Enter username" required>
      <input id="pass" name="pass" type="password" placeholder="Enter password" required>
      <button id="submit" type="submit">Submit</button>
    </form>
  </div>

</body>
</html>

  )rawliteral";

const char htmlPage_main[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Control Panel</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f4f4f9;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
    }

    .container {
      background-color: #fff;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
      width: 300px;
      text-align: center;
    }

    h1 {
      font-size: 24px;
      margin-bottom: 20px;
      color: #333;
    }

    form {
      display: flex;
      flex-direction: column;
    }

    input[type="text"] {
      padding: 10px;
      margin: 10px 0;
      border: 1px solid #ccc;
      border-radius: 5px;
      font-size: 14px;
    }

    button {
      padding: 10px;
      border: none;
      border-radius: 5px;
      background-color: #4CAF50;
      color: white;
      font-size: 16px;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }

    button[type="button"] {
      background-color: #ff5722;
    }

    button:hover {
      background-color: #45a049;
    }

    button[type="button"]:hover {
      background-color: #e64a19;
    }

    .toggle-btn {
      margin-top: 10px;
      font-size: 14px;
      text-decoration: underline;
      cursor: pointer;
    }

    .form-footer {
      margin-top: 15px;
      font-size: 14px;
      color: #555;
    }

  </style>
</head>
<body>
  <div class="container">
    <h1>Control Panel</h1>
    <form id="data" method="post" name="data_form" action="/submit">
      <input type="text" placeholder="Enter motor speed" id="motor_speed" name="motor_speed">
      <input type="text" placeholder="Enter motor position" id="motor_pos" name="motor_pos">
      <input type="text" placeholder="Enter mode radar or manual" id="man_or_auto" name="man_or_auto">
      <input type="text" placeholder="Enter your desired unit" id="unit_choice" name="unit_choice">
      <button type="submit">Submit</button>
    </form>
    <button type="button" onclick="toggleMode()">Switch to Manual</button>
    <div class="form-footer">
      <p>Adjust settings as per your preference.</p>
    </div>
  </div>

  <script>
    // JavaScript to toggle between auto and manual mode
    function toggleMode() {
      var modeInput = document.getElementById('man_or_auto');
      if (modeInput.value === 'auto') {
        modeInput.value = 'manual';
        document.querySelector('button[type="submit"]').textContent = 'Switch to Auto';
      } else {
        modeInput.value = 'auto';
        document.querySelector('button[type="submit"]').textContent = 'Switch to Manual';
      }
    }
  </script>
</body>
</html>


)rawliteral";

  //servox
  #define SERVO 33
  Servo servo;

  #define BATTERY 32

  //lcd 0x27
  LiquidCrystal_I2C lcd(0x27,16,2);

  //sensor
  #define TRIG 26
  #define ECHO 27

  //buzzer
  #define BUZZER 14

  //variables 
  const float speed = 0.0347;
  float distance = 1;
  int servo_delay = 300; // by percentage
  int servo_pos = 90;
  //if false then true as manual to automatic if false then not to autoamtic stays manual
  String man_to_auto = "auto";
  String unit_choice = "cm";

  void setup() {
    //servo init
    servo.attach(SERVO);

    //init wifi again (not the last time)
    WiFi.softAP(ssid, password);

    //on form reuqest
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", htmlPage_auth);
    });

    //on fomr submition
    server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request) {
      String user = "";
      String pass = "";

      if (request->hasParam("user", true)) {
        user = request->getParam("user", true)->value();
      }
      if (request->hasParam("pass", true)) {
        pass = request->getParam("pass", true)->value();
      }

      //test auth function
      if (pass == pass_auth && user == user_auth) {
        //request->send(200, "text/plain", "Login successful! Welcome " + user);
        request->redirect("/main");
      } else {
        request->send(200, "text/plain", "Login failed! Incorrect password.");
      }
    });
      if(pass == pass_auth){
        server.on("/main", HTTP_GET, [](AsyncWebServerRequest *request) {
          request->send_P(200, "text/html", htmlPage_main);
        });
      };
    server.on("/submit",HTTP_POST, [](AsyncWebServerRequest *request) {
      if(request->hasParam("motor_speed",true)) {
        servo_delay = request->getParam("motor_speed",true)->value().toInt();
        servo_delay = max(servo_delay,1);
        servo_delay = min(servo_delay,1000);
      }
      if(request->hasParam("motor_pos",true)) {
        servo_pos = request->getParam("motor_pos",true)->value().toFloat();
        servo_pos = max(servo_pos,0);
        servo_pos = min(servo_pos,180);
      }
      if(request->hasParam("man_or_auto",true)) {
        man_to_auto = request->getParam("man_or_auto",true)->value();
      }
      else{man_to_auto = "auto";}
      if(request->hasParam("unit_choice",true)) {
        unit_choice = request->getParam("unit_choice",true)->value();
        if(unit_choice != "cm" && unit_choice != "m" && unit_choice != "inch" && unit_choice != "mm"){
          unit_choice = "cm";
        }
      }
      request->redirect("/main");
    });

    //start server (abra kadabra)
    server.begin();

    //lcd init
    lcd.init();
    lcd.init();
    lcd.clear();
    lcd.backlight();

    //sensor init
    pinMode(TRIG,OUTPUT);
    pinMode(ECHO,INPUT);

    //buzzer init
    pinMode(BUZZER,OUTPUT);
  } 

  float measure_distance(){
    float distanc = 0;
    float dur = 0;
    digitalWrite(TRIG,LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG,HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG,LOW);
    dur = pulseIn(ECHO,HIGH);
    distanc = (dur*speed)*0.5;
    return distanc;
  } 

  int measure_batt(){
    int rawValue = analogRead(BATTERY);
    float voltage = (rawValue / 4095.0) * 3.3;
    float batteryPercentage = map(voltage, 3.7, 4.2, 0, 100);
    return constrain(batteryPercentage, 0, 100);
  }

  void disp(auto data,int x,int y) {
    lcd.setCursor(x,y);
    lcd.print(data);
  }

  float check_dist(float distance,String unit_choice){
      if(distance > 330) {

          digitalWrite(BUZZER,HIGH);
          disp(measure_batt(),0,1);
          disp("object too far",0,0);
          disp("or too near",0,1);
          distance = 0;
      }
      else if(distance <= 330){
        if (unit_choice == "m") {
          distance = measure_distance() *0.001;
        }
        else if (unit_choice == "mm") {
          distance = measure_distance() * 10.0;
        }
        else if (unit_choice == "inch") {
          distance = measure_distance() * 0.393701;
        }
        digitalWrite(BUZZER,LOW);
        disp(distance,0,0);
        disp(measure_batt(),0,1);
        distance = 0;
      }
      return distance;
  }

  void auto_radar(){
    for(int i = 0;i < 180;i+=10){
      if(man_to_auto == "manual")break;
      lcd.clear();
      servo.write(i);
      distance = measure_distance();
      distance = check_dist(distance,unit_choice);
      //function that calls for error
      delay(servo_delay);
    }
    for(int i = 180;i > 0;i-=10){
      if(man_to_auto == "manual")break;
      lcd.clear();
      servo.write(i);
      distance = measure_distance();
      distance = check_dist(distance);
      //function that calls for error
      delay(servo_delay);
    }
  }

  void loop() {
    if(man_to_auto == "auto"){
      auto_radar(unit_choice);
    }
    else { 
      servo.write(servo_pos);
      distance = measure_distance();
      distance= check_dist(distance);.
      // a function that calls for error
      delay(servo_delay);
    }
  }
