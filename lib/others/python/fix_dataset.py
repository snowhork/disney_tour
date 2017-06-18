import pandas as pd
import sys
from math import fabs

df = pd.read_csv(sys.argv[1])

column = [
    '8:00', '9:00', '10:00', '11:00', '12:00',
    '13:00', '14:00', '15:00', '16:00', '17:00',
    '18:00', '19:00', '20:00', '21:00', '22:00'
]


def seven_to_one(num):
    return float(str(num)[:-3].replace('7', '1')+str(num)[-3:])


for i in range(len(column) - 1):
    pl, nl = list(df[column[i]]), list(df[column[i+1]])
    for j in range(len(pl)):
        s = seven_to_one(pl[j])
        if fabs(s - nl[j]) < fabs(pl[j] - nl[j]) * 0.5:
            print 'fixed', column[i], j, pl[j], s
            pl[j] = s

        s = seven_to_one(nl[j])
        if fabs(pl[j] - s) < fabs(pl[j] - nl[j]) * 0.5:
            print 'fixed', column[i+1], j, nl[j], s
            pl[j] = s

    df[column[i]] = pl
    df[column[i+1]] = nl

df.to_csv(sys.argv[1][:-4]+'_fixed.csv')