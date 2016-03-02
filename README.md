# MASH Framework 1.1.0

http://github.com/idiap/mash

----------------------------

The *MASH Framework* contains the source code of all the servers in the
"computation farm" of the **MASH project** (http://www.mash-project.eu),
developed at the **Idiap Research Institute** (http://www.idiap.ch).

It is implemented in C++ and Python, and is supported on Linux and MacOS X.

It can be used to develop new "plugins" (heuristics, classifiers, goal-planners
and instruments), in C++.

The MASH SDK is a subset of the Framework, containing only the needed tools to
develop new heuristics.

Note that the denomination "predictor" is used to designate either a classifier
or a goal-planner. Also note that a classifier can be used to solve either image
classification or object detection tasks (ie. the API is the same).


## License

The *MASH Framework* is is made available under either the GPLv2 or the GPLv3
License, whichever suits most your needs. The text of the licenses are in the
files 'COPYING.GPL2' and 'COPYING.GPL3'.

    Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
    Written by Philip Abbet <philip.abbet@idiap.ch>

    The MASH Framework is free software: you can redistribute it and/or modify
    it under the terms of either the GNU General Public License version 2 or
    the GNU General Public License version 3 as published by the Free
    Software Foundation, whichever suits the most your needs.

    The MASH Framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public Licenses
    along with the MASH Framework. If not, see <http://www.gnu.org/licenses/>.


## Content of the Framework

  * Image Server:
          an Application Server providing access to several databases of
          images

  * Maze Server:
          an Interactive Application Server used to move a cursor in a maze

  * GoalPlanning Simulator:
          an Interactive Application Server using recorded datasets to simulate
          other Interactive Application Servers (at the moment, only the Robotic
          Arm Controller)

  * Experiment Server:
          the server running the experiments (can be used as a standalone
          program too)

  * Compilation Server:
          the server used to check that the uploaded heuristics can be
          compiled, don't crash and don't try to do something harmful (to some
          extent)

  * Clustering Server:
          the server used to cluster the heuristics (needs an external
          software to really perform the clustering)

  * mash-core:
          the core library of the system

  * mash-utils:
          some utility classes

  * mash-network:
          a C++ library used by the servers to communicate

  * mash-classification:
          the library containing the image classification and object detection
          classes

  * mash-goalplanning:
          the library containing the goal-planning classes

  * mash-instrumentation:
          the library containing the instrumentation classes

  * mash-sandboxing:
          the library containing the sandboxing classes

  * mash-appserver:
          a C++ library that can be used to easily implement an "Interaction
          Application Server"

  * sandbox:
          an executable used to run "untrusted plugins" in a sandboxed
          environment under strict monitoring

  * pymash:
          a Python implementation of mash-network


## Compilation of the Framework

Note: you'll need CMake 2.6 (or above) to compile the Framework.

    somepath> cd mash
    mash> mkdir build
    mash> cd build
    build> cmake ..
    build> make

    (Optional)
    build> make test

If you want to change one compilation setting, use ccmake instead of cmake.
Warning: if you enable the 'advanced tests', the command 'make test' will take
a while to complete (~25 minutes).

Here we compile the Framework in 'mash/build/', but you can put your build
folder anywhere you want.


If the compilation of the plugins written by someone else fail (or if you don't
care about some plugins), you can tell the Framework to not compile them at all.

Just edit the following files found in your 'mash/build/' folder, and put a '-'
(minus) sign in front of the plugins you don't want to compile:

  * heuristics.txt
  * classifiers.txt
  * classification_libraries.txt
  * goalplanners.txt
  * goalplanning_libraries.txt
  * instruments.txt
  * instrumentation_libraries.txt

When done, run cmake again to let the Framework notify your changes.


## Generalities about plugins development

  * For each kind of plugin, a Python script is provided to simplify the creation
    of a new plugin. You can choose to manually copy the appropriate template file
    instead.

  * It is RECOMMENDED to put your plugins in a separate folder, outside of the
    Framework. This allows you to clone the GIT repositories of the plugins, and
    directly add your plugins in the right place.

    By default, the Framework look for plugins in 'mash/<plugins_type>' and
    'mash/../<plugins_type>'. You can change this behavior by modifying the
    compilation setting MASH_<PLUGINS_TYPE>_LOCATIONS (by using ccmake instead of
    cmake).

  * Each plugin has a name of the form '<username>/<plugin_name>'. Heuristics can
    also be versioned: '<username>/<heuristic_name>/<version>'

  * Each plugin is implemented by subclassing a base class provided by the
    Framework

  * This is a valid file hierarchy for the heuristics:

      heuristics/
        johndoe/
          heuristicA.cpp
          heuristicB.cpp
          heuristicB_v2.cpp

    In that folder, 3 heuristics are implemented:

      * johndoe/heuristicA
      * johndoe/heuristicB
      * johndoe/heuristicB/2

    Note that 'johndoe/heuristicB' is equivalent to 'johndoe/heuristicB/1'.

  * The predictors can be implemented in two ways:

      * a simple predictor (in only one source file)
      * an advanced predictor (in several source files)

    This is a valid file hierarchy for the predictors (classifiers are used for
    this example):

      classifiers/
        johndoe/
          classifierA.cpp
          classifierB/
            classifierB.cpp
            myclass.h
            myclass.cpp

    In that folder, 2 classifiers are implemented:

      * johndoe/classifierA (a simple classifier, in one .cpp file)
      * johndoe/classifierB (an advanced classifier, in two .cpp and one .h
                             files, all contained in a subfolder)

    The name of an advanced predictor is the one of the folder containing all its
    source files. By convention, the source file containing the predictor subclass
    has the same name.

  * The job of an instrument is to write some data about the running experiment.
    The format and content of this data is instrument-specific. The Framework make
    no assumption about it. This data will be analyzed by an external process
    after the end of the experiment, and/or displayed on the website. An instrument
    is notified by the Framework about all the events that happen.

  * The predictors can also write some data for later analysis.

  * The instruments are implemented in three parts:

      * the instrument itself (implemented in several source files)
      * some 'views', which are the scripts used by the website to display the
        data recorded by the instrument
      * (not defined yet) the necessary code to analyze the data recorded by the
        instrument

    This is a valid file hierarchy for the instruments:

      instruments/
        johndoe/
          instrumentA/
            instrument/
              instrumentA.cpp
            views/
              viewsA.py
              viewsB.py

    In that folder, one instrument (johndoe/instrumentA) is implemented, with one
    source file (johndoe/instrumentA/instrument/instrumentA.cpp) and two views
    (in johndoe/instrumentA/views/).

    As in this example, the source code of the instrument MUST be put in an
    'instrument/' subfolder, and the views MUST be put in a 'views/' subfolder.
    The location of the future analyzing code isn't yet defined.

    The name of an instrument is the one of the folder containing all its
    subfolders. By convention, the source file containing the instrument subclass
    has the same name.

  * To facilitate code reuse between several plugins (but not for the heuristics!),
    the Framework support the concept of plugin libraries. See the section 'Plugin
    libraries' below for a detailed explanation.


## Classifier development

More details can be found in classifiers/README

  1) Create a new classifier

      (Note: as recommended above, we put the new classifier in a folder outside
      the Framework)

      Simple classifier (one source file):

          mash> ./scripts/create_classifier.py --path=<PATH_TO_MY_CLASSIFIERS> <USERNAME> <CLASSIFIER_NAME>
          EDIT THE FILE 'PATH_TO_MY_CLASSIFIERS/USERNAME/CLASSIFIER_NAME.cpp'
          mash> cd build
          build> cmake .. (Only the first time, to let the Framework detect the new classifier)
          build> make

      Advanced classifier (several source files):

          mash> ./scripts/create_classifier.py --advanced --path=<PATH_TO_MY_CLASSIFIERS> <USERNAME> <CLASSIFIER_NAME>
          EDIT THE FILE 'PATH_TO_MY_CLASSIFIERS/USERNAME/CLASSIFIER_NAME/CLASSIFIER_NAME.cpp'
          mash> cd build
          build> cmake .. (Only the first time and when you add a new source file in 'PATH_TO_MY_CLASSIFIERS/USERNAME/CLASSIFIER_NAME/')
          build> make

      Note: Use uppercase characters when appropriate for <USERNAME> and
            <CLASSIFIER_NAME>. The generated filenames are lowercase BUT the
            name of the class of your classifier is exactly the same than
            <CLASSIFIER_NAME>.

      Example: ./scripts/create_classifier.py --path=../classifiers JohnDoe AdaBoost

  2) Setup an 'Image Server' (only once)

      Download and extract the image databases you are interested in from the
      MASH website.

      mash> cd application-servers/image-server/
      image-server> cp config_template.py config.py

      EDIT 'config.py':
        - If you don't use a database, set its 'enabled' parameter to 'False'
        - 'url_prefix' must contain the beginning of the URL that the Experiment
          Server will use to download the images. If your two servers run on the
          same machine, you can indicate the path to your copy of the database.
        - 'root_path' must contain the path to your copy of the database.

  3) Start the 'Image Server'

        mash> cd application-servers/image-server/
        image-server> ./image-server.py

      Please note that the Image Server can take a while to launch, depending of
      the databases you use (some of them require a bit of preprocessing to
      speed-up things later). This is what you should get when the Image Server
      is ready (the list of loaded databases depends of the databases enabled in
      your configuration file in step 2):

        ************************************************************************
        * Image Server
        * Protocol: 1.1
        ************************************************************************

        Loaded database 'caltech-256'
        Loaded database 'mini-mnist'
        Loaded database 'mnist'
        Loaded database 'coil-100'

        Ready

      (see ./image-server.py --help for more details)

  4) Setup the 'Experiment Server' (for each different experiment you want to run)

      mash> cd experiment-server/
      experiment-server> cp settings_classification_example.txt mysettings.txt

      EDIT mysettings.txt

  5) Start the 'Experiment Server'

      mash> cd build/bin/
      bin> ./experiment-server --settings=../../experiment-server/mysettings.txt --no-compilation --verbose

      (see ./experiment-server --help for more details)


