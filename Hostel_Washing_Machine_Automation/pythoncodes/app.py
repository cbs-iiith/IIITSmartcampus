from copfile import *
from flask import Flask, jsonify,request,redirect,url_for,render_template,flash

import time
import numpy as np
import re
import random


import gspread
from oauth2client.service_account import ServiceAccountCredentials

from team_list_file import team_list

app = Flask(__name__)

server = "http://139.59.42.21:8080"
cse = "/~/in-cse/in-name/"


i = 15
ae = "Team" + str(i) + "_" + team_list[i]['title']
print(ae)

contname = "pr_3_esp32_1"

# val is for the application entity to read data values from
val = server + cse + ae + "/" + contname + "/la"


# /////////////////////////////////////
# GOOGLE SHEETS --- OBTAIN LIST OF USERS

# scope = ["https://spreadsheets.google.com/feeds"]
scope = ['https://www.googleapis.com/auth/analytics.readonly',
      'https://www.googleapis.com/auth/drive',
      'https://www.googleapis.com/auth/spreadsheets']
creds = ServiceAccountCredentials.from_json_keyfile_name('client_secret.json',scope)
client = gspread.authorize(creds)
# print(creds)

# print(reco)




#/////////////////////////////////////////////////////////////////////////
# FLASK RENDERING PAGES


# Route for handling the login page logic
@app.route('/', methods=['GET', 'POST'])
def login():
    shert = client.open("sample").sheet1
# reco = shert.get_all_records()
    users = shert.col_values(1)
    rnos = shert.col_values(2)
    error = None
    if request.method == 'POST':
        uname = request.form['username']
        rno = request.form['password']
        uidx = [i for i in range(len(users)) if users[i] == uname]
        ridx = [i for i in range(len(rnos)) if rnos[i] == rno]
        if len(ridx)==0 or len(uidx)==0:
            error = "No such details found"
        elif ridx[0] != uidx[0] :
            error = 'Invalid Credentials. Please try again.'
        else:
            return redirect(url_for('result'))
    return render_template('login.html', error=error)

# ///////////////////////////////////////////////
@app.route('/result',methods= ['POST', 'GET'])
def result():

    # getting data from onem2m
    x = get_data(val)
    print(x)
    y = re.split('[(),]',x[1])
    print(y)
    y = list(filter(None, y))
    temperature = y[0]
    humidity = y[1]
    water_flow_rate = y[2]
    total_flow = y[3]
    result = {}
    result = {'temperature' : temperature , 'humidity' : humidity , 'water_flow_rate' : water_flow_rate , 'total_flow' : total_flow}
    return render_template("result.html",result = result)

# ///////////////////////////////////////////////////////
@app.route('/result/control',methods = ['POST', 'GET'])
def ctrl():

    wor= request.form['word']
    k = 1
    if(wor=="on"):
        print("ON")
        k = 1
    else:
        print("OFF")
        k = 0
    
    # This is to post data from the app to the onem2m server to turn the machine On/Off using relay
    create_data_cin(server + cse + ae + "/" + "project_description",str(k))

    return "OK"

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=True)

# ///////////////////////////////////////////////////////////////////////////                                                   
