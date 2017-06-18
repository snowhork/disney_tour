#!/usr/bin/env python
# coding: utf-8

import os
import datetime
import pandas as pd

attraction_id = {
    110: 'ﾋﾞｯｸﾞｻﾝﾀﾞｰﾏｳﾝﾃﾝ',
    163: 'ﾓﾝｽﾀｰｽﾞ･ｲﾝｸ“ﾗｲﾄﾞ＆ｺﾞｰｼｰｸ！”',
    123: 'ﾌﾟｰさんのﾊﾆｰﾊﾝﾄ',
    112: 'ｽﾌﾟﾗｯｼｭﾏｳﾝﾃﾝ',
    392: 'ｽﾃｨｯﾁ･ｴﾝｶｳﾝﾀｰ',
    126: 'ﾐｯｷｰの家とﾐｰﾄ･ﾐｯｷｰ',
    134: 'ﾊﾞｽﾞ･ﾗｲﾄｲﾔｰのｱｽﾄﾛﾌﾞﾗｽﾀｰ',
    366: 'ｼﾞｬﾝｸﾞﾙｸﾙｰｽﾞ：ﾜｲﾙﾄﾞﾗｲﾌ･ｴｸｽﾍﾟﾃﾞｨｼｮﾝ',
    133: 'ｽﾍﾟｰｽ･ﾏｳﾝﾃﾝ',
    132: 'ｽﾀｰ･ﾂｱｰｽﾞ：ｻﾞ･ｱﾄﾞﾍﾞﾝﾁｬｰｽﾞ･ｺﾝﾃｨﾆｭｰ',
    120: 'ﾎｰﾝﾃｯﾄﾞﾏﾝｼｮﾝ',
    102: 'ｶﾘﾌﾞの海賊',
    116: 'ﾐｯｷｰのﾌｨﾙﾊｰﾏｼﾞｯｸ',
    121: 'ｲｯﾂ･ｱ･ｽﾓｰﾙﾜｰﾙﾄﾞ',
    114: 'ﾋﾟｰﾀｰﾊﾟﾝ空の旅',
    167: 'ｼﾝﾃﾞﾚﾗのﾌｪｱﾘｰﾃｲﾙ･ﾎｰﾙ',
    # 128: 'ｶﾞｼﾞｪｯﾄのｺﾞｰｺｰｽﾀｰ',  -> 廃止
    124: 'ﾛｼﾞｬｰﾗﾋﾞｯﾄのｶｰﾄｩｰﾝｽﾋﾟﾝ',
    137: 'ｸﾞﾗﾝﾄﾞｻｰｷｯﾄ･ﾚｰｽｳｪｲ',
    136: 'ｽﾀｰｼﾞｪｯﾄ',
    106: '魅惑のﾁｷﾙｰﾑ：ｽﾃｨｯﾁ･ﾌﾟﾚｾﾞﾝﾂ“ｱﾛﾊ･ｴ･ｺﾓ･ﾏｲ！”',
    118: '空飛ぶﾀﾞﾝﾎﾞ',
    115: '白雪姫と七人のこびと',
    174: 'ｸﾞｰﾌｨｰのﾍﾟｲﾝﾄ＆ﾌﾟﾚｲﾊｳｽ',
    117: 'ﾋﾟﾉｷｵの冒険旅行',
    122: 'ｱﾘｽのﾃｨｰﾊﾟｰﾃｨｰ',
    107: 'ｳｴｽﾀﾝﾗﾝﾄﾞ･ｼｭｰﾃｨﾝｸﾞｷﾞｬﾗﾘｰ',
    109: '蒸気船マークトウェイン号',
    125: 'ミニーの家',
    113: 'ビーバーブラザーズのカヌー探険',
    129: 'ドナルドのボート',
    111: 'トムソーヤ島いかだ',
    119: 'キャッスルカルーセル',
    108: 'カントリーベア・シアター',
    127: 'チップとデールのツリーハウス',
    101: 'オムニバス',
    105: 'スイスファミリー・ツリーハウス'
}


def wget_gif(id, date):
    url = 'http://tokyodisneyresort.info/hourly_pictures/day/{}_{}.gif'.format(id, date)
    res = os.system('wget {}'.format(url))
    if not res:
        print('OK: {}_{}'.format(id, date))
        filename = url.split('/')[-1]
        os.system('mv {} data/'.format(filename))
    else:
        print('NG: {}_{}'.format(id, date))
    return res


def wget_gifs(id, first_date, last_date):
    now_date = first_date
    failed = []
    while now_date < last_date:
        date_str = now_date.strftime("%Y%m%d")
        res = wget_gif(id, date_str)
        if res: failed.append(date_str)
        now_date += datetime.timedelta(days=1)
    return failed


def main():
    for id in attraction_id.keys():
        if os.path.exists('failed_{}.txt'.format(id)):
            print 'continue'
            continue
        first_date = datetime.date(2013, 3, 1)
        last_date = datetime.date(2017, 4, 21)
        failed = wget_gifs(id, first_date, last_date)
        with open('failed_{}.txt'.format(id), 'w') as f:
            f.write('\n'.join(failed))


if __name__ == '__main__':
    main()