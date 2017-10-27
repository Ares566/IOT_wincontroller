// подключаем библиотеку для работы с DHT11
#include <dht.h>

// Библиотека для работы с серво
#include <Servo.h>

// подключаем бибилиотеку для работы с NodeMCU как с сервером
#include <ESP8266WiFi.h>

// скорости открывания/закрывания
#define W_FAST 1024
#define W_SLOW 800

// пин с которого можно считать показания датчика DHT11
#define dht_dpin D0 

// доступ к сети WiFi
const char* ssid = "";
const char* password = "";

// WiFi сервер работает на 80 порту 
WiFiServer server(80);

// определяем объект класса DHT для работы с датчиком
dht DHT;

// определяем переменную
Servo stopper;

void setup() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  } 
  
  server.begin();

  // управляем микросхемой L293NE
  pinMode(D3, OUTPUT);
  digitalWrite(D3, 0);
  pinMode(D7, OUTPUT);
  digitalWrite(D7, 0);
  pinMode(D8, OUTPUT);
  digitalWrite(D8, 0);
  stopper.attach(D1);
  stopper.write(33);
  delay(10);
}

void loop() {
  
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // пока не появится клиент NodeMCU будет висет в данном цикле
  while(!client.available()){
    delay(1);
  }
 
  // Появился клиент/запрос к серверу был отправлен
  String request = client.readStringUntil('\r');
  client.flush();
 
  // Работа с GET запросом клиента
  // запрос может быть и пустым
  int value = LOW;
  if (request.indexOf("/WIN=OPEN") != -1)  { // запрос вида http://192.168.1.250/WIN=OPEN
    stopper.write(0);
    openWindow(W_FAST);
    delay(2000);
    stopWMoving();
    value = HIGH;
    stopper.write(33);
  }
  if (request.indexOf("/WIN=CLOSE") != -1)  {
    stopper.write(0);
    closeWindow(W_FAST);
    delay(3000);
    stopWMoving();
    value = LOW;
    stopper.write(33);
  }
 
  // Формируем ответ сервера
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<meta http-equiv=Content-Type content=\"text/html;charset=UTF-8\">");
  client.println("<style type=\"text/css\">a{text-decoration: none;width: 50%;display: block;margin: 0 auto;height: 50px;text-align: center;border-radius: 3px;background-color: orange;line-height: 54px;font-size: 26px;}</style>");
  
  client.print("ТЕМПЕРАТУРА: ");
  // считываем данные с датчика DHT11
  DHT.read11(dht_dpin);
  client.print(DHT.temperature);// температуру
  client.print("°  ВЛАЖНОСТЬ: ");
  client.print(DHT.humidity);// и влажность
  client.println("<br><br>"); 
  client.print("ОКНО : ");
  // формируем состояние
  if(value == HIGH) {
    client.print("ОТКРЫТО");
  } else {
    client.print("ЗАКРЫТО");
  }
  client.println("<br><br>"); 
  // формируем кнопки "ОТКРЫТЬ" / "ЗАКРЫТЬ"
  client.println("<a  href=\"/WIN=OPEN\"\">ОТКРЫТЬ</a>");
  client.println("<br>");
  client.println("<a href=\"/WIN=CLOSE\"\">ЗАКРЫТЬ</a><br />");  
  client.println("</html>");
 
  delay(1);
  
}

// открытие окна, крутим мотор в определенную сторону
void openWindow( uint8_t speed ) {
  analogWrite(D3, speed); 
  digitalWrite(D7, HIGH);
  digitalWrite(D8, LOW);
}

// закрытие окна, крутим в противоположную сторону
void closeWindow( uint8_t speed ) {
  analogWrite(D3, speed); 
  digitalWrite(D7, LOW);
  digitalWrite(D8, HIGH);
}

// останавливаем движение мотора - окна
void stopWMoving() {
  digitalWrite(D3, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
}
