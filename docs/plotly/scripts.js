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
        zeroline: false,
        fixedrange: true
    },
    yaxis: {
        showgrid: true,
        showline: false,
        fixedrange: true
    }
};
const section_colors = ["#d02c06", "#F4AC45", "#21bf27"];
const gen_color = "#21bf27";
const use_color = "#F4AC45"; //"#00bec4";
const batt_color = "#00bec4";
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
    levelColors: levelSectors(section_colors, [2, 3, 5]), // may need to change to make adaptive to LowBatteryThreshold settings const
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
    // levelColors: ["#00bec4"],
    labelFontColor: '#1f2120',
    valueFontColor: '#1f2120',
    relativeGaugeSize: true,

};

var DialGenMax;
var DialUseMax;
var BatteryDischargeFloor;
var LowBatteryThreshold;
var MidBatteryThreshold;
var Co2PerWh;
// var lastUpdate = {
//     'settings': null,
//     'liveData': null,
//     'dayData': null,
//     'monthData': null
// };

// let data;
// https://github.com/bernii/gauge.js/
// https://github.com/toorshia/justgage
const plot_generic = {

}
const wGen_trace = {
    fill: 'tozeroy',
    type: 'scatter',
    mode: 'lines',
    name: 'Power Generated',
    hovertemplate: '%{y:.2f} W',
    line: {
        color: gen_color,
        width: 2
    }
}
const w_lay = {
    title: 'Power Generated & Used - Daily',
    yaxis: {
        title: 'Power (W)'
    },
    xaxis: {
        // title: 'Time'
    },
    showlegend: true,
    legend: { xanchor: 'center', x: 0.5,y: 1.15, orientation: 'h',bgcolor: '#00000000', traceorder:'normal'} 

};
const wUse_trace = {
    fill: 'tonexty',
    type: 'scatter',
    mode: 'lines',
    name: 'Power Used',
    hovertemplate: '%{y:.2f} W',
    line: {
        color: use_color,
        width: 2
    }
}
const whGen_trace = {
    // fill: 'tozeroy',
    type: 'bar',
    // mode: 'lines+markers',
    name: 'Energy Generated',
    hovertemplate: '%{y:.2f} Wh',
    marker: {
        color: gen_color,
        // width: 2
    }
}
const whUse_trace = {
    // fill: 'tozeroy',
    type: 'bar',
    // mode: 'lines+markers',
    name: 'Energy Used',
    hovertemplate: '%{y:.2f} Wh',
    marker: {
        color: use_color,
        // width: 2
    }
}
const wh_lay = {
    title: 'Energy Generated & Used - Monthly',
    yaxis: {
        title: 'Energy (Wh)'
    },
    // margin:{
    //     l:0,
    //     r:0,
    //     t:0,
    //     b:0,
    //     pad: 40,
    // },
    // autosize: true,
    showlegend: true,
    legend: { xanchor: 'center', x: 0.5,y: 1.15, orientation: 'h',bgcolor: '#00000000', } 
};


const batt_trace = {
    fill: 'tozeroy',
    type: 'scatter',
    mode: 'lines',
    name: 'Battery Capacity',
    hovertemplate: '%{y:.2f} %',
    line: {
        color: batt_color,
        width: 2
    },

}
const battplot_lay = {
    title: 'Battery Capacity - Daily',
    yaxis: {
        title: 'Capacity (%)',
        // fixedrange: true,
        range: [0, 100]
    },
    showlegend: false,
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
    // var keys = Object.keys(obj);
    // var d = g_lay;
    // keys.forEach(function(val) { d[val] = obj[val] });
    // return d;
    return combineConfig(obj, g_lay);
}

function fill_stat(id, stat) {
    // auto-populate an element with id "id" with the statistic "stat"
    // basically a shorthand
    document.getElementById(id).innerHTML = stat.toLocaleString();
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
    low = Math.round(LowBatteryThreshold / 10);
    mid = Math.round(MidBatteryThreshold / 10);
    high = 10 - low - mid;
    sectors = { levelColors: levelSectors(section_colors, [low, mid, high]) };
    // batt.update();
    batt.refresh(Math.round(batt_scale(d['bat%'])));
    g1.refresh(d['WGen'], DialGenMax);
    g2.refresh(d['WUse'], DialUseMax);
    // still populate statistics with random values since ain't got no other values to play with.
    fill_stat('wh-cumu', Math.round(d.cumulativeWhGen));
    fill_stat('co2-cumu', Math.round(Co2PerWh * d.cumulativeWhGen));
    now = new Date();
    now_off = now.getTime() + (now.getTimezoneOffset() * 60 * 1000);
    // console.log(now.getTime());
    since = new Date(Math.abs(now_off - (d.time * 1000)));
    fill_stat('lastUpdate', tdiff(now_off/1000,d.time));
}

