#!/usr/bin/env python # coding: utf-8
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

from flask import Flask, render_template
from flask import request, jsonify

app = Flask(__name__)
app.config['DEBUG'] = True

import os
import pandas as pd
from predict_wait_time import get_today


_df = pd.read_csv('attraction_id_v2.csv')
# name_dict = {row['id']: row['name'] for _, row in _df.iterrows()}


@app.route("/")
def index():
    return "Hello World!"


@app.route("/pred/today", methods=['GET'])
def get_pred_wait_time():
    year, month, day = get_today()
    csv_path = '/var/www/python/data/pred_wait_time_{}{:02}{:02}.csv'.format(year, month, day)
    if os.path.exists(csv_path):
        df = pd.read_csv(csv_path)
        df = df.applymap(lambda x: round(x, 1))
        df['id'] = [i for i in range(len(df))]
        df['name'] = _df['name']
        df = df.reindex(columns=[df.columns[-1]]+list(df.columns[:-1]))

        return render_template(
            'table.html',
            title='predict time',
            data=df.as_matrix(),
            column_name=df.columns,
        )
    else:
        return 'sorry'


@app.route("/actual/today", methods=['GET'])
def get_wait_time():
    year, month, day = get_today()
    csv_path = '/var/www/python/data/_wait_time_{}{:02}{:02}.csv'.format(year, month, day)
    if os.path.exists(csv_path):
        df = pd.read_csv(csv_path)
        df = df.applymap(lambda x: round(x, 1))
        df['id'] = [i for i in range(len(df))]
        df['name'] = _df['name']
        return render_template(
            'table.html',
            title='predict time',
            data=df.as_matrix(),
            column_name=df.columns,
        )
    else:
        return 'sorry'


@app.route("/b", methods=['POST'])
def send():
    print request
    print request.form
    return "Hello World!"

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5100, debug=True)