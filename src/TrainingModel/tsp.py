import sys

import numpy as np
import platform

import ctypes as ct
import os
from os import path

BASE_DIR = path.dirname(path.dirname(path.abspath(__file__)))
# print('current path is ', BASE_DIR)
if platform.system() == 'Linux':
    dllPath = path.join(BASE_DIR, 'Cluster_ATSP/ATSP.so')
    ATSP_DLL = path.join(BASE_DIR, 'ATSP/ATSPSolver.so')

else:
    dllPath = path.join(BASE_DIR, 'Cluster_ATSP/banchandcutTSP.dll')
    ATSP_DLL = r'C:\Users\Zean Le\Desktop\Master Program\banchandcutTSP\x64\Release\ATSPSolver.dll'


# print('Path exist tsp', os.path.exists(dllPath))

def solveRouteClustered(route,c,stopSequence,zoneSequence):
    """

    Parameters
    ----------
    route: The route to solve
    c: travel cost matrix
    stopSequence: list of stops, ordered
    zoneSequence: ordered list of zone ids, with Z[i] is the number of stops in that zone in ith position

    Returns: Sorted Optimal Sequence begin at depot and optimal distance
    -------

    """
    hllDll = ct.CDLL(dllPath)

    runFunction = hllDll.Heuristic_Clustered_ATSP

    depot = route.depot
    dim_N = len(route.stops)
    dim_Z = len(zoneSequence) - 1

    UI16Ptr = ct.POINTER(ct.c_double)
    UI16PtrPtr = ct.POINTER(UI16Ptr)
    Int32Ptr = ct.POINTER(ct.c_int32)
    runFunction.argtypes = [ct.c_int,ct.c_int,ct.c_int, UI16PtrPtr,Int32Ptr, Int32Ptr,Int32Ptr]
    runFunction.restype = ct.c_double
    final_sequence = (ct.c_int32 * (dim_N + 1))()
    list_all_stops = (ct.c_int * len(stopSequence))(*stopSequence)
    zone_dim = (ct.c_int * len(zoneSequence))(*zoneSequence)

    np_arr_2d = np.array(c)

    # The "magic" happens in the following (3) lines of code
    ct_arr = np.ctypeslib.as_ctypes(np_arr_2d)
    UI16PtrArr = UI16Ptr * ct_arr._length_
    ct_ptr = ct.cast(UI16PtrArr(*(ct.cast(row, UI16Ptr) for row in ct_arr)), UI16PtrPtr)

    distance = runFunction(depot,dim_N,dim_Z,ct_ptr,zone_dim,list_all_stops,final_sequence)
    # print('distance is', distance)
    sequence = []
    for i in range(0, dim_N):
        sequence.append(final_sequence[i])
    assert (len(sequence) == dim_N )

    return sequence, distance

# ATSP_DLL =  path.join(BASE_DIR, 'Cluster_ATSP/ATSPSolver.dll')

# ATSP_DLL =  r'C:\Users\Zean Le\Desktop\Master Program\banchandcutTSP\x64\Release\ATSPSolver.dll'


def solveATSP(N,costMatrix):
    UI16Ptr = ct.POINTER(ct.c_double)
    UI16PtrPtr = ct.POINTER(UI16Ptr)
    hllDll = ct.CDLL(ATSP_DLL)
    runFunction = hllDll.runInstNew
    runFunction.argtypes = [ct.POINTER(ct.c_int32), UI16PtrPtr, ct.c_int]
    np_arr_2d = np.array(costMatrix)

    # The "magic" happens in the following (3) lines of code
    ct_arr = np.ctypeslib.as_ctypes(np_arr_2d)
    UI16PtrArr = UI16Ptr * ct_arr._length_
    ct_ptr = ct.cast(UI16PtrArr(*(ct.cast(row, UI16Ptr) for row in ct_arr)), UI16PtrPtr)
    result = (ct.c_int32 * (N))()

    runFunction(result, ct_ptr, N)
    sequence = []
    for i in range(0, N):
        sequence.append(result[i])
    # cost = 0
    # for src,dst in zip(sequence,sequence[1:]):
    #     cost += costMatrix[src][dst]
    # print('Cost is ', cost)
    return sequence

