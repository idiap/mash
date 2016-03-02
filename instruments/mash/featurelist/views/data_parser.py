#! /usr/bin/env python

################################################################################
# The MASH Framework contains the source code of all the servers in the
# "computation farm" of the MASH project (http://www.mash-project.eu),
# developed at the Idiap Research Institute (http://www.idiap.ch).
#
# Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
# Written by Philip Abbet (philip.abbet@idiap.ch)
#
# This file is part of the MASH Framework.
#
# The MASH Framework is free software: you can redistribute it and/or modify
# it under the terms of either the GNU General Public License version 2 or
# the GNU General Public License version 3 as published by the Free
# Software Foundation, whichever suits the most your needs.
#
# The MASH Framework is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public Licenses
# along with the MASH Framework. If not, see <http://www.gnu.org/licenses/>.
################################################################################



class FeatureList(object):
    
    def __init__(self, heuristic, nb_features_total, features):
        self.heuristic          = heuristic
        self.nb_features_total  = nb_features_total
        self.features           = features


class DataParser(object):
    
    def parse(self, data):
        
        results = []

        if data is None:
            return results

        format = '1.0'
        
        # Process each line
        lines = data.split('\n')
        for line in lines:
            
            # Read format version
            if line.startswith('# FORMAT '):
                format = line[9:]
                continue

            # Ignore comments
            if line.startswith('#'):
                continue
            
            # New heuristic entry
            if line.startswith('HEURISTIC'):
                parts = line.split(' ')
                
                if (format == '1.0') and (len(parts) < 3):
                    continue
                
                featurelist = FeatureList(int(parts[1]), int(parts[2]), map(lambda x: int(x), parts[4:]))
                results.append(featurelist)

                continue

        return (format, results)
