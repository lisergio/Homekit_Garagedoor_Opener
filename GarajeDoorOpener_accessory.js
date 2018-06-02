var estado=0;
// MQTT Setup
var mqtt = require('mqtt');
console.log("Connecting to MQTT broker...");
var mqtt = require('mqtt');
var options = {
  port: 1883,
  host: '192.168.0.56',
  clientId: 'garajedooropener'
};
var client = mqtt.connect(options);
console.log("GarajeDoorOpener Connected to MQTT broker");
client.subscribe('garajedooropener');
var Accessory = require('../').Accessory;
var Service = require('../').Service;
var Characteristic = require('../').Characteristic;
var uuid = require('../').uuid;


var GARAGE = {
  opened: false,
  open: function() {
if (estado == 0) {
    console.log("Opening the Garage!");
    client.publish('garajedooropener', 'abrir');
    //add your code here which allows the garage to open
    GARAGE.opened = true;
    estado=1;
	}
  },
  close: function() {
if(estado ==1){
    console.log("Closing the Garage!");
    client.publish('garajedooropener', 'cerrar');
    //add your code here which allows the garage to close
    GARAGE.opened = false;
    estado=0;
	}
  },
  identify: function() {
    //add your code here which allows the garage to be identified
    console.log("Identify the Garage");
  },
  status: function(){
    //use this section to get sensor values. set the boolean FAKE_GARAGE.opened with a sensor value.
    console.log("Sensor queried!");
    //FAKE_GARAGE.opened = true/false;
  }
};

var garageUUID = uuid.generate('hap-nodejs:accessories:'+'GarageDoor');
var garage = exports.accessory = new Accessory('Garage Door', garageUUID);

// Add properties for publishing (in case we're using Core.js and not BridgedCore.js)
garage.username = "C1:4D:01:45:AA:99"; //edit this if you use Core.js
garage.pincode = "031-45-154";

garage
  .getService(Service.AccessoryInformation)
  .setCharacteristic(Characteristic.Manufacturer, "Lisergio")
  .setCharacteristic(Characteristic.Model, "0001")
  .setCharacteristic(Characteristic.SerialNumber, "LiWorPres");

garage.on('identify', function(paired, callback) {
  GARAGE.identify();
  callback();
});

garage
  .addService(Service.GarageDoorOpener, "Garage Door")
  .setCharacteristic(Characteristic.TargetDoorState, Characteristic.TargetDoorState.CLOSED) // force initial state to CLOSED
  .getCharacteristic(Characteristic.TargetDoorState)
  .on('set', function(value, callback) {

    if (value == Characteristic.TargetDoorState.CLOSED) {
      GARAGE.close();
      callback();
      garage
        .getService(Service.GarageDoorOpener)
        .setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.CLOSED);
    }
    else if (value == Characteristic.TargetDoorState.OPEN) {
      GARAGE.open();
      callback();
      garage
        .getService(Service.GarageDoorOpener)
        .setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.OPEN);
    }
  });


garage
  .getService(Service.GarageDoorOpener)
  .getCharacteristic(Characteristic.CurrentDoorState)
  .on('get', function(callback) {

    var err = null;
    GARAGE.status();

    if (GARAGE.opened) {
      console.log("Query: Is Garage Open? Yes.");
      callback(err, Characteristic.CurrentDoorState.OPEN);
    }
    else {
      console.log("Query: Is Garage Open? No.");
      callback(err, Characteristic.CurrentDoorState.CLOSED);
    }
  });

client.on('message', function(topic, message) {
    console.log(String(message));
    if(String(message) == "abierta"){
        GARAGE.opened = true;
	estado=1;
        console.log("La puerta esta abierta");
	garage
    	.getService(Service.GarageDoorOpener)
	.setCharacteristic(Characteristic.TargetDoorState, Characteristic.TargetDoorState.OPEN)
    }else if(String(message) == "cerrada"){
        GARAGE.opened = false;
	estado=0;
        console.log("La puerta esta cerrada");
	garage
    	.getService(Service.GarageDoorOpener)
	.setCharacteristic(Characteristic.TargetDoorState, Characteristic.TargetDoorState.CLOSED)
    }//else if(String(message) == "abriendo"){
//        GARAGE.opened = true;
//        console.log("La puerta se esta abriendo");
//        estado=1;
//	 garage
//        .getService(Service.GarageDoorOpener)
//        .setCharacteristic(Characteristic.TargetDoorState, Characteristic.TargetDoorState.OPEN)
//        .setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.CLOSED)
//    }else if(String(message) == "cerrando"){
//        GARAGE.opened = false;
//        console.log("La puerta se esta cerrando");
//	estado=0;
//        garage
//        .getService(Service.GarageDoorOpener)
//        .setCharacteristic(Characteristic.TargetDoorState, Characteristic.TargetDoorState.CLOSED)
//        .setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.OPEN)
//    }
});
