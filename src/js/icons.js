var exports = module.exports = {};

var _platform = require('os').platform();
const _imgFolder =  __dirname + "/../img/";


exports.getInitialIcon = function(){
    let trayImage;
    // Determine appropriate icon for platform
    if (_platform === 'darwin') {
        trayImage = _imgFolder + '/toilet-icon.png';
    }
    else if (_platform === 'win32') {
        trayImage = _imgFolder + '/tray.ico';
    }

    return trayImage;
};