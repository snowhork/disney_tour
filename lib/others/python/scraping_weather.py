#!/usr/bin/env python
# -*- coding: utf-8 -*-

# from selenium import webdriver
# import pandas as pd
#
# b = webdriver.PhantomJS()
# b.get('http://tokyodisneyresort.info/callendar.php?year=2017&month=03&park=sea#sea_cal')
# trs = b.find_element_by_class_name('calendar').find_elements_by_tag_name('tr')
#
# print trs

import re
import numpy as np
import pandas as pd
import urllib2
from bs4 import BeautifulSoup
from dateutil.relativedelta import relativedelta


def get_tables(year=2017, month=3):
    month = '{0:02}'.format(int(month))
    url = 'http://tokyodisneyresort.info/callendar.php?year={}&month={}&park=sea#sea_cal'.format(year, str(month))
    html = urllib2.urlopen(url)

    soup = BeautifulSoup(html, 'html.parser')
    tables = soup.find_all('table')
    return tables


def get_pre_tables(year=2017, month=3):
    month = '{0:02}'.format(int(month))
    url = 'http://tokyodisneyresort.info/pre_callendar.php?year={}&month={}&park=sea#sea_cal'.format(year, str(month))
    html = urllib2.urlopen(url)

    soup = BeautifulSoup(html, 'html.parser')
    tables = soup.find_all('table')
    return tables


def main(table):
    soup = BeautifulSoup(str(table), 'html.parser')

    tds = soup.find_all('td')

    weathers = []
    crowds = []

    for td in tds:
        soup = BeautifulSoup(str(td), 'html.parser')
        try:
            img = soup.find('img').get('src')
            weather = re.search(r'\d', img).group()
            #print weather

        except:
            weather = -1  # unknown

        divs = soup.find_all('div')
        c = -1
        for div in divs:
            #print div.string
            try:
                c = re.search(r'\d+', div.string).group()
            except:
                pass

        if not (weather == -1 and c == -1):
            weathers.append(weather)
            crowds.append(c)

        #print '*****************'
        #print '-------'

    # for n, (i,j) in enumerate(zip(weathers, crowds)):
    #     print n+1,i,j

    return weathers, crowds


def get_df(year, month_):
    tables = get_tables(year, month_)
    weathers_land, crowds_land = main(tables[1])
    weathers_sea, crowds_sea = main(tables[2])

    df = pd.DataFrame(np.array([weathers_land, crowds_land, crowds_sea]).T,
                      columns=['weather', 'crowds_land', 'crowds_sea'])
    df['year'] = year
    df['month'] = month_
    df['day'] = [i+1 for i in range(len(df))]
    return df


import datetime
if __name__ == '__main__':
    year = 2016

    dfs = pd.DataFrame([], columns=['year', 'month', 'day', 'weather', 'crowds_land', 'crowds_sea'])

    first_date = datetime.date(2013, 3, 1)
    last_date = datetime.date(2017, 5, 1)

    now_date = first_date
    failed = []
    while now_date < last_date:
        year, month_ = now_date.strftime("%Y/%m").split("/")
        print year, month_
        df = get_df(year, month_)
        dfs = dfs.append(df)

        now_date += relativedelta(months=1)

    dfs = dfs.reindex(columns=['year', 'month', 'day', 'weather', 'crowds_land', 'crowds_sea'])
    dfs.to_csv('weather.csv', index=False)