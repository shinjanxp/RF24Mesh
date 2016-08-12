
/** RF24MeshPingPairNodee.ino by shinjanxp
 *
 * This example sketch shows how to manually configure a node via RF24Mesh, and send data to the
 * master node, or to other nodes whose ID we know.
 * The nodes will refresh their network address as soon as a single write fails. This allows the
 * nodes to change position in relation to each other and the master node.
 */


#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>
//#include <printf.h>


/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(9,10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

/**
 * User Configuration: nodeID - A unique identifier for each radio. Allows addressing
 * to change dynamically with physical changes to the mesh.
 *
 * In this example, configuration takes place below, prior to uploading the sketch to the device
 * A unique value from 1-255 must be configured for each node.
 * This will be stored in EEPROM on AVR devices, so remains persistent between further uploads, loss of power, etc.
 *
 * Config needed:
 * nodeID - The ID for this node
 * otherNodeID - Node to which we want to send data. Ignore if we want to send data to master
 * role - The role for this node i.e. pingout or pongback.
 * dataStr - String message to send. Must end with 0\0
 **/
#define nodeID 2 //The ID for this node
#define otherNodeID 1 //set this to the node to which we want to send data
#define MESSAGE_MAX_SIZE 128

uint32_t displayTimer = 0;
//Choose a role for this node
//bool role= 0;//Pingout role
bool role= 1;//Pong back role

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};
char dataStr[] = {"abcdefghijklmnopqrstuvwxyzABCDEFabcdefghijklmnopqrstuvwxyzABCDEFabcdefghijklmnopqrstuvwxyzABC0\0"};
char message[MESSAGE_MAX_SIZE];
int dataSize;
void setup() {

  Serial.begin(115200);
  //printf_begin();
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  mesh.begin(97,RF24_2MBPS);
}



void loop() {

  mesh.update();
  if(role==0){//Ping out role
    // Send to the master node every 5 seconds
    if (millis() - displayTimer >= 5000) {
      dataStr[sizeof(dataStr)-3]++;
      displayTimer = millis();
  
      // Send an 'S' type message containing the string
      mesh.write(dataStr, 'S', sizeof(dataStr),otherNodeID); //remove the otherNodeID parameter, if we want to send data to the master node
      Serial.print("sent ");
      Serial.println(dataStr);
      Serial.print(" at ");
      Serial.println(displayTimer);
    }
    while (network.available()) {
      uint32_t RTT =millis();
      RF24NetworkHeader header;
      network.read(header, message, sizeof(message));
      Serial.print("Received ");
      Serial.println(message);
      Serial.print(" at ");
      Serial.println(RTT);
    }
  }
  else{//pong back role
    if(network.available()){
    RF24NetworkHeader header;
    network.peek(header);
    
    uint32_t dat=0;
    switch(header.type){
      // Display the incoming millis() values from the sensor nodes
      case 'M': network.read(header,&dat,sizeof(dat)); Serial.println(dat); break;
      case 'S': dataSize=network.read(header,message,sizeof(message));
                delay(1);
                mesh.write(message,'S',dataSize,mesh.getNodeID(header.from_node));
                Serial.print("Received ");
                Serial.print(message);
                Serial.print(" sized ");
                Serial.print(dataSize);
                Serial.print(" from ");
                Serial.println(mesh.getNodeID(header.from_node));
                break;
      default: network.read(header,0,0); Serial.println(header.type);break;
    }
  }
    
  }
}






