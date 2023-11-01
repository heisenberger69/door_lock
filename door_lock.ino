#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
using namespace std;
#include<string>
#include<vector>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);


const char* ssid = "iPhone ";
const char* wifi_password = "12345678";


class member 
{
  public:
    String name;
    int id;
    bool inside_status;

    member(const String& n, int i, bool inside) : name(n), id(i), inside_status(inside) {}


};


vector<member> members;


SoftwareSerial mySerial(4, 5);



Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

const int password = 1234;
bool isPasswordCorrect = false;


void setup() {
  Serial.begin(57600);
  
  delay(100);
    WiFi.begin(ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, handleRoot);

  server.begin();
    Serial.println("HTTP server started");


  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
}


uint8_t readnumber(void) {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void enroll() {

   Serial.println("Ready to enroll a fingerprint!");
       Serial.println("Enter Your Name");
       Serial.read();
  
       while (!Serial.available()); // Wait for input
       String temp_name = Serial.readStringUntil('\n');
       Serial.println("type the #ID you want to store this person as: ");

       int iD = readnumber();

      
  if (iD == 0) {// ID #0 not allowed, try again!
    return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(iD);

  while (!getFingerprintEnroll(iD,temp_name));
}

uint8_t getFingerprintEnroll(uint8_t id,String name) {
  int p = -1;
  Serial.print("Waiting for a valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place the same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
    member temp_member(name,id,true);
     
    members.push_back(temp_member);









  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

void enter_finger() {
  getFingerprintID();
  delay(50);  // Don't need to run this at full speed.
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

void loop() {
    server.handleClient(); // Handle incoming client requests





  int n;
  Serial.read();
  Serial.println("Enter 2 to enroll a fingerprint or 3 to verify or 4 to empty database: ");
  
  while (!Serial.available());
  n = Serial.parseInt();
  
   if (n == 2) {
    Serial.read();
    Serial.println("Enter password: ");
    while (!Serial.available()); // Wait for input
    int enteredPassword = Serial.parseInt();
  

    if (enteredPassword == password) {
      //  Serial.println("Ready to enroll a fingerprint!");
      //  Serial.println("Enter Your Name");
      //  Serial.read();
  
      //  while (!Serial.available()); // Wait for input
      //  String temp_name = Serial.readStringUntil('\n');
      //  Serial.println("type the #ID you want to store this person as: ");

      //  int iD = readnumber();

      //   member temp_member(temp_name,iD,true);
     
      //   members.push_back(temp_member);
       
       enroll();
    } 
    else {
      Serial.print(enteredPassword);
      Serial.println(" is not the correct password.");
    }
  } 

   else if (n == 3) {
    // Fingerprint verification
    Serial.println("Place your finger on the sensor...");
    while (finger.getImage() != FINGERPRINT_OK) {
    
      delay(500);
    }
    
    int fingerprintID = getFingerprintID(); // Verify the fingerprint
    
    if (fingerprintID != -1) {
      Serial.println("Fingerprint verified!");
    } else {
      Serial.println("Fingerprint not verified.");
    }
  } 
  else if(n == 4)
{
  Serial.read();
    Serial.println("Enter password: ");
    while (!Serial.available()); // Wait for input
    int enteredPassword = Serial.parseInt();
  

    if (enteredPassword == password) {
       finger.emptyDatabase();
       members.clear();
       Serial.println("database is now empty");
    } 
    else {
      Serial.print(enteredPassword);
      Serial.println(" is not the correct password.");
    }
  } 
  else {
    Serial.println("Invalid input, please try again.");
  }
}



// void handleRoot() {
//   String html = "<html><body>";
//   html += "<h1>Member Data</h1>";
//   html += "<table border='1'><tr><th>Name</th><th>ID</th><th>Inside Status</th></tr>";

//   for (const member& m : members) {
//     html += "<tr><td>" + m.name + "</td><td>" + String(m.id) + "</td><td>" + (m.inside_status ? "Inside" : "Outside") + "</td></tr>";
//   }

//   html += "</table>";
//   html += "</body></html>";

//   server.send(200, "text/html", html);
// }

void handleRoot() {
  String html = "<html><head><meta http-equiv='refresh' content='10'></head><body>"; // Refresh every 10 seconds
  html += "<h1>Member Data</h1>";
  html += "<table border='1'><tr><th>Name</th><th>ID</th><th>Inside Status</th></tr>";

  for (const member& m : members) {
    html += "<tr><td>" + m.name + "</td><td>" + String(m.id) + "</td><td>" + (m.inside_status ? "Inside" : "Outside") + "</td></tr>";
  }

  html += "</table>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}