function dateSince(date) {
    // let's hope the station won't be down for more than a day.
    str = '';
    h = date.getHours();
    m = date.getMinutes();
    if (h >= 1) {
        if (h > 1) {
            str += h + ' hours, ';
        } else {
            str += h + ' hour, ';
        }
    }
    if (m > 1 || m == 0) {
        str += m + ' minutes';
    } else {
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
    fill_stat('wh-cumu', 'N/A');
    fill_stat('co2-cumu', 'N/A');
}

function call_settings(d) {
    var message = d.websiteErrorMessage;
    DialGenMax = d.DialGenMax;
    DialUseMax = d.DialUseMax;
    BatteryDischargeFloor = d.BatteryDischargeFloor;
    LowBatteryThreshold = d.LowBatteryThreshold;
    MidBatteryThreshold = d.MidBatteryThreshold;
    Co2PerWh = d.Co2PerWh;
    // lastUpdate.settings = new Date();
    if (!(message == 'null' || message =='')) {
        // avail.classList.add("hide");
        // unavail.classList.add("hide");
        fill_stat('maintain', message);
        maintain.classList.remove("disp");
    } else {
        // avail.classList.remove("hide");
        // unavail.classList.remove("hide");
        fill_stat('maintain', '');
        maintain.classList.add("disp");
    }
}

function call_plots_day(din) {
    var dout = [];
    for (var i in din) {
        // console.log(new Date(din[i].time*1000));
        dout.push(din[i]);

    }
    // console.log(dout);
    // timeBar(dout, 'WGen', 's1', self);
    power_arr = [combineConfig(makeTrace(dout, 'time', 'WUse', toDate, self), wUse_trace), combineConfig(makeTrace(dout, 'time', 'WGen', toDate, self), wGen_trace)];
    batt_arr = [combineConfig(makeTrace(dout, 'time', 'bat%', toDate, batt_scale), batt_trace)];
    Plotly.newPlot('power-daily', power_arr.reverse(), combineConfig(w_lay, layout), config);
    Plotly.newPlot('batt-daily', batt_arr, combineConfig(battplot_lay, layout), config);
    // timeLine(dout, 'WUse', 's2', self,false,'');
    // timeLine(dout, 'bat%', 's3', batt_scale,true,'tozeroy');
    // timeBar(dout, 'bat%', 's4', self);
}

function call_plots_month(din) {
    var dout = [];
    for (var i in din) {
        // console.log(new Date(din[i].time*1000));
        dout.push(din[i]);

    }
    // console.log(dout);
    // timeBar(dout, 'WGen', 's1', self);
    energy_arr = [combineConfig(makeTrace(dout, 'time', 'WhGen', toDate, self), whGen_trace), combineConfig(makeTrace(dout, 'time', 'WhUse', toDate, self), whUse_trace)];
    Plotly.newPlot('energy-monthly', energy_arr, combineConfig(wh_lay, layout), config);
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
    if (fill) {
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
    var d = Object.assign({}, cfig);
    // keys.forEach(function(val) { d[val] = obj[val] });
    for (var k in obj) {
        if (typeof obj[k] == 'object' && cfig[k] != null) {
            d[k] = combineConfig(obj[k], cfig[k]);
        } else {
            d[k] = obj[k];
        }
    }
    return d;
}
function tdiff(a,b){
    t = (a - b);
    d = (24*60*60);
    h = (60*60);
    m = 60;
    days = Math.floor(t/(d));
    hours = Math.floor((t-days*d)/h);
    minutes = Math.floor((t-days*d-hours*h)/m);
    seconds = Math.floor(t-days*d-hours*h-minutes*m);
    str = "";
    // str = days+" days, "+hours+" hours, "+minutes+" minutes,"+seconds+" seconds";
    if (t <=0){
        return "just now";
    }
    if (days > 0){
        str += days+" day";
        if (days > 1){
            str += "s";
        }
        str += ", ";
    }
    if (hours > 0){
        str += hours+" hour";
        if (hours > 1){
            str += "s";
        }
        str += ", ";
    }
    if (minutes > 0){
        str += minutes+" minute";
        if (minutes > 1){
            str += "s";
        }
        str += ", ";
    }
    if (seconds > 0){
        str += seconds+" second";
        if (seconds > 1){
            str += "s";
        }
        str += ", ";
    }
    str = str.substring(0,str.length-2);
    return str+" ago";
}