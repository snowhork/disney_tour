import pandas as pd
import sys

a = pd.read_csv("/Users/HayatoSumino/Desktop/cloned/disney_tour/lib/others/data/dataset_fixed.csv")
b = a[a["year"] == 2017][a["month"] == 4][a['day'] == 1]

l = [[0 for i in range(15)] for j in range(37)]
for _, row in b.iterrows():
    l[int(row['id'])] = list(row[-15:])

df = pd.DataFrame(l, columns=list(range(8, 23)))
df.to_csv('_wait.csv', index=False)
