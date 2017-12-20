const axios = require("axios");
const notifier = require('node-notifier');
const path = require('path');



state = {
    locationForUpdates: "",
    locationBusy: false
};
function removeClassForAllElements(selector, className){
    Array.prototype.forEach.call(document.getElementsByClassName(selector), function(el) {
        el.classList.remove(className);
    });
}

let rest = {
    enterQueue: function (id) {
        let locationDom = document.getElementById(id);


        removeClassForAllElements("location", "selected");

        if(id === state.locationForUpdates){
            state.locationForUpdates = "";
            locationDom && locationDom.classList.remove("selected");
        } else {
            state.locationForUpdates = id;
            locationDom && locationDom.classList.add("selected");
        }



    },
    getState: function (success) {
        axios.get("https://6hh1k5swyd.execute-api.us-east-1.amazonaws.com/prod/can-i-go")
            .then(function (response) {
                success(response.data);
            })
            .catch(function (err) {
                console.log("Failed to fetch status: " + err);
            });
    }
};

let render = {
    updateView: function (stateObject) {
        if (stateObject) {

            stateObject.forEach(function (el) {
                let boothDom = document.getElementById(el.location + "." + el.id);
                boothDom && boothDom.classList.remove("booth-busy","booth-free");
                boothDom && boothDom.classList.add(el.status === "Closed" ? "booth-busy" : "booth-free");
            });

            let locationBusy = stateObject
              .filter(function(el) {
                return (state.locationForUpdates.length > 0) ? el.location === state.locationForUpdates : true;
            }).map(function (el) {
                return el.status === "Closed"
            }).reduce(function(accum, val) {
                return accum & val;
            });

            if(locationBusy !== state.locationBusy){
                state.locationBusy = locationBusy;
                let stateString = (locationBusy) ? "Busy" : "Available";
                notifier.notify({
                    'title': stateString + ' bathroom',
                    'message': 'Bathroom at '+ state.locationForUpdates +' just become '+stateString.toLowerCase()+'!',
                    "sound": "true",
                    "closeLabel":"Whatever...",
                    "icon": path.join(__dirname, '/../img/happy-crap-512.png')
                });
            }

            console.log("locationBusy: " + locationBusy);
        } else {
            console.log("stateObject is false");
        }
    }
};

module.exports = {
    rest: rest,
    render: render
};