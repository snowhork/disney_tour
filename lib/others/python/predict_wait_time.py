# coding: utf-8

import os
import re
import pandas as pd
import numpy as np
import sys
from numpy.random import rand
import datetime
from MLuni.predict import Predictor
from MLuni.utils import io_handler as IO
import requests
import time
import json
import urllib2
import traceback
from bs4 import BeautifulSoup
from dateutil.relativedelta import relativedelta
from pprint import pprint
import scraping_weather as sw
from pytz import timezone

NaN = float('nan')

df = pd.read_csv(os.path.join(os.path.abspath(os.path.dirname(__file__)), 'attraction_id_v2.csv'))
convert = {r['site_id']: r['id'] for _, r in df.iterrows()}


def get_today_wait_time(year, month, day):
    csv_path = '/var/www/python/data/_wait_time_{}{:02}{:02}.csv'.format(year, month, day)
    if os.path.exists(csv_path):
        waitdf = pd.read_csv(csv_path)
        _data = []
        for i, row in waitdf.iterrows():
            _data.append(fill_nan(list(row)))
        waitdf = pd.DataFrame(_data, columns=waitdf.columns)

        return {int(k): waitdf[k].tolist() for k in waitdf.columns}
    else:
        return {k: [float('nan')] * 37 for k in range(8, 23)}


def update_now():
    print('Updating...')

    url = 'http://tokyodisneyresort.info/realtime.php?park=land'
    html = urllib2.urlopen(url)
    soup = BeautifulSoup(html, 'html.parser')
    items = soup.find_all('div', class_='realtime_item')

    year, month, day = get_today()
    wait_time = get_today_wait_time(year, month, day)

    for item in items:
        href = item.a.get('href')

        id_ = re.search(u'attr_id=(\d+)', href).group(1)
        try:
            s = re.search(u'(\d+)分', item.text).group(1)
            t = re.search(u'(\d+):\d+更新', item.text).group(1)
            wait_time[int(t)][convert[int(id_)]] = s
            print('{},{} -> {}min'.format(t, convert[int(id_)], s))
        except:
            pass  # print(traceback.format_exc())

    pd.DataFrame.from_dict(wait_time).to_csv(
        '/var/www/python/data/_wait_time_{}{:02}{:02}.csv'.format(year, month, day), index=False)


def fill_nan(l):
    c = 5
    for n, v in enumerate(l):
        if np.isnan(v):
            i = n + 1
            if i >= len(l):
                continue
            while np.isnan(l[i]):
                i += 1
                if i == len(l):
                    break

            if i != len(l):
                l[n] = l[i]
                c = l[i]

    return list(pd.Series(l).fillna(c))


def get_past_data(year, month, day, hour):
    wait_time = get_today_wait_time(year, month, day)

    data = {}
    for h in range(8, hour):
        data['{}:00'.format(h)] = wait_time[h]
    return data


def get_weather(hour):
    assert 8 <= hour <= 22
    index = max(0, (hour - 9) / 3)

    res = requests.get('http://api.openweathermap.org/data/2.5/forecast?q=urayasu,jp&APPID=ff668600431cdf728b52e272657b9513')
    res = json.loads(res.text)
    weather = res['list'][index]['weather'][0]['main']

    if weather == 'Clear':
        return 1
    elif weather == 'Clouds':
        return 2
    elif weather == 'Rain':
        return 3
    else:
        line_notify('unknown weather {}'.format(weather))
        return 1


def get_today():
    x = datetime.datetime.now(timezone('Asia/Tokyo'))
    return x.year, x.month, x.day


def get_crowds(year, month, day):
    # スクレイピングする
    table = sw.get_pre_tables(year, month)
    _, crowds_land = sw.main(table[1])
    _, crowds_sea = sw.main(table[2])
    try:
        c_land, c_sea = crowds_land[day-1], crowds_sea[day-1]
        return c_land, c_sea
    except:
        line_notify('failed crowds {}'.format(crowds_land))
        return 50, 50


def main(year, month, day, hour, model):
    """
    hour int
    model str
    """

    weather = get_weather(hour)
    past_data = get_past_data(year, month, day, hour)
    crowds_land, crowds_sea = get_crowds(year-1, month, day)

    df = pd.read_csv(os.path.join(os.path.abspath(os.path.dirname(__file__)), 'attraction_id_v2.csv'))

    df['year'] = year
    df['month'] = month
    df['day'] = day
    df['weekday'] = datetime.datetime(year, month, day).weekday()
    df['weather'] = weather
    df['crowds_land'] = crowds_land
    df['crowds_sea'] = crowds_sea

    for h in past_data.keys():
        df[h] = past_data[h]

    #predictor_path = '/Users/HayatoSumino/Desktop/ml_data/disneys'
    predictor_path = '/var/www/python/predictor'
    predictor = Predictor(os.path.join(predictor_path, 'disney{}/{}'.format(hour, model)))
    pred = predictor.predict(df, return_type=dict)
    for p in pred.keys():
        df[p] = pred[p]

    # if 'site_id' in df.columns:
    #     del df['site_id']
    # if 'name' in df.columns:
    #     del df['name']

    df = df.reindex(columns=[u'id', u'9:00', u'10:00', u'11:00', u'12:00', u'13:00', u'14:00',
       u'15:00', u'16:00', u'17:00', u'18:00', u'19:00', u'20:00', u'21:00',
       u'22:00'])
    print(df)

    df.to_csv('/var/www/python/data/pred_wait_time_{}{:02}{:02}.csv'.format(year, month, day), index=False)
    df.to_csv('/var/www/python/data/_backup_{}{:02}{:02}{:02}.csv'.format(year, month, day, hour), index=False)

    # 応急処置
    # os.system('scp -r pred_wait_time.csv ec2-user@52.198.30.107:/var/www/disney_tour/current/lib/others/cpp/input/')


def line_notify(message,token = "o0Xs2ymLhJulwT9v9f4thY4ASKVJ3J0FV9M0o4AtZJ3"):
    import urllib
    import urllib2

    params = {"message":message}
    params = urllib.urlencode(params)

    req = urllib2.Request("https://notify-api.line.me/api/notify")
    # ヘッダ設定
    req.add_header("Authorization","Bearer "+token)
    # パラメータ設定
    req.add_data(params)

    res = urllib2.urlopen(req)
    r = res.read()
    print(r)


if __name__ == '__main__':
    model = 'RandomForestRegressor'
    if len(sys.argv) > 2:
        year, month, day, hour = map(int, sys.argv[1:])
        main(year, month, day, hour, model)
        exit()

    while True:
        hour = datetime.datetime.now(timezone('Asia/Tokyo')).hour
        if 8 <= hour < 23:
            try:
                update_now()
            except:
                line_notify(traceback.format_exc())

        if not 9 <= hour < 23:
            print('good night..')
            time.sleep(60)
            continue

        try:
            year, month, day = get_today()
            main(year, month, day, hour, model)
            time.sleep(300)
        except:
            print(traceback.format_exc())
            line_notify(traceback.format_exc())
            time.sleep(60)

        time.sleep(60)


