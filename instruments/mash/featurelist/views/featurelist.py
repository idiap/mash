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


import os
import sys
from optparse import OptionParser
from data_parser import DataParser, FeatureList
from pymash import DataReport


INSTRUMENT_NAME = 'mash/featurelist'


def output_snippet(html_snippet, options, output_folder):
    html = ''

    if options.dev:
        html += """<html>
    <head>
        <link rel="stylesheet" href="featurelist.css" />
    </head>
    <body style="font-family: Arial;">
        <div id="mash_featurelist_featurelist" style="width: 864px;">
"""

    html += html_snippet

    if options.dev:
        html += '</div>\n'
        html += '</body></html>\n'

    outFile = open(os.path.join(output_folder, 'snippet.html'), 'wb')
    outFile.write(html)
    outFile.close()



# Setup of the command-line arguments parser
parser = OptionParser("Usage: %prog [options] <data_folder> <output_folder> <url_prefix>", version="%prog 1.0")
parser.add_option("--dev", action="store_true", dest="dev", default=False,
                  help="Development mode")


# Handling of the arguments
(options, args) = parser.parse_args()
if (len(args) != 3):
    parser.print_help()
    sys.exit(1)

data_folder = args[0]
output_folder = args[1]


# Import the data report
report = DataReport()
if not(report.parse(data_folder)):
    print 'ERROR: Failed to parse the data reports'
    sys.exit(1)


# Parse the data written by the instrument
parser = DataParser()
(data_format, features) = parser.parse(report.data(INSTRUMENT_NAME))

if len(features) == 0:
    output_snippet('<p class="explanation">No feature used</p>', options, output_folder)
    sys.exit(0)


# Retrieve the name of each heuristic and sort them
def _getName(featurelist):
    featurelist.heuristic_name = report.heuristic(featurelist.heuristic)
    return featurelist

features = map(_getName, features)
features.sort(cmp=lambda x, y: cmp(x.heuristic_name, y.heuristic_name))


# Create the output file
html = '<table><tbody>\n'

for index, featurelist in enumerate(features):
    html += '<tr><td><a href="/heuristics/view/%s">%s</a></td><td class="number">%d</td><td class="total">/%d</td></tr>\n' % (featurelist.heuristic_name, featurelist.heuristic_name, len(featurelist.features), featurelist.nb_features_total)

html += '</tbody></table>\n'

if len(features) < report.nbHeuristics():
    html += '<p class="explanation spacing">The other heuristics weren\'t used by the predictor</p>\n'

output_snippet(html, options, output_folder)
