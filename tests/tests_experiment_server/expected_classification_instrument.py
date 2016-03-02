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


expected = [
    (1, ['setup']),
    (1, ['onExperimentStarted']),
    (1, ['onClassifierTrainingStarted']),
    (1, ['onFeaturesComputedByClassifier']),    # The classifier only cares about the first features of the first heuristic on the first object
    (100, ['onFeaturesComputedByClassifier', 'onClassifierClassificationDone']),    # Computation of the training error
    (1, ['onClassifierTrainingDone']),
    (1, ['onFeatureListReported']),
    (1, ['onClassifierTestStarted']),
    (100, ['onFeaturesComputedByClassifier', 'onClassifierClassificationDone']),    # Computation of the test error
    (1, ['onClassifierTestDone']),
    (1, ['onExperimentDone']),
]
