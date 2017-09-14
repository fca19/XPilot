console.log('Lets do it');

var SerialPort = require('serialport');
var port = new SerialPort('COM8', {
  baudRate: 9600
});
port.on('data', function (data) {
  console.log('Data:', data.toString());
});