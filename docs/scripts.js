// CONSTANTS
const url = "https://bellala.org/assets/testing.json";
const config = {
    // staticPlot: true, //makes chart static and no longer interactive, uncomment to turn them into static plots
    displayModeBar: false, // disables toolbar for aesthetics
    responsive: true // Makes plot responsive to window size
};
const layout = {

};
// data loading
function loadJSON() {
    //http request to get the json
    var http = new XMLHttpRequest();
    http.open('get', url); // get the json
    http.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {//was I able to access the json?
            json = JSON.parse(http.responseText);
            callback(json, true);
        }else{
            json = '';
            callback(json, false);
        }
    };
    http.send();
}

function isDown() {
    // replaces plot divs w/ placeholders if called
    // Can be customized to even change the entire page.
    let x = document.getElementsByClassName("plot");
    let message = 'Error: no data available';
    for (i = 0; i < x.length; i++) {
        x[i].classList.add('placeholder');
        x[i].innerHTML = '<p>' + message + '</p>';
    }
}

function isUp() {
    //inverse of isDown, needed to switch back to rendering plots if broken.
    let x = document.getElementsByClassName("plot");
    let message = 'Error: no data available';
    for (i = 0; i < x.length; i++) {
        x[i].classList.remove('placeholder');
        x[i].innerHTML = '';
    }
}


function callback(j,exist) {
    //takes in completed json data 
    data = j;
    if (exist) { // if data have data, run plot code
        isUp();
        Plotly.newPlot('plot', [make_bar(data, 'live', 'time', 's1'), make_bar(data, 'live', 'time', 's3'), make_bar(data, 'live', 'time', 's2')], {}, config);
    } else { // data ain't got no data, render is down view
        isDown();
    }
}


function make_bar(dw, sub, x, y) {
    //returns variable describing bar chart data
    // console.log(dw);
    d = dw[sub];
    var get_d = function(a, ele) { return a[ele] };
    x_d = d.map(function(a) { return a[x] });
    y_d = d.map(function(a) { return a[y] });
    return {
        x: x_d,
        y: y_d,
        type: 'bar'
    };
}