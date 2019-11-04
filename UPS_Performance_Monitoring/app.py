import dash
from dash.dependencies import Output, Input
import dash_core_components as dcc
import dash_html_components as html
import plotly
import random
import plotly.graph_objs as go
from collections import deque
from OneM2M import *
import numpy as np
import time

XTemp = deque(maxlen=20)
XTemp.append(1)
YTemp = deque(maxlen=20)
YTemp.append(1)
XHum = deque(maxlen=20)
XHum.append(1)
YHum = deque(maxlen=20)
YHum.append(1)
Xem1 = deque(maxlen=20)
Xem1.append(1)
Yem1 = deque(maxlen=20)
Yem1.append(1)
Xem2 = deque(maxlen=20)
Xem2.append(1)
Yem2 = deque(maxlen=20)
Yem2.append(1)


app = dash.Dash(__name__)
app.layout = html.Div(
    [   html.H1('Temperature - Time Graph'),
        html.P('Y-Axis = Temp(Celsius), X-Axis = Time'),
        dcc.Graph(id='live-graph1', animate=False),
        html.H1('Humidity - Time Graph'),
        html.P('Y-Axis = Humidity(%), X-Axis = Time'),
        dcc.Graph(id='live-graph2', animate=False),
        html.H1('EnergyMeter1 - Time Graph'),
        html.P('Y-Axis = EnergyMeter1, X-Axis = Time'),
        dcc.Graph(id='live-graph3', animate=False),
        html.H1('EnergyMeter2 - Time Graph'),
        html.P('Y-Axis = EnergyMeter2, X-Axis = Time'),
        dcc.Graph(id='live-graph4', animate=False),
        dcc.Interval(
            id='graph-update1',
            interval=1*1000
        ),

    ]
)

@app.callback(Output('live-graph1', 'figure'),
              [Input('graph-update1', 'n_intervals')])
def update_graph_scatter(input_data):
    temp=get_data("http://139.59.42.21:8080/~/in-cse/in-name/Team43_UPS_performance_monitoring/pr_5_esp32_1/oe/oe_1_temperature/la")
    temp=temp[temp.find("(")+1:temp.find(")")]
    tempcel=(temp.split(",")[0])
    tempfeh=temp.split(",")[1]
    XTemp.append(XTemp[-1]+1)
    YTemp.append(tempcel)

    data = plotly.graph_objs.Scatter(
            x=list(XTemp),
            y=list(YTemp),
            name='Scatter',
            mode= 'lines+markers'
            )   

    return {'data': [data],'layout' : go.Layout(xaxis=dict(range=[min(XTemp),max(XTemp)]),
                                                yaxis=dict(range=[min(YTemp),max(YTemp)]),
                                                height=300)}

@app.callback(Output('live-graph2', 'figure'),
              [Input('graph-update1', 'n_intervals')])
def update_graph_scatter(input_data):
    hum=get_data("http://139.59.42.21:8080/~/in-cse/in-name/Team43_UPS_performance_monitoring/pr_5_esp32_1/oe/oe_1_rh/la")
    humidity=hum.split(";")[0]
    humtime=hum.split(";")[1]
    XHum.append(XHum[-1]+1)
    YHum.append(humidity)

    data = plotly.graph_objs.Scatter(
            x=list(XHum),
            y=list(YHum),
            name='Scatter',
            mode= 'lines+markers'
            )

    return {'data': [data],'layout' : go.Layout(xaxis=dict(range=[min(XHum),max(XHum)]),
                                                yaxis=dict(range=[min(YHum),max(YHum)]),
                                                height=300)}

@app.callback(Output('live-graph3', 'figure'),
              [Input('graph-update1', 'n_intervals')])
def update_graph_scatter(input_data):
    em1=get_data("http://139.59.42.21:8080/~/in-cse/in-name/Team43_UPS_performance_monitoring/pr_5_esp32_1/em/em_1_watts_total/la")
    em1=em1.split(";")[0]
    #print(em1)
    #humtime=hum.split(";")[1]
    Xem1.append(Xem1[-1]+1)
    Yem1.append(em1)

    data = plotly.graph_objs.Scatter(
            x=list(Xem1),
            y=list(Yem1),
            name='Scatter',
            mode= 'lines+markers'
            )

    return {'data': [data],'layout' : go.Layout(xaxis=dict(range=[min(Xem1),max(Xem1)]),
                                                yaxis=dict(range=[min(Yem1),max(Yem1)]),
                                                height=300)}

@app.callback(Output('live-graph4', 'figure'),
              [Input('graph-update1', 'n_intervals')])
def update_graph_scatter(input_data):
    em2=get_data("http://139.59.42.21:8080/~/in-cse/in-name/Team43_UPS_performance_monitoring/pr_5_esp32_1/em/em_1_watts_total/ol")
    em2=em2.split(",")[1]
    em2=em2.split(";")[0]
    #print(em1)
    #humtime=hum.split(";")[1]
    Xem2.append(Xem2[-1]+1)
    Yem2.append(em2)

    data = plotly.graph_objs.Scatter(
            x=list(Xem2),
            y=list(Yem2),
            name='Scatter',
            mode= 'lines+markers'
            )

    return {'data': [data],'layout' : go.Layout(xaxis=dict(range=[min(Xem2),max(Xem2)]),
                                                yaxis=dict(range=[min(Yem2),max(Yem2)]),
                                                height=300)}


if __name__ == '__main__':
    app.run_server(host='0.0.0.0', port=8080 ,debug=False)