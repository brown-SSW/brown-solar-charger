function loadJSONOld() {
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

function isDownOld() {
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

function isUpOld() {
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


function callbackOld(j, exist) {
    //takes in completed json data 
    data = j;
    if (exist) { // if data have data, run plot code
        isUp();
        time_bar('s1', 'live', 's1');
        time_bar('s2', 'live', 's2');
        time_bar('s3', 'live', 's3');
        batt.refresh(Math.round(Math.random() * 100));
        g1.refresh(Math.round(Math.random() * 10));
        g2.refresh(Math.round(Math.random() * 50));
        fill_stat('st1', Math.round(Math.random() * 100));
        fill_stat('st2', Math.round(Math.random() * 100));
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
function norm_gaugeOld(id, max, symbol, label) {
    //makes a generic gauge
    var d = g_lay;
    d['id'] = id;
    d['max'] = max;
    d['label'] = label;
    d['symbol'] = symbol;
    return d;
}