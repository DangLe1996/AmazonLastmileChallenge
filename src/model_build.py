
from os import path
import sys, json, time

sys.path.insert(0, 'TrainingModel')
from TrainingModel.exploreParameterMain import runMain

BASE_DIR = path.dirname(path.dirname(path.abspath(__file__)))
# BASE_DIR = r'/private/i/icontrer/l_dan15/dummyFolder'

# Get Directory

if __name__ == '__main__':
    training_routes_path=path.join(BASE_DIR, 'data/model_build_inputs')
    output_training_Path=path.join(BASE_DIR, 'data/model_build_outputs')

    runMain(training_routes_path,output_training_Path)

