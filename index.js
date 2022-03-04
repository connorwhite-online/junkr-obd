const five = require('johnny-five');

const board = new five.Board();

board.on("ready", () => {
    const led = new five.Led(3);
    led.blink(250);
});