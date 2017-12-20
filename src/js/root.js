const {Notification, BrowserWindow, app, Tray, Menu} = require('electron');
var Positioner = require('electron-positioner');



const Icons = require('./icons.js');
let pressed = false;
tray = null;
let menu = null;
let menuWindow = null;
let positioner;

function startApp() {

    tray = new Tray(Icons.getInitialIcon());

    //the definition of the menu window
    menuWindow = new BrowserWindow({width: 500, height: 370, frame: false, show: false});
    menuWindow.loadURL(`file://${__dirname}/../html/menu.html`);

    //todo open menu on click
    tray.on('click', function (event, bounds) {
        pressed = !pressed;
        console.log("pressed: " + pressed);

        if (pressed) {
            menuWindow.show();
            positioner = new Positioner(menuWindow);
            positioner.move('trayCenter', bounds); // Moves the window top right on the screen.
        } else {
            menuWindow.hide();
        }

    });

    tray.setToolTip('Can I go?');
}

app.on('ready', startApp);