## Goal-planner development

More details can be found in goalplanners/README

In order to implement a new goal-planner, you need to:

  1) Create a new goal-planner

      (Note: as recommended above, we put the new goal-planner in a folder
      outside the Framework)

      Simple goal-planner (one source file):

          mash> ./scripts/create_planner.py --path=<PATH_TO_MY_CLASSIFIERS> <USERNAME> <PLANNER_NAME>
          EDIT THE FILE 'PATH_TO_MY_CLASSIFIERS/USERNAME/PLANNER_NAME.cpp'
          mash> cd build
          build> cmake .. (Only the first time, to let the Framework detect the new classifier)
          build> make

      Advanced goal-planner (several source files):

          mash> ./scripts/create_planner.py --advanced --path=<PATH_TO_MY_CLASSIFIERS> <USERNAME> <PLANNER_NAME>
          EDIT THE FILE 'PATH_TO_MY_CLASSIFIERS/USERNAME/PLANNER_NAME/PLANNER_NAME.cpp'
          mash> cd build
          build> cmake .. (Only the first time and when you add a new source file in 'PATH_TO_MY_CLASSIFIERS/USERNAME/PLANNER_NAME/')
          build> make

      Note: Use uppercase characters when appropriate for <USERNAME> and
            <PLANNER_NAME>. The generated filenames are lowercase BUT the
            name of the class of your goal-planner is exactly the same than
            <PLANNER_NAME>.

      Example: scripts/create_planner.py --path=../goalplanners JohnDoe MyPlanner

  2) Start the 'Maze Server' or the 'GoalPlanning Simulator'

        Maze Server:

            mash> cd build/bin
            bin> ./maze-server --logfolder=logs/maze-server --host=127.0.0.1 --port=11100 --verbose

            (see ./maze-server --help for more details)

        GoalPlanning Simulator:

            Download and extract the goal-planning datasets you are interested in from the
            MASH website.

            mash> cd application-servers/goalplanning-simulator
            goalplanning-simulator> cp config_template.txt config.txt

            EDIT config.txt

            goalplanning-simulator> cd ../..
            mash> cd build/bin
            bin> ./goalplanning-simulator --host=127.0.0.1 --port=11200 --verbose

            (see ./goalplanning-simulator --help for more details)

  3) Setup the 'Experiment Server' (for each different experiment you want to run)

      mash> cd experiment-server/
      experiment-server> cp settings_goalplanning_example.txt mysettings.txt

      EDIT mysettings.txt

  4) Start the 'Experiment Server'

      mash> cd build/bin/
      bin> ./experiment-server --settings=../../experiment-server/mysettings.txt --no-compilation --verbose

      You can also use the following parameters:

        --rounds=COUNT: Number of tests to perform

      (see ./experiment-server --help for more details)


