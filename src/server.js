console.log('Lets do it');

var SerialPort = require('serialport');
var port = new SerialPort('/dev/tty-usbserial1', {
  baudRate: 57600
});