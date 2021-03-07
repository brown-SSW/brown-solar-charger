// CONSTANTS & IMPORTANT VARIABLES
// MADE USING FIREBASE
const base_url = 'https://brown-solar-charger-default-rtdb.firebaseio.com/';
const end_bit = '.json';
const config = {
    // staticPlot: true, //makes chart static and no longer interactive, uncomment to turn them into static plots
    displayModeBar: false, // disables toolbar for aesthetics
    responsive: true // Makes plot responsive to window size
};
const layout = {
  //  margin: {
  //   l: 0,
  //   r: 0,
  //   b: 0,
  //   t: 0,
  //   pad: 0
  // },
  hovermode: 'x unified',
  xaxis: {
    showgrid: false,
    zeroline: false
  },
  yaxis: {
    showline: false
  }
};
const section_colors = ["#d02c06", "#F4AC45", "#21bf27"];
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
    hideMinMax: true,
    noGradient: true,
    levelColorsGradient: false,
    // gaugeColor: "#00000000", // set transparency
    levelColors: levelSectors(section_colors,[2,3,5]), // may need to change to make adaptive to LowBatteryThreshold settings const
    relativeGaugeSize: true
};

function levelSectors(colors, frac) {
    var arr = [];
    for (i = 0; i < colors.length; i++) {
        for (j = 0; j < frac[i]; j++) {
            arr.push(colors[i]);
        }
    }
    return arr;
}

const g_lay = {
    value: 0,
    min: 0, //set hard max or calculate max? EDIT: get max from settings json
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
    labelFontColor: '#1f2120',
    valueFontColor: '#1f2120',
    relativeGaugeSize: true,

};

var DialGenMax;
var DialUseMax;
var BatteryDischargeFloor;
// var lastUpdate = {
//     'settings': null,
//     'liveData': null,
//     'dayData': null,
//     'monthData': null
// };

// let data;
// https://github.com/bernii/gauge.js/
// https://github.com/toorshia/justgage
const fill_generic = {

}
const wGen_trace = {
    fill: 'tozeroy',
    type: 'scatter',
    mode: 'lines+markers',
    name: 'Power Produced',
    hovertemplate: '%{y:.2f} W',
    line: {
        color: '#21bf27',
        width: 2
    }
}
const wGen_lay = {
    title: 'Power Produced',
    yaxis: {
        title: 'Power Produced (W)'
    },
    xaxis: {
        title: 'Time'
    },

};
const whGen_trace = {
    fill: 'tozeroy',
    type: 'bar',
    // mode: 'lines+markers',
    name: 'Energy Produced',
    hovertemplate: '%{y:.2f} Wh',
    marker: {
        color: '#21bf27',
        // width: 2
    }
}
const whGen_lay = {
    title: 'Energy Produced',
    yaxis: {
        title: 'Energy Produced (Wh)'
    },
    xaxis: {
        title: 'Time'
    },

};


// data loading
// ah yes creative variable names
function loadJSON(sub, cb, time) {
    /* where,
    sub is specific portion of json to load
    cb is callback function to run
    time is an object containing ideal and alt times to wait in ms
    */
    //http request to get the json
    var linky = base_url + sub + end_bit;
    var http = new XMLHttpRequest();
    http.open('get', linky); // get the json
    http.onreadystatechange = function() {
        if (this.readyState == 4) { //was I able to access the json?
            if (this.status == 200) {
                var json = JSON.parse(http.responseText);
                cb(json);
                setTimeout(loadJSON, time.up, sub, cb, time);
            } else {
                allDown();
                setTimeout(loadJSON, time.down, sub, cb, time);
            }
        }
    };
    http.send();
}

function norm_gauge(obj) {
    //makes a generic gauge w/ defined params
    // id, max are required
    var keys = Object.keys(obj);
    var d = g_lay;
    keys.forEach(function(val) { d[val] = obj[val] });
    return d;
}

function fill_stat(id, stat) {
    // auto-populate an element with id "id" with the statistic "stat"
    // basically a shorthand
    document.getElementById(id).innerHTML = stat;
}


function call_stats(d) {
    liveUp(d);
    if (d.Available) {
        availUp();
    } else {
        availDown();
    }
}

function liveUp(d) {
    // refresh values
    batt.refresh(batt_scale(d['bat%'], BatteryDischargeFloor));
    g1.refresh(d['WGen'], DialGenMax);
    g2.refresh(d['WUse'], DialUseMax);
    // still populate statistics with random values since ain't got no other values to play with.
    fill_stat('st1', Math.round(Math.random() * 100));
    fill_stat('st2', Math.round(Math.random() * 100));
    now = new Date();
    console.log(now.getTime());
    since = new Date(Math.abs(now.getTime()-(d.time*1000)));
    fill_stat('lastUpdate', dateSince(since));
}

