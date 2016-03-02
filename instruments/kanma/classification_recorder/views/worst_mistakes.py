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
from data_parser import DataParser, ClassificationResult
from pymash import DataReport


INSTRUMENT_NAME = 'kanma/classification_recorder'


class Mistake:

    def __init__(self, label, false_positives, false_negatives):
        self.label           = label
        self.false_positives = false_positives
        self.false_negatives = false_negatives


def output_images(mistakes, report, options, nb_max):
    html = '<div class="images">\n'

    width = 0
    nb = 0
    for mistake in mistakes:
        image = report.image(mistake.image_index)

        width += image.width
        nb += 1

        if (width > 800) or (nb > (nb_max / 2)):
            html += '</div>\n<div class="images">\n'
            width = image.width
            nb = 1

        if options.database_url_prefix is None:
            url = image.url()
        else:
            url = os.path.join(options.database_url_prefix, image.name)

        html += '<image src="%s" width="%d" height="%d">\n' % (url, image.width, image.height)

    html += '</div>\n'

    return html


def output_snippet(html_snippet, options, output_folder):
    html = ''

    if options.dev:
        html += """<html>
    <head>
        <link rel="stylesheet" href="worst_mistakes.css" />
    </head>
    <body style="font-family: Arial;">
        <div id="kanma_classification_recorder_worst_mistakes" style="width: 864px;">
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
parser.add_option("--database-url-prefix", action="store", dest="database_url_prefix", default=None,
                  help="Prefix for the URLs of the images")


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
(data_format, classification_results) = parser.parse(report.data(INSTRUMENT_NAME))


# Retrieve the classification errors
errors = filter(lambda x: (x.error != ClassificationResult.ERROR_NONE) and (len(x.results) > 0) and \
                          (report.image(x.image_index).test_index is not None),
                classification_results)

if len(errors) == 0:
    output_snippet('<p class="explanation">No mistakes found</p>', options, output_folder)
    sys.exit(0)


# Retrieve the number of mistakes to display
try:
    nb_mistakes_max = int(report.getInstrumentSetting(INSTRUMENT_NAME, 'VIEW_WORST_MISTAKES_NB_MAX'))
except:
    nb_mistakes_max = 10


# Complete the data if it was saved in an old format
if data_format != '1.1':
    label_names = []
    for label in range(0, report.nbLabels()):
        label_names.append(report.labelName(label))

    database_name = report.getExperimentSettingDatabaseName()

    for error in errors:
        image_name = report.image(error.image_index).name

        if (database_name == 'mini-mnist') or (database_name == 'mnist'):
            parts = image_name.split('/')
            error.label = label_names.index(parts[1])
        elif database_name == 'caltech-256':
            parts = image_name.split('/')
            s = parts[1]
            s = s[s.find('.')+1:]
            error.label = label_names.index(s)
        elif database_name == 'coil-100':
            error.label = label_names.index(str(int(image_name[3:6].replace('_', '')) - 1))
        elif database_name == 'mugs':
            parts = image_name.split('/')
            error.label = int(parts[1])


# Process each label
worst_mistakes = []
nb_labels = report.nbLabels()
for label in range(0, nb_labels):

    # Retrieve the false positives and sort them
    false_positives = filter(lambda x: x.bestResult()[0] == label, errors)
    false_positives.sort(cmp=lambda x, y: cmp(x.bestResult()[1], y.bestResult()[1]))

    # Retrieve the false negatives and sort them
    false_negatives = filter(lambda x: (x.bestResult()[0] != label) and (x.label == label), errors)
    false_negatives.sort(cmp=lambda x, y: cmp(x.bestResult()[1], y.bestResult()[1]))

    if (len(false_positives) > 0) or (len(false_negatives) > 0):
        worst_mistakes.append(Mistake(label, false_positives[0:nb_mistakes_max], false_negatives[0:nb_mistakes_max]))


# Create the output file
html = ''

for index, mistake in enumerate(worst_mistakes):
    if index > 0:
        html += '<div class="separator"></div>\n'

    html += '<div class="label">\n'
    html += '<p class="label">Label <span class>%s</span></p>\n' % report.labelName(mistake.label)

    if len(mistake.false_positives) > 0:
        html += '<p class="section">Worst false positives</p>\n'
        html += output_images(mistake.false_positives, report, options, nb_mistakes_max)

    if len(mistake.false_negatives) > 0:
        html += '<p class="section">Worst false negatives</p>\n'
        html += output_images(mistake.false_negatives, report, options, nb_mistakes_max)

    html += '</div>\n'

output_snippet(html, options, output_folder)
