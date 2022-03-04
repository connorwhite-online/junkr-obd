const five = require('johnny-five');
const board = new five.Board();

const acc = new five.Switch;
const ignition = new five.Switch;

const sensorA4 = new five.Sensor();

board.on("ready", () => {
    const led = new five.Led(3);
    led.blink(250);
});

new five.Thermometer({
    pin: 10
});

new five.Pin({
    pin: 12
});
