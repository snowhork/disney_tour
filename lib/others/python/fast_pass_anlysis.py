# coding: utf-8

import sys
import numpy as np
import pandas as pd
from scraping_weather import get_tables, main

#df = pd.read_csv('/Users/HayatoSumino/Desktop/cloned/disney_tour/lib/others/data/crowds.csv')
weather4, crowds_land4 = main(get_tables(2017, 4)[1])
weather5, crowds_land5 = main(get_tables(2017, 5)[1])

csv_path = sys.argv[1]

fp = pd.read_csv(csv_path, header=None)
fp = fp.rename(columns={0: 'time', 1: 'diff', 2: 'date'})
fp['crowds_land'] = -1
fp['weather'] = -1

for ix, row in fp.iterrows():
    r = str(int(row['date']))
    year, month, day = map(int, [r[:4], r[4:6], r[6:]])
    if month == 4:
        fp.ix[ix, 'crowds_land'] = crowds_land4[day-1]
        fp.ix[ix, 'weather'] = weather4[day-1]
    elif month == 5:
        fp.ix[ix, 'crowds_land'] = crowds_land4[day-1]
        fp.ix[ix, 'weather'] = weather5[day-1]


steps_of_history=10
def create_dataset(dataset, steps_of_history=10, steps_in_future=1):
    X, Y = [], []
    for i in range(0, len(dataset)-steps_of_history, steps_in_future):
        X.append(dataset[i:i+steps_of_history])
        Y.append(dataset[i + steps_of_history])
    #X = np.reshape(np.array(X), [-1, steps_of_history, 1])
    #Y = np.reshape(np.array(Y), [-1, 1])
    return X, Y


days = fp['date'].drop_duplicates()
data = []
for day in days:
    _fp = fp[fp['date'] == day]
    _fp.sort_values(by='time')
    previous, _ = create_dataset(list(_fp['diff']), steps_of_history)
    other = _fp[steps_of_history:]
    for i in range(len(previous)):
        data.append(previous[i] + list(other.iloc[i]))

df = pd.DataFrame(data, columns=list(reversed(range(10)))+list(fp.columns))
df.to_csv(sys.argv[1][:-4]+'_formatted.csv', index=False)