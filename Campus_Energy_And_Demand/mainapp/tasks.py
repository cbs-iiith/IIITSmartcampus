from celery import task
from .models import MainModel
from ESW_Campus_Energy_and_Demand_Dashboard.settings import OneM2M_IP
import requests
from datetime import datetime, timedelta

@task()
def import_values():
    model = MainModel()
    URL = "http://" + OneM2M_IP + ":8080/~/in-cse/in-name/campus_energy_demand/node_1/la/"
    headers = {
        "X-M2M-Origin": "admin:admin",
        "Accept": "application/json"
    }
    data = requests.get(URL, headers=headers).json()
    l = data['m2m:cin']['con'].split()
    model.temp = l[1]
    model.humidity = l[0]
    # shabbydo

    model.save()

@task()
def delete_values():
    time_threshold = datetime.now() - timedelta(hours=24)
    MainModel.objects.filter(time__lt=time_threshold).delete()
