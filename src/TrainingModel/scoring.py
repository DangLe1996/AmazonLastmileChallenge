# These are modified versions of the original Amazon-provided
# scoring functions, where cost matrices are expected to be arrays
# and actual/submitted sequences are expected to be lists of ranks
#
# Example of score args in this version:
#
#   >>> score([3, 2, 1, 0], [1, 3, 2, 0],
#             np.array([[ 0., 10., 20., 30.],
#                       [10.,  0., 10., 20.],
#                       [20., 10.,  0., 10.],
#                       [30., 20., 10.,  0.]]))
#   0.4590202484393975
import csv

import numpy as np

from joblib import delayed, Parallel
from dask import array as da
import dask.bag as db
def isinvalid(actual, sub):
    '''
    Checks if submitted route is invalid.
    Parameters
    ----------
    actual : list
        Actual route.
    sub : list
        Submitted route.
    Returns
    -------
    bool
        True if route is invalid. False otherwise.
    '''
    if len(actual) != len(sub) or set(actual) != set(sub):
        raise Exception('not same length or contain same set')
        return True
    elif actual[0] != sub[0]:
        raise Exception('does not start from same depot')
        return True
    else:
        return False
    

def gap_sum(path, g):
    '''
    Calculates ERP between two sequences when at least one is empty.
    Parameters
    ----------
    path : list
        Sequence that is being compared to an empty sequence.
    g : int/float
        Gap penalty.
    Returns
    -------
    res : int/float
        ERP between path and an empty sequence.
    '''
    res = 0
    for p in path:
        res += g
    return res


def dist_erp(p_1, p_2, mat, g=1000):
    '''
    Finds cost between two points. Outputs g if either point is a gap.
    Parameters
    ----------
    p_1 : str
        ID of point.
    p_2 : str
        ID of other point.
    mat : dict
        Normalized cost matrix.
    g : int/float, optional
        Gap penalty. The default is 1000.
    Returns
    -------
    dist : int/float
        Cost of substituting one point for the other.
    '''
    if p_1 == 'gap' or p_2 == 'gap':
        dist = g
    else:
        dist = mat[p_1,p_2]
    return dist



def score(actual, sub, cost_mat, g=1000):
    '''
    Scores individual routes.
    Parameters
    ----------
    actual : list
        Actual route.
    sub : list
        Submitted route.
    cost_mat : ndarray
        Cost matrix.
    g : int/float, optional
        ERP gap penalty. Irrelevant if large and len(actual)==len(sub). The
        default is 1000.
    Returns
    -------
    float
        Accuracy score from comparing sub to actual.
    '''
    assert actual is not None, "actual sequence was None"
    assert sub is not None, "submitted sequence was None"
    norm_mat = normalize_matrix(cost_mat)
    actual, sub = convertToList(actual,sub)

    seqScore = seq_dev(actual, sub)
    editScore = erp_per_edit(actual, sub, norm_mat, g)
    # print('Seq dev score is ', seqScore)
    # print('ERP efit score is', editScore)
    return seqScore *editScore


def erp_per_edit(actual, sub, matrix, g=1000):
    '''
    Outputs ERP of comparing sub to actual divided by the number of edits involved
    in the ERP. If there are 0 edits, returns 0 instead.
    Parameters
    ----------
    actual : list
        Actual route.
    sub : list
        Submitted route.
    matrix : dict
        Normalized cost matrix.
    g : int/float, optional
        ERP gap penalty. The default is 1000.
    Returns
    -------
    int/float
        ERP divided by number of ERP edits or 0 if there are 0 edits.
    '''
    total, count = erp_per_edit_helper(actual, sub, matrix, g)
    if count == 0:
        return 0
    else:
        return total / count


