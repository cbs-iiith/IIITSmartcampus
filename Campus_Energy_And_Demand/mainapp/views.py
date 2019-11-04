from django.shortcuts import render
from django.http import HttpResponse
from collections import OrderedDict
from mainapp.models import MainModel
from datetime import datetime, timedelta

from .fusioncharts import FusionCharts


def index(request):
    time_threshold = datetime.now() - timedelta(hours=100)

    l = MainModel.objects.filter(time__gte=time_threshold)

    data_source = [OrderedDict(), OrderedDict()]
    data_source[0]['chart'] = OrderedDict({
        "caption": "Temperature(in past 24 hours)",
        "subCaption": "In degrees Celsius",
        "xAxisName": "Reading in past 24 hours",
        "yAxisName": "Temperature",
        "theme": "candy"
    })
    data_source[0]['data'] = []
    data_source[1]['chart'] = OrderedDict({
        "caption": "Humidity(in past 24 hours)",
        "subCaption": "In %",
        "xAxisName": "Reading in past 24 hours",
        "yAxisName": "Humidity",
        "theme": "candy"
    })
    data_source[1]['data'] = []
    i = 1

    for model in l[:40]:
        data_source[0]["data"].append({'label': i, 'value': float(model.temp)})
        data_source[1]["data"].append({'label': i, 'value': float(model.humidity)})
        i = i + 1

    graph = [
        FusionCharts("line", "Temperature", "1000", "700", "temp-container", "json", data_source[0]),
        FusionCharts("line", "Humidity", "1000", "700", "hum-container", "json", data_source[1])
    ]

    return render(request, 'index.html', {
        'temp_output': graph[0].render(),
        'hum_output': graph[1].render(),
    })
