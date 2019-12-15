#include <SD.h>
#include <ESP8266WiFi.h>

// Replace with your network credentials
const char *ssid = "TP-LINK_E889BB";
const char *password = "******";

//file
File myFile;

//server adress
String post_host = "example.com";
const int  post_port  = 8001;
String  url = "/api/simple_upload/";

//format bytes
String formatBytes(unsigned int bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

void setup()
{
  //set serial port
  Serial.begin(9600);

  //test connect to wifi
  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(250);
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  //test connect to sd
  Serial.print("Initializing SD card...");
  if (!SD.begin(D0))
  {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  //read file from SD
  //define file
  myFile = SD.open("TEST.WAV", FILE_READ);


  String fileName = myFile.name();
  String fileSize = formatBytes(myFile.size());

  Serial.println();
  Serial.println("file exists");

  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  // try connect or return on fail
  if (!client.connect(post_host, post_port)) {
    Serial.println("http post connection failed");
    return;
  } else {
    //test correcting file
    if (myFile)
    {
      Serial.println("test file:ok");
      // print content length and host
      Serial.print("contentLength : ");
      Serial.println(fileSize);
      Serial.print("connecting to ");
      Serial.println(post_host);

      // We now create a URI for the request
      Serial.println("Connected to server");
      Serial.print("Requesting URL: ");
      Serial.println(url);

      // Make a HTTP request and add HTTP headers
      String boundary = "CustomizBoundarye----";
      //change with your content type
      String contentType = "audio/x-wav";
      String portString = String(post_port);
      String hostString = String(post_host);

      // prepare http post data(generally, you dont need to change anything here)
      String postHeader = "POST " + url + " HTTP/1.1\r\n";
      postHeader += "Host: " + post_host + ":80 \r\n";
      postHeader += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
      postHeader += "Accept-Charset: utf-8;\r\n";

      //my key name is "file"(name=\"file\") change with your key
      String keyHeader = "--" + boundary + "\r\n";
      keyHeader += "Content-Disposition: form-data; name=\"file\"\r\n\r\n";

      //my key name is "file" (name=\"file\") change with your key
      String requestHead = "--" + boundary + "\r\n";
      requestHead += "Content-Disposition: form-data; name=\"file\"; filename=\"" + fileName + "\"\r\n";
      requestHead += "Content-Type: " + contentType + "\r\n\r\n";

      // post tail
      String tail = "\r\n--" + boundary + "--\r\n\r\n";

      // content length
      int contentLength = keyHeader.length() + requestHead.length() + myFile.size() + tail.length();
      postHeader += "Content-Length: " + String(contentLength, DEC) + "\n\n";

      // send post header
      char charBuf0[postHeader.length() + 1];
      postHeader.toCharArray(charBuf0, postHeader.length() + 1);
      client.write(charBuf0);
      //Serial.print("send post header=");
      //Serial.println(charBuf0);

      // send key header
      char charBufKey[keyHeader.length() + 1];
      keyHeader.toCharArray(charBufKey, keyHeader.length() + 1);
      client.write(charBufKey);
      //Serial.print("send key header=");
      //Serial.println(charBufKey);

      // send request buffer
      char charBuf1[requestHead.length() + 1];
      requestHead.toCharArray(charBuf1, requestHead.length() + 1);
      client.write(charBuf1);
      //Serial.print("send request buffer=");
      //Serial.println(charBuf1);

      // send myFile:
      //this method is for upload data very fast
      //and only work in ESP software version after 2.3.0
      //if your version is lower than please update
      //esp software to last version or use bellow comment code
      client.write(myFile);
      //      // create file buffer
      //      const int bufSize = 2048;
      //      byte clientBuf[bufSize];
      //      int clientCount = 0;
      //
      //      while (myFile.available())
      //      {
      //        clientBuf [clientCount] = myFile.read ();
      //        clientCount++;
      //        if (clientCount > (bufSize - 1))
      //        {
      //          client.write((const uint8_t *)clientBuf, bufSize);
      //          clientCount = 0;
      //        }
      //      }
      //      if (clientCount > 0)
      //      {
      //        client.write((const uint8_t *)clientBuf, clientCount);
      //        //Serial.println("Sent LAST buffer");
      //      }

      // send tail
      char charBuf3[tail.length() + 1];
      tail.toCharArray(charBuf3, tail.length() + 1);
      client.write(charBuf3);
      //Serial.print(charBuf3);
    }
    else
    {
      // if the file didn't open, print an error:
      Serial.println("error opening test.WAV");
      Serial.println("Post Failure");
    }

  }

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("request sent");
  String responseHeaders = "";

  while (client.connected() ) {
    //      Serial.println("while client connected");
    String line = client.readStringUntil('\n');
    Serial.println(line);
    responseHeaders += line;
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  String line = client.readStringUntil('\n');

  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");

  //close file
  myFile.close();
}

void loop()
{}
