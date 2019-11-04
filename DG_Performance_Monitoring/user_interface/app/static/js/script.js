$(document).ready(function(){
    let flag = 1;
    var max_temp_div = $('#max_temp');
    var min_temp_div = $('#min_temp');
    var average_temperature_div = $('#average_temperature');
    var max_humidity_div = $('#max_humidity');
    var min_humidity_div = $('#min_humidity');
    var average_humidity_div = $('#average_humidity');
    var max_power_div = $('#max_power');
    var min_power_div = $('#min_power');
    var average_power_div = $('#average_power');
    var total_energy_div = $('#total_energy');
    let number_of_readings = 0;
    let max_temp = -400;
    let min_temp = 1000000;
    let temp_sum = 0;
    let max_humidity = -400;
    let min_humidity = 1000000;
    let humid_sum = 0;
    let max_power = -400;
    let min_power = 1000000;
    let power_sum = 0;
    var power_send = 0;
    var time_send = 0;
    var layout1 = {
        title: {
            text:'Temperature of room',
            font: {
                family: 'Courier New, monospace',
                size: 24
            },
            xref: 'paper',
            x: 0.05,
        },
        xaxis: {
            title: {
                text: 'Time',
                font: {
                    family: 'Courier New, monospace',
                    size: 18,
                    color: '#7f7f7f'
                }
            },
        },
        yaxis: {
            title: {
                text: 'Temperature(in celcius)',
                font: {
                    family: 'Courier New, monospace',
                    size: 18,
                    color: '#7f7f7f'
                }
            }
        }
    };
    var layout2 = {
        title: {
            text:'Relative humidity in room',
            font: {
                family: 'Courier New, monospace',
                size: 24
            },
            xref: 'paper',
            x: 0.05,
        },
        xaxis: {
            title: {
                text: 'Time',
                font: {
                    family: 'Courier New, monospace',
                    size: 18,
                    color: '#7f7f7f'
                }
            },
        },
        yaxis: {
            title: {
                text: 'Relative humidity',
                font: {
                    family: 'Courier New, monospace',
                    size: 18,
                    color: '#7f7f7f'
                }
            }
        }
    };
    function update_graphs() {
        $.ajax({
            url: '_getReadings',

            method: 'POST',

            data: {
                "all": flag,
                "power": power_send,
                "time": time_send
            },

            success: function(response){
                arr_temp_c = [];
                arr_humidity = [];
                arr_power = [];
                arr_temp_c_x = [];
                arr_humidity_x = [];
                arr_power_x = [];
                for(var i = 0; i < response.Sensor.length; i++){
                    var temp_cur = response.Sensor[i].temp_c;
                    var humidity_cur = response.Sensor[i].humidity;
                    var power_cur = response.Meter[i].power;
                    arr_temp_c.push(temp_cur);
                    arr_temp_c_x.push(response.Sensor[i].time);
                    arr_humidity_x.push(response.Sensor[i].time);
                    arr_humidity.push(humidity_cur);
                    arr_power.push(power_cur);
                    arr_power_x.push(response.Meter[i].time);
                    number_of_readings++;
                    temp_sum = temp_sum + temp_cur;
                    humid_sum = humid_sum + humidity_cur;
                    power_sum = power_sum + power_cur;
                    power_send = response.Meter[i].power;
                    time_send = response.Meter[i].time;
                    if(max_temp < temp_cur) {
                        max_temp = temp_cur;
                        max_temp_div.html(max_temp);
                    }
                    if(min_temp > temp_cur) {
                        min_temp = temp_cur;
                        min_temp_div.html(min_temp);
                    }
                    if(max_humidity < humidity_cur) {
                        max_humidity = humidity_cur;
                        max_humidity_div.html(max_humidity);
                    }
                    if(min_humidity > humidity_cur) {
                        min_humidity = humidity_cur;
                        min_humidity_div.html(min_humidity);
                    }
                    if(max_power < power_cur) {
                        max_power = power_cur;
                        max_power_div.html(max_power);
                    }
                    if(min_power > power_cur) {
                        min_power = power_cur;
                        min_power_div.html(min_power);
                    }
                    console.log(response.Energy)
                    total_energy_div.html(response.Energy);
                }
                average_temperature_div.html(temp_sum/number_of_readings);
                average_humidity_div.html(humid_sum/number_of_readings);
                average_power_div.html(power_sum/number_of_readings);
                if(flag == 1) {

                    var trace1 = {
                        y: arr_temp_c,
                        x: arr_temp_c_x,
                        type: 'line'
                    }
                    var data1 = [trace1];
                    var trace2 = {
                        y: arr_humidity,
                        x: arr_humidity_x,
                        type: 'line'
                    }
                    var data2 = [trace2];
                    var trace3 = {
                        y: arr_power,
                        x: arr_power_x,
                        type: 'line'
                    }
                    var data3 = [trace3]
                    Plotly.plot('temperature_graph', data1, layout1);
                    Plotly.plot('humidity_graph', data2, layout2);
                    Plotly.plot('energy_graph', data3);
                }
                else {
                    Plotly.extendTraces('temperature_graph', {y:[arr_temp_c], x:[arr_temp_c_x]}, [0]);
                    Plotly.extendTraces('humidity_graph', {y:[arr_humidity], x: [arr_humidity_x]}, [0]);
                    Plotly.extendTraces('energy_graph', {y:[arr_power], x: [arr_power_x]}, [0]);
                }
                flag = 0;
            },

            complete: function(){
                setTimeout(update_graphs, 5000);
            },

            error: function(){
                console.log('error!');
            }
        })
    }
    setTimeout(update_graphs, 5000);
})