## Instrument development

More details can be found in instruments/README

  1) Create a new instrument

      (Note: as recommended above, we put the new instrument in a folder outside
      the Framework)

          mash> ./scripts/create_instrument.py --path=<PATH_TO_MY_INSTRUMENTS> <USERNAME> <INSTRUMENT_NAME>
          EDIT THE FILE 'PATH_TO_MY_INSTRUMENTS/USERNAME/INSTRUMENT_NAME/instrument/INSTRUMENT_NAME.cpp'
          mash> cd build
          build> cmake .. (Only the first time and when you add a new source file in 'PATH_TO_MY_INSTRUMENTS/USERNAME/INSTRUMENT_NAME/instrument/')
          build> make

      Note: Use uppercase characters when appropriate for <USERNAME> and
            <INSTRUMENT_NAME>. The generated filenames are lowercase BUT the
            name of the class of your instrument is exactly the same than
            <INSTRUMENT_NAME>.

      Example: ./scripts/create_instrument.py --path=../instruments JohnDoe MyInstrument

  2) Modify the settings file of your experiment to use your instrument (see the
     'Classifier development' and 'Goal-planner development' sections to learn
     how to launch an experiment)

  3) Run your experiment. The data written by the instruments is put in
     'mash/build/bin/out'. You can change this location by using the
     --outputfolder parameter.