function dateSince(date){
    // let's hope the station won't be down for more than a day.
    str = '';
    h = date.getHours();
    m = date.getMinutes();
    if (h >= 1){
        if (h > 1){
            str += h + ' hours, ';
        }else{
            str += h + ' hour, ';
        }
    }
    if (m > 1 || m == 0){
        str += m + ' minutes';
    }else{
        str += m + ' minute';
    }
    return str;
}

function availUp() {
    avail.classList.remove("disp");
    unavail.classList.add("disp");
}

function availDown() {
    avail.classList.add("disp");
    unavail.classList.remove("disp");
}

function liveDown() {
    batt.refresh(0);
    g1.refresh(0);
    g2.refresh(0);
    fill_stat('st1', 'N/A');
    fill_stat('st2', 'N/A');
}

function call_settings(d) {
    var message = d.websiteErrorMessage;
    DialGenMax = d.DialGenMax;
    DialUseMax = d.DialUseMax;
    BatteryDischargeFloor = d.BatteryDischargeFloor;
    // lastUpdate.settings = new Date();
    if (!(message == 'null')) {
        avail.classList.add("hide");
        unavail.classList.add("hide");
        fill_stat('maintain', message);
        maintain.classList.remove("disp");
    } else {
        avail.classList.remove("hide");
        unavail.classList.remove("hide");
        fill_stat('maintain', '');
        maintain.classList.add("disp");
    }
}

function call_plots_day(din) {
    var dout = [];
    for(var i in din){
        // console.log(new Date(din[i].time*1000));
        dout.push(din[i]);

    }
    // console.log(dout);
    // timeBar(dout, 'WGen', 's1', self);
    Plotly.newPlot('s1', [combineConfig(makeTrace(dout, 'time', 'WGen', toDate, self),wGen_trace)], combineConfig(wGen_lay,layout), config);
    // timeLine(dout, 'WUse', 's2', self,false,'');
    // timeLine(dout, 'bat%', 's3', batt_scale,true,'tozeroy');
    // timeBar(dout, 'bat%', 's4', self);
}
function call_plots_month(din) {
    var dout = [];
    for(var i in din){
        // console.log(new Date(din[i].time*1000));
        dout.push(din[i]);

    }
    // console.log(dout);
    // timeBar(dout, 'WGen', 's1', self);
    Plotly.newPlot('s2', [combineConfig(makeTrace(dout, 'time', 'WhGen', toDate, self),whGen_trace)], combineConfig(whGen_lay,layout), config);
    // timeLine(dout, 'WUse', 's2', self,false,'');
    // timeLine(dout, 'bat%', 's3', batt_scale,true,'tozeroy');
    // timeBar(dout, 'bat%', 's4', self);
}

function makeTrace(data, x, y, xfun, yfun) {
    //returns object describing x and y traces
    // console.log(data);
    var get_d = function(a, ele) { return a[ele] };
    x_d = data.map(function(a) { return xfun(a[x]) });
    y_d = data.map(function(a) { return yfun(a[y]) });
    return {
        x: x_d,
        y: y_d
    };
}

function toDate(epoch) {
    return new Date(epoch * 1000);
}

function self(a) {
    return a;
}

function batt_scale(val) {
    xMax = 100;
    xMin = 0;

    yMax = 100;
    yMin = BatteryDischargeFloor;

    percent = (val - yMin) / (yMax - yMin);
    outputX = percent * (xMax - xMin) + xMin;
    return outputX;
}

function timeBar(data, stat, id, yfun) {
    //Generate a bar graph for certain subcategory sub and statistic stat
    // very generic; intended only for quick testing
    t1 = makeTrace(data, 'time', stat, toDate, yfun);
    t1['type'] = 'bar';
    Plotly.newPlot(id, [t1], { title: 'Plot of ' + stat }, config);
}
function timeLine(data, stat, id, yfun, fill, fillType) {
    //Generate a bar graph for certain subcategory sub and statistic stat
    // very generic; intended only for quick testing
    t1 = makeTrace(data, 'time', stat, toDate, yfun);
    t1['type'] = 'lines';
    if (fill){
        t1['fill'] = fillType;
    }
    Plotly.newPlot(id, [t1], { title: 'Plot of ' + stat }, config);
}

function allDown() {
    // buh bye webpage
    availDown();
    liveDown();
    //TODO: replace plotly container div with something?
}

function doSomething() {
    console.log("10 seconds");
    setTimeout(doSomething, 10000);
}

// setTimeout(doSomething, 10000); // 10 second timer

function combineConfig(obj, cfig) {
    //makes a generic obj w/ defined params
    // works recursively for nested objects yay :sob:
    var keys = Object.keys(obj);
    var d = cfig;
    // keys.forEach(function(val) { d[val] = obj[val] });
    for(var k in obj){
        if(typeof obj[k] == 'object' && cfig[k] != null){
            d[k] = combineConfig(obj[k], cfig[k]);
        } else{
            d[k] = obj[k];
        }
    }
    return d;
}