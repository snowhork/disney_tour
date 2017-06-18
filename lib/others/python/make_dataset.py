#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import unicode_literals

import pandas as pd
import os
import datetime
from wget_gif import attraction_id


wdf = pd.read_csv('weather.csv')
data = pd.read_csv('アトラクションID_ランド_v2.csv').ix[:, ['id', 'site_id']].as_matrix()
siteid_to_id = {k[1]: k[0] for k in data}

def make_dataset(id):
    filename = '/Users/HayatoSumino/Desktop/python/Disney_predict/wget_and_ocr//ocr_done/data_{}.csv'.format(id)
    if not os.path.exists(filename):
        return
    df = pd.read_csv(filename, index_col=0)
    df = df.rename(columns={str(k): str(k) + ':00' for k in range(8, 23)})

    #map(lambda x: x[:4], list(df.index))
    df['year'] = map(lambda x: str(x)[:4], df.index.tolist())
    df['month'] = map(lambda x: str(x)[4:6], df.index.tolist())
    df['day'] = map(lambda x: str(x)[6:8], df.index.tolist())

    w = []  # 天気
    cl = []  # ランドの混み具合指数
    cs = []  # シーの混み具合指数
    wd = []  # 曜日

    for _, val in df.iterrows():
        year, month, day = map(int, [val['year'], val['month'], val['day']])
        # print id, year, month, day
        wdfi = wdf[wdf['year'] == year][wdf['month'] == month][wdf['day'] == day]
        wdfa = wdfi.as_matrix()[0]
        #print wdfa
        w.append(int(wdfa[3]))
        cl.append(int(wdfa[4]))
        cs.append(int(wdfa[5]))

        wd.append(datetime.datetime(year, month, day).weekday())

    df['weather'] = w
    df['crowds_land'] = cl
    df['crowds_sea'] = cs
    df['site_id'] = id
    df['id'] = siteid_to_id[id]
    df['weekday'] = wd

    # df.to_csv('/Users/HayatoSumino/Desktop/cloned/reciept/h_sumino/slight/data_{}+.csv'.format(id))
    return df

columns = [
    'year', 'month', 'day', 'weekday', 'weather', 'crowds_land',
    'crowds_sea', 'id', 'site_id',
    '8:00', '9:00', '10:00', '11:00', '12:00', '13:00', '14:00', '15:00', '16:00', '17:00',
    '18:00', '19:00', '20:00', '21:00', '22:00'
]

def main():
    dfs = pd.DataFrame([], columns=columns) # new
    for id in attraction_id.keys():

        df = make_dataset(id)
        if df is None:
            continue
        df = df.reindex(columns=columns)
        dfs = dfs.append(df)

    dfs = dfs.sort_values(by=['year', 'month', 'day'])
    dfs.to_csv('dataset.csv', index=False)

___c = [u'id', u'9:00', u'10:00', u'11:00', u'12:00', u'13:00', u'14:00',
    u'15:00', u'16:00', u'17:00', u'18:00', u'19:00', u'20:00', u'21:00',
    u'22:00']


def for_cpp_input(year, month, day):
    df = pd.read_csv('dataset.csv')
    df = df.query('year==@year & month==@month & day==@day')
    df = df.ix[:, ___c]
    df = df.sort_values(by='id')
    df.to_csv('wait_time_{}_{}_{}.csv'.format(year, str(month), str(day)), index=False)


if __name__ == '__main__':
    #for_cpp_input(2017, 4, 1)
    main()
