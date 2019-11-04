import requests
from app import app
from flask import render_template
from flask import jsonify
from flask import request
from flask import url_for
import sqlite3 as sqlite
import sys
from flask_sqlalchemy import SQLAlchemy
import json
import time
from sqlalchemy.sql import func

app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///sensor.db'
db = SQLAlchemy(app)

class sensor_reading(db.Model):
    id = db.Column(db.Integer, primary_key = True)
    temperature_c = db.Column(db.Float, nullable = False)
    temperature_f = db.Column(db.Float, nullable = False)
    relative_humidity = db.Column(db.Float, nullable = False)
    time = db.Column(db.String, nullable = False)
    def __init__(self, temperature_c, temperature_f,relative_humidity, time):
        self.temperature_c = temperature_c
        self.temperature_f = temperature_f
        self.relative_humidity = relative_humidity
        self.time = time

class meter_reading(db.Model):
    id = db.Column(db.Integer, primary_key = True)
    power_reading = db.Column(db.Float, nullable = False)
    time = db.Column(db.String, nullable = False)
    def __init__(self, reading, time):
        self.power_reading = reading
        self.time = time

class hour_energy(db.Model):
    id = db.Column(db.Integer, primary_key = True)
    hour = db.Column(db.Integer, nullable = True)
    energy = db.Column(db.Integer, nullable = True)
    def __init__(self, hour, energy):
        self.hour = hour
        self.energy = energy

@app.route('/')
def home():
    return render_template('home.html')

@app.route('/_getReadings', methods = ['POST'])
def getReadings():
    headers = {
    	'X-M2M-Origin': 'admin:admin',
    	'Content-type': 'application/json'
    }

    response_temp = requests.get('http://139.59.42.21:8080/~/in-cse/in-name/Team40_DG_performance_monitoring/pr_2_esp32_1/oe/oe_1_temperature/la', headers = headers)
    sensor_time = response_temp.json()['m2m:cin']['con'].split(";")[1]
    response_temp = response_temp.json()['m2m:cin']['con'].split(";")[0]
    response_temp = response_temp[1:]
    response_temp = response_temp[:-1]
    response_temp = response_temp.split(',')

    response_humidity = requests.get('http://139.59.42.21:8080/~/in-cse/in-name/Team40_DG_performance_monitoring/pr_2_esp32_1/oe/oe_1_rh/la', headers = headers)
    response_humidity = response_humidity.json()['m2m:cin']['con'].split(";")[0]

    response_power = requests.get('http://139.59.42.21:8080/~/in-cse/in-name/Team40_DG_performance_monitoring/pr_2_esp32_1/em/em_1_watts_total/la', headers = headers)
    meter_time = response_power.json()['m2m:cin']['con'].split(";")[1]
    response_power = response_power.json()['m2m:cin']['con'].split(";")[0]
    meter_hour = int(meter_time.split(" ")[1].split(":")[0])

    new_sensor_reading = sensor_reading(float(response_temp[0]), float(response_temp[1]),float(response_humidity), str(sensor_time))
    new_power_reading = meter_reading(float(response_power), str(meter_time))

    db.create_all()
    db.session.add(new_sensor_reading)
    db.session.add(new_power_reading)
    db.session.commit()

    flag = request.form['all']
    ret = {'Sensor': [], 'Meter': []}
    flag = int(flag)
    energy = int(0)
    total_energy = 0

    time_prev = request.form['time']
    power_prev = float(request.form['power'])
    # print(request.form['power'])
    print(time_prev)
    if time_prev != "0":
        var1 = int(time.mktime(time.strptime(time_prev, '%Y-%m-%d %H:%M:%S')))
        var2 = int(time.mktime(time.strptime(str(meter_time), '%Y-%m-%d %H:%M:%S')))
        energy = (float(power_prev) * (int(var2) - int(var1)))/3600
        new_hour_energy = hour_energy(meter_hour, energy)
        db.create_all()
        db.session.add(new_hour_energy)
        db.session.commit()

    all_hour_energy = hour_energy.query.all()
    for readings in all_hour_energy:
        total_energy = total_energy + readings.energy

    if flag == 0:
        cur = {}
        cur2 = {}
        cur['temp_c'] = float(response_temp[0])
        cur['temp_f'] = float(response_temp[1])
        cur['humidity'] = float((response_humidity))
        cur['time'] = str(sensor_time)
        cur2['power'] = float(response_power)
        cur2['time'] = str(meter_time)
        ret['Sensor'].append(cur)
        ret['Meter'].append(cur2)
        print(var1, var2)
        ret['Energy'] = total_energy
        return jsonify(ret)

    all_sensor_reading = sensor_reading.query.all()
    all_meter_reading = meter_reading.query.all()
    for readings in all_sensor_reading:
        cur = {}
        cur['temp_c'] = readings.temperature_c
        cur['temp_f'] = readings.temperature_f
        cur['humidity'] = readings.relative_humidity
        cur['time'] = readings.time
        ret['Sensor'].append(cur)

    for readings in all_meter_reading:
        cur = {}
        cur['power'] = readings.power_reading
        cur['time'] = readings.time
        ret['Meter'].append(cur)
    ret['Energy'] = total_energy
    return jsonify(ret)
