
import json
import numpy as np
import os
import pickle
from time import time as now
from utils import *

class Interner:
    def __init__(self):
        self._interned = {}

    def __call__(self, value):
        return self._interned.setdefault(value, value)


class ALMCData:
    """
        Holds data structures useful for reading route- and stop-level
        features from the Amazon Last Mile Challenge datasets.

        >>> routes = ALMCData.from_json('data/model_build_inputs')
        >>> routes[0].{id,station,departure,capacity,score}
        >>> routes[0].stops[0].{id, })
        >>> routes[0].station)
    """

    class Route:
        __slots__ = ('_id', 'station_code', 'date', 'departure_time', 'executor_capacity',
                     'score', 'stops', 'times', 'actual', 'depot'
                     ,'departure_time_utc', )

        def __repr__(self): return f'{self.id} ({len(self.stops)} stops)'

        @property
        def id(self): return 'RouteID_' + self._id

        def stop_ids(self): return tuple(stop.id for stop in self.stops)

        def __len__(self): return len(self.stops)

    class Stop:
        __slots__ = ('id', 'lat', 'lng', 'type', 'zone_id', 'packages', 'end_time_utc', 'start_time_utc')

        def __repr__(self): return f'{self.id} ({len(self.packages) if self.packages else 0} packages)'

    class Package:
        __slots__ = ('_id', 'scan_status', 'planned_service_time',
                     'start_time', 'end_time', 'volume', 'end_time_utc', 'start_time_utc')

        def __repr__(self): return self.id or 'Package'

        @property
        def id(self): return ('PackageID_' + self._id) if self._id else None





    @classmethod
    def load(cls, inputs_dir, *, caching=False, score_filter=None, want_times=True,
             want_packages=True, want_package_ids=False):
        """
        Loads data from the given inputs directory.
        Use caching=True to check for a fast cached version (or build one if none).
        Use score_filter={'High'} (or can add 'Medium', 'Low') to filter what gets parsed.
        Use want_*=False to skip some parsing (faster, smaller file) when suitable.
        """
        if caching:
            # Generate a cache file name based on the options requested
            option_str = ''.join('T' if opt else 'F' for opt in [want_times,
                                                                 want_packages,
                                                                 want_package_ids])
            filter_str = ''.join(score[0] for score in score_filter)
            cache_file = f"cache-{option_str}-{filter_str}.pkl"
            cache_path = os.path.join(inputs_dir, cache_file)

            # If that file exists, just load it and don't parse the raw JSON
            if os.path.exists(cache_path):
                with open(cache_path, 'rb') as f:
                    return pickle.load(f)

        # Parse the raw JSON
        data = cls.load_json(inputs_dir, score_filter=score_filter, want_times=want_times,
                             want_packages=want_packages, want_package_ids=want_package_ids)

        # If we were asked to use caching, then write out a cache for next time
        if caching:
            data.save(cache_path)

        return data

    @classmethod
    def load_json(cls, inputs_dir, *, want_times=True, want_packages=True,
                  want_package_ids=False,
                  score_filter=None):
        """Parses the raw JSON data."""

        is_build = 'build' in inputs_dir
        file_prefix = '' if is_build else 'new_'  # new_route_data instead of route_data

        def load_json(json_file):
            print(f'Loading {json_file:30}... ', end='');
            tic = now()
            json_path = os.path.join(inputs_dir, json_file)
            with open(json_path, 'rb') as f:
                json_data = json.load(f)
            print(f'{now() - tic:.4f} sec')
            return json_data

        # Interning strings etc reduces memory/file by ~60 MB.
        intern = Interner()

        ####################################
        # Load routes and stops
        ####################################
        def get_stop(route_id, stop_id, route, stop):
            zone_id = stop['zone_id']
            if not isinstance(zone_id, str):
                zone_id = None
            s = ALMCData.Stop()
            s.id = intern(stop_id)
            s.lat = intern(stop['lat'])
            s.lng = intern(stop['lng'])
            s.type = intern(stop['type'])
            s.zone_id = intern(zone_id)
            return s

        def get_route(route_id, route):
            score = None
            if 'route_score' in route:
                score = route['route_score']
                if score_filter and score not in score_filter:
                    return None
            r = ALMCData.Route()
            r._id = route_id.replace('RouteID_', '')
            r.station_code = intern(route['station_code'])
            r.date = intern(route['date_YYYY_MM_DD'])
            r.departure_time = intern(route['departure_time_utc'])
            r.departure_time_utc = route['departure_time_utc']
            r.executor_capacity = intern(route['executor_capacity_cm3'])
            r.score = intern(score)
            r.stops = tuple(get_stop(route_id, stop_id, route, stop)
                            for stop_id, stop in route['stops'].items())




            return r

        routes = load_json(file_prefix + 'route_data.json')
        print(f'{"Processing routes":38}... ', end='');
        tic = now()
        data = cls()
        data._routes = tuple(get_route(route_id, route) for route_id, route in routes.items())
        data._routes = tuple(filter(lambda route: route is not None, data._routes))
        num_stops = sum(len(route.stops) for route in data._routes)
        route_ids = {route.id for route in data._routes}
        print(f'{now() - tic:.4f} sec ({len(data)} routes, {num_stops} stops)')
        del routes

        ####################################
        # Load packages
        ####################################
        if want_packages:
            def get_package(package_id, package):
                time_window = package['time_window']
                start_time = time_window['start_time_utc']

                if isinstance(start_time, str):
                    # Remove date, since can be inferred from
                    # route.date and route.departure_time
                    start_time = start_time[start_time.find(' ') + 1:]
                    end_time = time_window['end_time_utc']
                    end_time = end_time[end_time.find(' ') + 1:]
                else:
                    start_time = None
                    end_time = None
                package_dims = package['dimensions']
                p = ALMCData.Package()
                p._id = package_id.removeprefix('PackageID_') if want_package_ids else None
                p.scan_status = intern(package['scan_status']) \
                    if 'scan_status' in package else None
                p.planned_service_time = intern(package['planned_service_time_seconds'])
                p.start_time = intern(start_time)
                p.end_time = intern(end_time)
                p.end_time_utc = time_window['end_time_utc']
                p.start_time_utc = time_window['start_time_utc']
                p.volume = intern(int(package_dims['depth_cm'] * \
                                      package_dims['height_cm'] * \
                                      package_dims['width_cm']))
                return p

            packages = load_json(file_prefix + 'package_data.json')
            print(f'{"Processing packages":38}... ', end='');
            tic = now()
            num_packages = 0
            for route in data._routes:
                for stop in route.stops:
                    stop_packages = packages[route.id][stop.id]
                    stop.start_time_utc = None
                    stop.end_time_utc = None
                    stop.packages = tuple(get_package(package_id, package)
                                          for package_id, package in stop_packages.items())
                    for p in stop.packages:
                        if p.start_time_utc != None:
                            stop.start_time_utc = p.start_time_utc
                            stop.end_time_utc = p.end_time_utc
                            break
                    num_packages += len(stop.packages)
                    if len(stop.packages) == 0:
                        route.depot = route.stops.index(stop)
                fixZoneID(route)

            print(f'{now() - tic:.4f} sec ({num_packages} packages)')
            del packages

        ####################################
        # Load travel times
        ####################################
        if want_times:
            def load_travel_times(json_file):
                print(f'Loading {json_file:30}... ', end='');
                tic = now()

                EOB_MARKER = b'}}, '
                EOF_MARKER = b'}}}'
                CHUNK_SIZE = 256  # 4096*8
                chunks = []
                times = {}
                eob = -1  # End-of-block character index within concatenation of all chunks.
                eof = False

                def update_eob_eof():
                    nonlocal chunks
                    nonlocal eob
                    nonlocal eof
                    chunk = chunks[-1]
                    chunk_eob = chunk.find(EOB_MARKER)
                    eob = -1
                    if chunk_eob != -1:
                        # Found an EOB marker, so calculate its index in final block
                        eob = sum(len(c) for c in chunks[:-1]) + chunk_eob
                    elif len(chunks) > 1:
                        # Check for end-of-block marker that straddles two chunks (annoying)
                        chunk_edge = chunks[-2][-(len(EOB_MARKER) - 1):] + chunk[:len(EOB_MARKER) - 1]
                        chunk_eob = chunk_edge.find(EOB_MARKER)
                        if chunk_eob >= 0:
                            eob = sum(len(c) for c in chunks[:-1]) - len(EOB_MARKER) + 1 + chunk_eob
                    if eob == -1 and chunk.endswith(EOF_MARKER):
                        # Check for end-of-file marker last
                        eob = sum(len(c) for c in chunks[:-1]) + len(chunk) - 3
                        eof = True

                with open(os.path.join(inputs_dir, json_file), 'rb') as f:
                    eof = False
                    while not eof:
                        # While haven't found "end of JSON block" pattern, keep reading.
                        while eob == -1:
                            assert not eof, "problem parsing JSON file"
                            chunk = f.read(CHUNK_SIZE)
                            chunks.append(chunk)
                            update_eob_eof()
                            eof = len(chunk) < CHUNK_SIZE

                        # The current chunk contains the end of at least one JSON block
                        while eob != -1:
                            # Fuse the current chunks and extract the next JSON block
                            buffer = b''.join(chunks)
                            block = buffer[:eob + 2] + b'}'  # include the }} from the file plus extra }

                            # Parse the JSON
                            id_end = block.index(b'"', 2)
                            route_id = block[2:block.index(b'"', 2)].decode('utf-8')
                            assert route_id.startswith('RouteID_')
                            if route_id in route_ids:
                                doc = json.loads(block)
                                assert len(doc.items()) == 1
                                _, srcs = next(iter(doc.items()))
                                times_array = np.fromiter((dist for _, dsts in srcs.items()
                                                           for _, dist in dsts.items()),
                                                          dtype=np.float)
                                times[route_id] = times_array.reshape(len(srcs), len(srcs))

                            # Reset the chunks at continuation in buffer
                            chunks = [b'{', buffer[eob + len(EOB_MARKER):]]
                            update_eob_eof()

                print(f'{now() - tic:.4f} sec')
                return times

            times = load_travel_times(file_prefix + 'travel_times.json')
            assert len(times) == len(route_ids), f'Expected {len(route_ids)} but found {len(times)}'
            for route in data._routes:
                route.times = times[route.id]
                assert len(route.times) == len(route.stops)
            del times

        ####################################
        # Load actual sequences (if any)
        ####################################
        if is_build:
            actual_seqs = load_json('actual_sequences.json')
            for route in data._routes:
                route.actual = tuple(actual_seqs[route.id]['actual'].values())

            del actual_seqs

        return data

    def __iter__(self):
        return self._routes.__iter__()

    def __repr__(self):
        return f'({len(self)} routes)'

    def __len__(self):
        return self._routes.__len__()

    def __getitem__(self, i):
        return self._routes[i]

    def save(self, filename):
        with open(filename, 'wb') as f:
            return pickle.dump(self, f, pickle.HIGHEST_PROTOCOL)

    def sort(self, key):
        self._routes = sorted(self._routes, key=key)


    def filter(self,routes, func):
        return  tuple(filter(func, routes))


load = ALMCData.load