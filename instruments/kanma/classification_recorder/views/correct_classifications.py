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


class LabelInfo:
    
    def __init__(self, label):
        self.label                  = label
        self.nb_correct_detections  = 0
        self.nb_objects             = 0


def output_snippet(html_snippet, options, output_folder):
    html = ''

    if options.dev:
        html += """<html>
    <head>
        <link rel="stylesheet" href="correct_classifications.css" />
        <script type="text/javascript" src="../../common/js/jquery.js" ></script>
        <script type="text/javascript" src="../../common/js/rgraph/RGraph.common.core.js" ></script>
        <script type="text/javascript" src="../../common/js/rgraph/RGraph.common.tooltips.js"></script>
        <script type="text/javascript" src="../../common/js/rgraph/RGraph.hbar.js" ></script>
        <!--[if IE 8]><script type="text/javascript" src="../../common/js/excanvas.min.js"></script><![endif]-->
    </head>
    <body style="font-family: Arial;">
        <div style="border-bottom: 1px solid #007000; color: #007000; font-size: 16px; font-weight: bold; width: 864px;">- Correct classifications</div>
        <div id="kanma_classification_recorder_correct_classifications" style="width: 864px; padding-top: 15px;">
"""

    html += html_snippet

    if options.dev:
        html += """</div>\n
        <script language="javascript">
            $(document).ready(function(){
                start_kanma_classification_recorder_correct_classifications();
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
    objects = filter(lambda x: x.label == label, classification_results)
    
    info = LabelInfo(report.labelName(label))
    info.nb_objects = len(objects)
    info.nb_correct_detections = len(filter(lambda x: x.bestResult()[0] == label, objects))
    
    label_infos.append(info)


# Create the output file
html = """<a id="help_link" href="#">Display help</a>

<div id="help" style="display: none;">
    <p class="explanation">This chart shows the number of images correctly classified for each label.</p>
    <p class="explanation">Move your mouse over the bars to see how much images this represents.</p>
</div>

<canvas id="kanma_correct_classifications_canvas" width="864" height="%d">
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
function start_kanma_classification_recorder_correct_classifications()
{
    var data = [%s];

    var hbar = new RGraph.HBar('kanma_correct_classifications_canvas', data);
    hbar.Set('chart.labels', [%s]);
    hbar.Set('chart.labels.above', true);
    hbar.Set('chart.labels.above.decimals', 2);
	hbar.Set('chart.text.font', 'Arial');
	hbar.Set('chart.text.size', 12);
    hbar.Set('chart.units.ingraph', true);
    hbar.Set('chart.xlabels', false);
    hbar.Set('chart.vmargin', 6);
    hbar.Set('chart.xmax', 100);
    hbar.Set('chart.xmin', 0);
    hbar.Set('chart.units.post', '%%');
    hbar.Set('chart.background.grid', true);
    hbar.Set('chart.colors', ['#00A000']);
    hbar.Set('chart.shadow', true);
    hbar.Set('chart.background.grid.autofit', true);
    hbar.Set('chart.background.grid.autofit.numvlines', 10);
    hbar.Set('chart.background.grid.autofit.numhlines', 0);
    hbar.Set('chart.tooltips', [%s]);
    hbar.Set('chart.tooltips.event', 'onmousemove');

	hbar.context.font="12pt Arial";
	var textWidth = hbar.context.measureText("%s");

    hbar.Set('chart.gutter.left', textWidth.width + 10);
    hbar.Set('chart.gutter.right', 10);
    hbar.Set('chart.gutter.top', 10);
    hbar.Set('chart.gutter.bottom', 10);

    hbar.Draw();


    $('#kanma_classification_recorder_correct_classifications #help_link').click(function() {
        var text = $(this).text();
        if (text == 'Display help')
            $(this).text('Hide help');
        else
            $(this).text('Display help');
        
        $('#kanma_classification_recorder_correct_classifications #help').slideToggle('fast');
        
        return false;
    });
}
</script>
""" % (nb_labels * 40,
       ', '.join(map(lambda x: '%.2f' % (100.0 * x.nb_correct_detections / x.nb_objects), label_infos)),
       ', '.join(map(lambda x: "'%s'" % x.label, label_infos)),
       ', '.join(map(lambda x: "'%d/%d'" % (x.nb_correct_detections, x.nb_objects), label_infos)),
       reduce(longer_label, label_infos).label
      )

output_snippet(html, options, output_folder)
