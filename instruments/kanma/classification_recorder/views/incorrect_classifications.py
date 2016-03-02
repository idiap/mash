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


class IncorrectDetection:

    def __init__(self, label_name, nb_errors):
        self.label_name = label_name
        self.nb_errors  = nb_errors


class LabelInfo:
    
    def __init__(self, label, nb_labels):
        self.label                         = label
        self.nb_total_incorrect_detections = 0
        self.incorrect_detections          = []
    
    def process(self):
        self.incorrect_detections.sort(cmp=lambda x, y: cmp(x.nb_errors, y.nb_errors))
        
        to_group = filter(lambda x: (float(x.nb_errors) / self.nb_total_incorrect_detections) < .01, self.incorrect_detections)
        if len(to_group) > 1:
            self.incorrect_detections = filter(lambda x: x not in to_group, self.incorrect_detections)
            nb_grouped_errors = reduce(lambda x, y: x + y.nb_errors, to_group, 0)
            grouped_errors_rate = float(nb_grouped_errors) / self.nb_total_incorrect_detections
            self.incorrect_detections.append(IncorrectDetection('others (%.2f%%)' % grouped_errors_rate, nb_grouped_errors))
    
    def incorrect_detections_list(self):
        return '[%s]' % ', '.join(map(lambda x: '%.2f' % (100.0 * x.nb_errors / self.nb_total_incorrect_detections), self.incorrect_detections))

    def labels(self):
        return '[%s]' % ', '.join(map(lambda x: "'%s (%.2f%%)'" % (x.label_name, (100.0 * x.nb_errors / self.nb_total_incorrect_detections)), self.incorrect_detections))

    def tooltips(self):
        return '[%s]' % ', '.join(map(lambda x: "'%d/%d'" % (x.nb_errors, self.nb_total_incorrect_detections), self.incorrect_detections))


def output_snippet(html_snippet, options, output_folder):
    html = ''

    if options.dev:
        html += """<html>
    <head>
        <link rel="stylesheet" href="incorrect_classifications.css" />
        <script type="text/javascript" src="../../common/js/jquery.js" ></script>
        <script type="text/javascript" src="../../common/js/rgraph/RGraph.common.core.js" ></script>
        <script type="text/javascript" src="../../common/js/rgraph/RGraph.common.tooltips.js"></script>
        <script type="text/javascript" src="../../common/js/rgraph/RGraph.pie.js" ></script>
        <!--[if IE 8]><script type="text/javascript" src="../../common/js/excanvas.min.js"></script><![endif]-->
    </head>
    <body style="font-family: Arial;">
        <div style="border-bottom: 1px solid #007000; color: #007000; font-size: 16px; font-weight: bold; width: 864px;">- Incorrect classifications</div>
        <div id="kanma_classification_recorder_incorrect_classifications" style="width: 864px; padding-top: 15px;">
"""

    html += html_snippet

    if options.dev:
        html += """</div>\n
        <script language="javascript">
            $(document).ready(function(){
                start_kanma_classification_recorder_incorrect_classifications();
            });
        </script>
    </body>
</html>
"""

    outFile = open(os.path.join(output_folder, 'snippet.html'), 'wb')
    outFile.write(html)
    outFile.close()


def longer_label(x, y):
    if len(x.label) > len(y.label):
        return x
    else:
        return y


# Setup of the command-line arguments parser
parser = OptionParser("Usage: %prog [options] <data_folder> <output_folder> <url_prefix>", version="%prog 1.0")
parser.add_option("--dev", action="store_true", dest="dev", default=False,
                  help="Development mode")


# Handling of the arguments
(options, args) = parser.parse_args()
if (len(args) != 3):
    parser.print_help()
    sys.exit(1)

data_folder   = args[0]
output_folder = args[1]


# Import the data report
report = DataReport()
if not(report.parse(data_folder)):
    print 'ERROR: Failed to parse the data report'
    sys.exit(1)


# Parse the data written by the instrument
parser = DataParser()
(data_format, classification_results) = parser.parse(report.data(INSTRUMENT_NAME))


# Only keep the images from the test set
classification_results = filter(lambda x: report.image(x.image_index).test_index is not None, classification_results)


# Complete the data if it was saved in an old format
if data_format != '1.1':
    label_names = []
    for label in range(0, report.nbLabels()):
        label_names.append(report.labelName(label))

    database_name = report.getExperimentSettingDatabaseName()

    for classification_result in classification_results:
        image_name = report.image(classification_result.image_index).name

        if (database_name == 'mini-mnist') or (database_name == 'mnist'):
            parts = image_name.split('/')
            classification_result.label = label_names.index(parts[1])
        elif database_name == 'caltech-256':
            parts = image_name.split('/')
            s = parts[1]
            s = s[s.find('.')+1:]
            classification_result.label = label_names.index(s)
        elif database_name == 'coil-100':
            classification_result.label = label_names.index(str(int(image_name[3:6].replace('_', '')) - 1))
        elif database_name == 'mugs':
            parts = image_name.split('/')
            classification_result.label = int(parts[1])
    

