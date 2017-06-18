def script_maker(hour):
    return "python execute.py ~/Desktop/ml_data/disneys/config.yaml" \
           " -f /Users/HayatoSumino/Desktop/ml_data/disneys/features_{}.yaml " \
           "-H /Users/HayatoSumino/Desktop/ml_data/disney+/disney{}".format(hour, hour)

for i in range(9, 23):
    print script_maker(i)