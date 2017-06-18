import os


def make_command(time):
    command = 'python /Users/HayatoSumino/Desktop/cloned/MLUni/examples/execute.py ' \
              '/Users/HayatoSumino/Desktop/ml_data/disneys/config.yaml ' \
              '-f /Users/HayatoSumino/Desktop/ml_data/disneys/features_{}.yaml ' \
              '-H /Users/HayatoSumino/Desktop/ml_data/disneys/disney{}'.format(time, time)
    print command
    return command


for i in range(20, 23):
    os.system(make_command(i))