## Heuristics development

  1) Create a new heuristic

      (Note: as recommended above, we put the new heuristic in a folder outside
      the Framework)

          mash> ./scripts/create_heuristic.py --path=<PATH_TO_MY_HEURISTICS> <USERNAME> <HEURISTIC_NAME>
          EDIT THE FILE 'PATH_TO_MY_HEURISTICS/USERNAME/HEURISTIC_NAME/instrument/HEURISTIC_NAME.cpp'
          mash> cd build
          build> cmake .. (Only the first time, to let the Framework detect the new heuristic)
          build> make

      Note: Use uppercase characters when appropriate for <USERNAME> and
            <HEURISTIC_NAME>. The generated filenames are lowercase BUT the
            name of the class of your heuristic is exactly the same than
            <HEURISTIC_NAME>.

      Example: ./scripts/create_heuristic.py --path=../heuristics JohnDoe MyHeuristic

  2) Modify the settings file of your experiment to use your heuristic (see the
     'Classifier development' and 'Goal-planner development' sections to learn
     how to launch an experiment)

  3) Run your experiment


## Plugin libraries

To facilitate code reuse between plugins of the same type (but not the
heuristics!), you can create libraries that are useable from any plugin of that
type (and from the other libraries too).

As an example, let's take the following goal-planners folder:

  goalplanners/
    johndoe/
      goalplannerA.cpp
      goalplannerB.cpp
    libraries/
      library1/
        library1.h
        library1.cpp
      library2/
        library2.h
        library2.cpp
        utils.cpp
        tests/
          test.cpp

It contains:

  * 2 goal-planners:

    - johndoe/goalplannerA (a simple goal-planner)
    - johndoe/goalplannerB (a simple goal-planner)

  * 2 libraries:

    - library1 (one .cpp file)
    - library2 (two .cpp files, ignore the 'tests/' subfolder for now)


If 'johndoe/goalplannerA' wants to use the functions defined in 'library1' (for
instance), it just needs to #include the appropriate header file. In this case:

    #include <library1/library1.h>

That's it. The Framework will take care of the rest.


What about the 'tests/' subfolder of 'library2'? This is a special convention:
the Framework will not attempt to compile the source files found in any 'tests/'
subfolder of a library.

You can then put whatever test programs you want in there, to ensure that your
library behaves correctly. BUT you are responsible to compile them yourself
(using CMake, Makefiles, shell scripts, ...).


All the other source files found in the folder of a library (and its subfolders)
will be compiled by the Framework.
