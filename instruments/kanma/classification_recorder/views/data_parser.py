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



class ClassificationResult(object):
    
    ERROR_NONE                  = 0
    ERROR_FALSE_ALARM           = 1
    ERROR_FALSE_REJECTION       = 2
    ERROR_WRONG_CLASSIFICATION  = 3
    
    
    def __init__(self, image_index, position_x, position_y, error, label):
        self.image_index    = image_index
        self.position_x     = position_x
        self.position_y     = position_y
        self.error          = error
        self.results        = {}
        self.label          = label

    def _addResult(self, label, score):
        self.results[label] = score
    
    def bestResult(self):
        
        def _mycmp(x, y):
            if x[1] > y[1]:
                return x
            else:
                return y
        
        if len(self.results) > 1:
            return reduce(_mycmp, self.results.items())
        elif len(self.results) == 1:
            return self.results.items()[0]
        else:
            return None


class DataParser(object):
    
    def parse(self, data):
        
        results = []
        current = None

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
            
            # New image entry
            if line.startswith('IMAGE'):
                if current is not None:
                    results.append(current)
                    current = None
                
                parts = line.split(' ')
                
                if ((format == '1.0') and (len(parts) != 5)) or \
                   ((format == '1.1') and (len(parts) != 6)):
                    continue
                
                if parts[4] == 'FA':
                    error = ClassificationResult.ERROR_FALSE_ALARM
                elif parts[4] == 'FR':
                    error = ClassificationResult.ERROR_FALSE_REJECTION
                elif parts[4] == 'WC':
                    error = ClassificationResult.ERROR_WRONG_CLASSIFICATION
                else:
                    error = ClassificationResult.ERROR_NONE
                
                if format == '1.1':
                    if parts[5] == '-':
                        current = ClassificationResult(int(parts[1]), int(parts[2]), int(parts[3]), error, None)
                    else:
                        current = ClassificationResult(int(parts[1]), int(parts[2]), int(parts[3]), error, int(parts[5]))
                else:
                    current = ClassificationResult(int(parts[1]), int(parts[2]), int(parts[3]), error, None)
                
                continue
            
            # New result entry
            if line.startswith('RESULT'):
                if current is None:
                    continue

                parts = line.split(' ')
                if len(parts) != 3:
                    continue

                current._addResult(int(parts[1]), float(parts[2]))

                continue
        
        if current is not None:
            results.append(current)
        
        return (format, results)
