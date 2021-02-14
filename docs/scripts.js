// CONSTANTS & IMPORTANT VARIABLES
const url = "https://bellala.org/assets/testing.json";
const config = {
    // staticPlot: true, //makes chart static and no longer interactive, uncomment to turn them into static plots
    displayModeBar: false, // disables toolbar for aesthetics
    responsive: true // Makes plot responsive to window size
};
const layout = {

};
const batt_lay = {
    id: "battery", // the id of the html element
    // title: "Battery Capacity",
    // label: "p",
    value: 0,
    min: 0,
    max: 100,
    // decimals: 2,
    gaugeWidthScale: 0.6,
    symbol: '%',
    donut: true,
    donutStartAngle: 0,
    showInnerShadow: false,
    counter: true,
    // hideMinMax: true,
    noGradient: true,
    // gaugeColor: "#00000000", // set transparency
    // customSectors: {
    //     ranges: [{
    //             color: "#d02c06",
    //             lo: 0,
    //             hi: 10
    //         },
    //         {
    //             color: "#F4AC45",
    //             lo: 11,
    //             hi: 50
    //         },
    //         {
    //             color: "#21bf27",
    //             lo: 51,
    //             hi: 100
    //         }
    //     ]
    // },
    levelColorsGradient: false,
    levelColors: ["#d02c06", "#F4AC45", "#21bf27"]
    // relativeGaugeSize: true
};
const g_lay = {
    value: 0,
    min: 0,//set hard max or calculate max?
    gaugeWidthScale: 0.6,
    showInnerShadow: false,
    // gaugeColor: "#00000000",
    pointer: true,
    pointerOptions: {
        toplength: -15,
        bottomlength: 10,
        bottomwidth: 12,
        color: '#1f2120',
        stroke: '#ffffff',
        stroke_width: 3,
        stroke_linecap: 'round'
    },
    levelColors: ["#00bec4"],
};
// let data;
// https://github.com/bernii/gauge.js/
// https://github.com/toorshia/justgage
// data loading
function loadJSON() {
    //http request to get the json
    var http = new XMLHttpRequest();
    http.open('get', url); // get the json
    http.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) { //was I able to access the json?
            json = JSON.parse(http.responseText);
            // document.getElementById('battery').innerHTML = json.dials.s1;
            callback(json, true);
        } else {
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
    batt.refresh(0);
    g1.refresh(0);
    g2.refresh(0);
    avail.classList.add("disp");
    unavail.classList.remove("disp");
    fill_stat('st1', 'N/A');
    fill_stat('st2', 'N/A');
    for (i = 0; i < x.length; i++) {
        x[i].classList.add('placeholder');
        x[i].innerHTML = '<p class="error">' + message + '</p>';
    }
}

function isUp() {
    //inverse of isDown, needed to switch back to rendering plots if broken.
    let x = document.getElementsByClassName("plot");
    avail.classList.remove("disp");
    unavail.classList.add("disp");
    for (i = 0; i < x.length; i++) {
        x[i].classList.remove('placeholder');
        x[i].innerHTML = '';
        // x[i].classList.remove('placeholder');
    }
}


function callback(j, exist) {
    //takes in completed json data 
    data = j;
    if (exist) { // if data have data, run plot code
        isUp();
        time_bar('s1', 'live', 's1');
        time_bar('s2', 'live', 's2');
        time_bar('s3', 'live', 's3');
        batt.refresh(Math.round(Math.random()*100));
        g1.refresh(Math.round(Math.random()*10));
        g2.refresh(Math.round(Math.random()*50));
        fill_stat('st1', Math.round(Math.random()*100));
        fill_stat('st2', Math.round(Math.random()*100));
        // Plotly.newPlot('s3', [make_trace('live', 'time', 's1'), make_trace('live', 'time', 's3'), make_trace('live', 'time', 's2')], {title:'Plot of e', colorway :['#456', '#EFE','#123'],barmode:'stack'}, config);
    } else { // data ain't got no data, render is down view
        isDown();
    }
}


function make_trace(sub, x, y) {
    //returns "dict" describing x and y traces
    d = data[sub];
    var get_d = function(a, ele) { return a[ele] };
    x_d = d.map(function(a) { return a[x] });
    y_d = d.map(function(a) { return a[y] });
    return {
        x: x_d,
        y: y_d
    };
}

function time_bar(div, sub, stat) {
    //Generate a bar graph for certain subcategory sub and statistic stat
    t1 = make_trace(sub, 'time', stat);
    t1['type'] = 'bar';
    Plotly.newPlot(div, [t1], { title: 'Plot of ' + stat }, config);
}

function norm_gauge(id, max, config){
    //makes a generic gauge
    var d = config;
    d['id'] = id;
    d['max'] = max;
    return d;
}

function fill_stat(id, stat) {
    // auto-populate a <span> element with id "id" with the statistic "stat"
    document.getElementById(id).innerHTML = stat;
}