# Process each label
label_infos = []
nb_labels = report.nbLabels()
for label in range(0, nb_labels):
    objects = filter(lambda x: (x.label == label) and (x.error != ClassificationResult.ERROR_NONE), classification_results)
    
    info = LabelInfo(report.labelName(label), nb_labels)
    info.nb_total_incorrect_detections = len(objects)

    if info.nb_total_incorrect_detections > 0:
        for lbl in range(0, nb_labels):
            det = IncorrectDetection(report.labelName(lbl), len(filter(lambda x: x.bestResult()[0] == lbl, objects)))
            if det.nb_errors > 0:
                info.incorrect_detections.append(det)
    
        info.process()
    
        label_infos.append(info)

if len(label_infos) == 0:
    html = """
<div id="help">
    <p class="explanation">All the images were correctly classified during this experiment</p>
</div>
"""
    output_snippet(html, options, output_folder)
    sys.exit(0)
    

# Create the output file
html = """<a id="help_link" href="#">Display help</a>

<div id="help" style="display: none;">
    <p class="explanation">This chart shows the number of images incorrectly classified for each label.</p>
    <p class="explanation">Move your mouse over the bars to see how much images this represents.</p>
</div>

<div>
    <span>View incorrect classifications for label:</span>
    <select id="label_selector">
"""

for index, label_info in enumerate(label_infos):
    additional = ''
    if index == 0:
        additional = ' selected="selected"'
    
    html += '<option value="%d"%s>%s</option>' % (index, additional, label_info.label)

html += """    </select>
</div>

<canvas id="kanma_incorrect_classifications_canvas" width="864" height="500">
    To view this chart, you need a modern browser (supporting the canvas tag), like:
    <ul>
        <li>Mozilla Firefox 3.0+</li>
        <li>Google Chrome 1+</li>
        <li>Apple Safari 3+</li>
        <li>Opera 9.5+</li>
        <li>Microsoft Internet Explorer 8+</li>
        <li>iPhone: iOS v4+</li>
        <li>iPad: iOS v4.2+</li>
    </ul>
</canvas>

<script type="text/javascript">

function start_kanma_classification_recorder_incorrect_classifications()
{
    function LabelInfo()
    {
        this.data = null;
        this.labels = null;
        this.colors = null;
        this.tooltips = null;
    }

    var label_infos = new Array();

"""

color_names = ['rgb(255,0,0)', '#ddd', 'rgb(0,255,0)', 'rgb(0,0,255)', 'rgb(255,255,0)', 'rgb(0,255,255)', 'pink', 'white']

for index, label_info in enumerate(label_infos):
    colors = (color_names * (1 + len(label_infos) / len(color_names)))[0:len(label_info.incorrect_detections)]
    if (len(colors) > 1) and (colors[-1] == colors[0]):
        colors = colors[0:-1]
        colors.append(colors[1])
    
    html += "label_info = new LabelInfo();\n"
    html += "label_info.data = %s;\n" % label_info.incorrect_detections_list()
    html += "label_info.labels = %s;\n" % label_info.labels()
    html += "label_info.colors = %s;\n" % str(colors)
    html += "label_info.tooltips = %s;\n" % label_info.tooltips()
    html += "label_infos[%d] = label_info;\n\n" % index


html += """

    function kanma_classification_recorder_incorrect_classifications_display(label_info)
    {
        var pie = new RGraph.Pie('kanma_incorrect_classifications_canvas', label_info.data);
        pie.Set('chart.labels', label_info.labels);
        pie.Set('chart.labels.sticks', true);
        pie.Set('chart.linewidth', 1);
        pie.Set('chart.stroke', '#000000');
    	pie.Set('chart.highlight.style', '2d');
        pie.Set('chart.colors', label_info.colors);
        pie.Set('chart.shadow', true);
       	pie.Set('chart.tooltips', label_info.tooltips);
        pie.Set('chart.tooltips.event', 'onmousemove');
        pie.Draw();
    }

    kanma_classification_recorder_incorrect_classifications_display(label_infos[$('#kanma_classification_recorder_incorrect_classifications #label_selector').val()]);

    $('#kanma_classification_recorder_incorrect_classifications #help_link').click(function() {
        var text = $(this).text();
        if (text == 'Display help')
            $(this).text('Hide help');
        else
            $(this).text('Display help');
        
        $('#kanma_classification_recorder_incorrect_classifications #help').slideToggle('fast');
        
        return false;
    });

    $('#kanma_classification_recorder_incorrect_classifications #label_selector').change(function() {
        kanma_classification_recorder_incorrect_classifications_display(label_infos[$(this).val()]);
    });
}
</script>
"""

output_snippet(html, options, output_folder)
