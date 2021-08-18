import os

import joblib
from os import path

from TrainingModel.classifierUtils import getAreaClf, getRegionClf
from TrainingModel.exploreParameterMain import solveClusterATSP
from TrainingModel.utils import isInStation
from dask import bag as db, delayed
from dask.diagnostics.progress import ProgressBar
from TrainingModel.scoring import scoreCustom, loss
import os


class RouteEstimator:
    def fit(self, trainRoutes, outFolder):
        Depots = list(set([r.station_code for r in trainRoutes]))
        for depot in Depots:
            depotHQRRoutes = list(filter(isInStation(depot), trainRoutes))
            AreaClf = getAreaClf(depotHQRRoutes)
            RegionClf = getRegionClf(depotHQRRoutes)

            RegionPickleFile = '-'.join(['Region', depot]) + '.pkl'
            RegionPath = path.join(outFolder, RegionPickleFile)

            AreaPickleFile = '-'.join(['Area', depot]) + '.pkl'
            AreaPath = path.join(outFolder, AreaPickleFile)

            if os.path.exists(AreaPath):
                os.remove(AreaPath)
                os.remove(RegionPath)

            joblib.dump(AreaClf, AreaPath)
            joblib.dump(RegionClf, RegionPath)

        return

    def score(self, routes, outFolder):
        Depots = list(set([r.station_code for r in routes]))
        scoreList = []
        optimalSequences = []
        for station in Depots:
            print('scoring at station ', station)
            depotHQRRoutes = list(filter(isInStation(station), routes))

            RegionPickleFile = '-'.join(['Region', station]) + '.pkl'

            RegionPath = path.join(outFolder, RegionPickleFile)

            AreaPickleFile = '-'.join(['Area', station]) + '.pkl'
            AreaPath = path.join(outFolder, AreaPickleFile)

            AreaClf = joblib.load(AreaPath)
            RegionClf = joblib.load(RegionPath)

            b = db.from_sequence(depotHQRRoutes)
            result = b.map(lambda r: solveClusterATSP(r, RegionClf, AreaClf))
            with ProgressBar():
                sequences = result.compute()
            average,scorelist = loss(depotHQRRoutes,sequences)
            optimalSequences.extend(sequences)
            scoreList.extend(scorelist)
            # for route, sequence in zip(depotHQRRoutes, sequences):
            #     scoreTasks.append(delayed(scoreCustom)(route,sequence))
            #     # scores.append(scoreCustom(route,sequence))
            # with ProgressBar():
            #     scoreList.append( scoreTasks.compute())
        output = f'Average is {sum(scoreList)/len(scoreList)}\n'
        print(output)
        f = open("log.txt", "a")
        f.write(output)
        f.close()
        return optimalSequences

