## Introduction
This folder contains the model_apply.py and model_build.py files, which are called during the training and testing phase respectively. During the model 
build phase, the algorithm build a set of decision tree that encode the behavior of
historical routes and then during the model apply phase, these decision trees will be used
to predict the optimal vehicle routes depending on its location. 

## Cluster_ATPS
This folder contains our open source Clustered Asycnmatric TSP solver. This
solver is implemented in C and called during the Model Apply phase. The solver
require asn an input the optimal zone_id sequence for the route, then it will
find a optimal tour for each stop while respecting the given optimal zone_id
sequence. 

The optimal zone_id sequence is obtained using the decision tree obtained during
the model build phase, where the model learned the directional biases that drivers
make for each geographical region. 

## Training Model
The files contained in this folder are as follow:

### classifierUtils.py
Functions that build the decision tree classifiers and also using the classifiers
to output predicted optimal zone_id sequence

### dataset.py
Class ALMCData contains functions to read raw JSON files and store it into 
a pickle file for quicker re-loading time. It also convert input data into
different class variables that can be used throughout the program

### exploreParameterMain.py
Contains the master master build and apply functions. 

### TSP.py
Calling the Cluster_ATPS code and return the optimal clustered ATSP route. 

### utils.py
Contains functions to fix the corrupted zone_id that were given from the 
competition. 
















