


from os import path
import sys, json, time

sys.path.insert(0, 'TrainingModel')
from TrainingModel.exploreParameterMain import applyMain

# Get Directory
BASE_DIR = path.dirname(path.dirname(path.abspath(__file__)))
# BASE_DIR = r'/private/i/icontrer/l_dan15/dummyFolder'


# BASE_DIR = r'R:\dummyFolder'
if __name__ == '__main__':

    training_routes_path=path.join(BASE_DIR, 'data/model_apply_inputs')
    output_training_Path=path.join(BASE_DIR, 'data/model_apply_outputs')
    build_output_path=path.join(BASE_DIR, 'data/model_build_outputs')

    applyMain(build_output_path,training_routes_path,output_training_Path)