def erp_per_edit_helper(actual, sub, matrix, g=1000, memo=None):
    '''
    Calculates ERP and counts number of edits in the process.
    Parameters
    ----------
    actual : list
        Actual route.
    sub : list
        Submitted route.
    matrix : dict
        Normalized cost matrix.
    g : int/float, optional
        Gap penalty. The default is 1000.
    memo : dict, optional
        For memoization. The default is None.
    Returns
    -------
    d : float
        ERP from comparing sub to actual.
    count : int
        Number of edits in ERP.
    '''
    if memo == None:
        memo = {}
    actual_tuple = tuple(actual)
    sub_tuple = tuple(sub)
    if (actual_tuple, sub_tuple) in memo:
        d, count = memo[(actual_tuple, sub_tuple)]
        return d, count
    if len(sub) == 0:
        d = gap_sum(actual, g)
        count = len(actual)
    elif len(actual) == 0:
        d = gap_sum(sub, g)
        count = len(sub)
    else:
        head_actual = actual[0]
        head_sub = sub[0]
        rest_actual = actual[1:]
        rest_sub = sub[1:]
        score1, count1 = erp_per_edit_helper(rest_actual, rest_sub, matrix, g, memo)
        score2, count2 = erp_per_edit_helper(rest_actual, sub, matrix, g, memo)
        score3, count3 = erp_per_edit_helper(actual, rest_sub, matrix, g, memo)
        option_1 = score1 + dist_erp(head_actual, head_sub, matrix, g)
        option_2 = score2 + dist_erp(head_actual, 'gap', matrix, g)
        option_3 = score3 + dist_erp(head_sub, 'gap', matrix, g)
        d = min(option_1, option_2, option_3)
        if d == option_1:
            if head_actual == head_sub:
                count = count1
            else:
                count = count1 + 1
        elif d == option_2:
            count = count2 + 1
        else:
            count = count3 + 1
    memo[(actual_tuple, sub_tuple)] = (d, count)
    return d, count

def normalize_matrix(mat):
    '''
    Normalizes cost matrix.
    Parameters
    ----------
    mat : ndarray
        Cost matrix.
    Returns
    -------
    new_mat : ndarray
        Normalized cost matrix.
    '''
    new_mat = (mat - np.mean(mat)) / np.std(mat)
    new_mat -= new_mat.min()
    return new_mat


def seq_dev(actual, sub):
    '''
    Calculates sequence deviation.
    Parameters
    ----------
    actual : list
        Actual route.
    sub : list
        Submitted route.
    Returns
    -------
    float
        Sequence deviation.
    '''


    actual = actual[1:-1]
    sub = sub[1:-1]
    comp_list = []
    for i in sub:
        comp_list.append(actual.index(i))
        comp_sum = 0
    for ind in range(1, len(comp_list)):
        comp_sum += abs(comp_list[ind] - comp_list[ind - 1]) - 1
    n = len(actual)
    score = (2 / (n * (n - 1))) * comp_sum
    # print(f"seq_dev score is {score}")
    return score

def stops2list(stops):
    '''
    Translates route from dictionary to list.
    Parameters
    ----------
    stops : dict
        Route stops as a dictionary.
    Returns
    -------
    route_list : list
        Route as a list.
    '''
    route_list = [0] * (len(stops) + 1)
    for stop in stops:
        route_list[stops[stop]] = stop
    route_list[-1] = route_list[0]
    return route_list

def convertToList(actual, sub):
    actual = np.argsort(actual).tolist()
    depot = actual[0]
    # %%
    resetIndex = sub.index(depot)
    sub = list(sub[resetIndex:] + sub[:resetIndex])

    actual.append(depot)
    sub.append(depot)

    return actual,sub
def scoreCustom(route,sub):
    actual = route.actual
    cost = route.times
    scoreValue = score(actual,sub,cost)
    # print(route.id,scoreValue)
    return scoreValue
def loss(routes, subs):
    '''Submission score of a set of routes and submitted sequences.'''
    tempList = [(route,sub) for route, sub in zip(routes, subs)]

    scoreList = db.from_sequence(tempList).map_partitions(lambda routes: [scoreCustom(r[0],r[1]) for r in routes]).compute()

    # test = [score(r[0],r[1])for r in tempList]
    totalScore =  sum(scoreList)
    # print('scoring exit')
    assert len(routes) == len(subs)

    result = []
    for scoreid, route in zip(scoreList, routes):
        result.append([scoreid,route.id, route.station_code])
    with open('submission2.csv', 'a', newline='\n') as file:
            writer = csv.writer(file)
            writer.writerows(result)

    return totalScore / len(routes), scoreList

