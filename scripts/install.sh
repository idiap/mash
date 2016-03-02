#! /bin/bash


function install_experiment_server {

    if [ -d $1 ]
    then
        echo "Experiment Server '$1' already installed"
    else
        echo "Installing Experiment Server '$1'..."

        mkdir $1
        chown experiment-server:experiment-server $1

        cd $1

        mkdir build
        mkdir heuristics
        mkdir jail
        mkdir logs
        mkdir out
        mkdir repositories
        mkdir temp

        chmod 777 jail

        chown experiment-server:experiment-server build
        chown experiment-server:experiment-server heuristics
        chown experiment-server:experiment-server jail
        chown experiment-server:experiment-server logs
        chown experiment-server:experiment-server out
        chown experiment-server:experiment-server repositories
        chown experiment-server:experiment-server temp

        chmod 777 out

        cp /mash-build/bin/coredump_rights_changer .
        cp /mash-build/bin/experiment-server .
        cp /mash-build/bin/lib* .
        cp /mash-build/bin/sandbox .

        cp -R /mash-build/bin/classifiers .
        cp -R /mash-build/bin/goalplanners .
        cp -R /mash-build/bin/instruments .

        cp /mash-framework/experiment-server/manage.py .

        cp /mash-framework/sandbox/coredump_analyzer.py .
        cp /mash-framework/sandbox/get_stackframe.cmd.template .
        cp /mash-framework/sandbox/get_stacktrace.cmd .

        mkdir heuristics_cmake
        cp /mash-framework/experiment-server/heuristics_cmake/CMakeLists.txt heuristics_cmake/
        cp /mash-framework/experiment-server/heuristics_cmake/CMakeDeclarations.install heuristics_cmake/CMakeDeclarations.txt

        mkdir mash
        cp /mash-framework/mash/dynlibs_manager.h /mash-framework/mash/heuristic.h /mash-framework/mash/heuristics_manager.h /mash-framework/mash/image.h /mash-framework/mash/imageutils.h mash/

        mkdir mash-utils
        cp /mash-framework/mash-utils/arguments_list.h /mash-framework/mash-utils/declarations.h /mash-framework/mash-utils/errors.h /mash-framework/mash-utils/platform.h mash-utils/

        chown -R experiment-server:experiment-server .

        chown root:root coredump_rights_changer
        chmod +s coredump_rights_changer

        chown root:sandboxed sandbox
        chmod +s sandbox

        cd ..
    fi
}


function install_compilation_server {
    
    if [ -d $1 ]
    then
        echo "Compilation Server '$1' already installed"
    else
        echo "Installing Compilation Server '$1'..."

        mkdir $1
        chown compilation-server:compilation-server $1

        cd $1

        mkdir build
        mkdir heuristics
        mkdir jail
        mkdir logs
        mkdir repositories
        mkdir temp

        chmod 777 jail

        chown compilation-server:compilation-server build
        chown compilation-server:compilation-server heuristics
        chown compilation-server:compilation-server jail
        chown compilation-server:compilation-server logs
        chown compilation-server:compilation-server repositories
        chown compilation-server:compilation-server temp

        cp /mash-build/bin/checkheuristic .
        cp /mash-build/bin/coredump_rights_changer .
        cp /mash-build/bin/lib* .
        cp /mash-build/bin/sandbox .

        cp /mash-framework/compilation-server/compilation-server.py .
        cp /mash-framework/compilation-server/config_template.py .

        cp /mash-framework/sandbox/coredump_analyzer.py .
        cp /mash-framework/sandbox/get_stackframe.cmd.template .
        cp /mash-framework/sandbox/get_stacktrace.cmd .

        mkdir data
        cp /mash-framework/data/*.* data/

        mkdir heuristics_cmake
        cp /mash-framework/compilation-server/heuristics_cmake/CMakeLists.txt heuristics_cmake/
        cp /mash-framework/compilation-server/heuristics_cmake/CMakeDeclarations.install heuristics_cmake/CMakeDeclarations.txt

        mkdir mash
        cp /mash-framework/mash/dynlibs_manager.h /mash-framework/mash/heuristic.h /mash-framework/mash/heuristics_manager.h /mash-framework/mash/image.h /mash-framework/mash/imageutils.h mash/

        mkdir mash-utils
        cp /mash-framework/mash-utils/arguments_list.h /mash-framework/mash-utils/declarations.h /mash-framework/mash-utils/errors.h /mash-framework/mash-utils/platform.h mash-utils/

        mkdir pymash
        cp /mash-framework/pymash/*.py pymash/

        if [ -f /mash-current/config/compilation-server-config.py ]
        then
            cp /mash-current/config/compilation-server-config.py config.py
        fi

        chown -R compilation-server:compilation-server .

        chown root:root coredump_rights_changer
        chmod +s coredump_rights_changer

        chown root:sandboxed sandbox
        chmod +s sandbox

        cd ..
    fi
}


# Compilation Servers
install_compilation_server compilox1
install_compilation_server compilox2
install_compilation_server compilox3


# Experiment Servers
install_experiment_server experimentix-eval-1
install_experiment_server experimentix-eval-2
install_experiment_server experimentix-eval-3
install_experiment_server experimentix-priv-1
install_experiment_server experimentix-priv-2
install_experiment_server experimentix-priv-3
install_experiment_server experimentix-priv-4
install_experiment_server experimentix-priv-5
install_experiment_server experimentix-cons-1
install_experiment_server experimentix-cons-2
install_experiment_server experimentix-cons-3
install_experiment_server experimentix-publ-1
install_experiment_server experimentix-cont-1
install_experiment_server experimentix-cont-2
install_experiment_server experimentix-cont-3
install_experiment_server experimentix-1
install_experiment_server experimentix-2
install_experiment_server experimentix-test


# Image Server
if [ -d image-server ]
then
    echo "Image Server already installed"
else
    echo "Installing Image Server..."

    mkdir image-server
    chown image-server:image-server image-server
    
    cd image-server

    mkdir logs

    chown image-server:image-server logs

    cp /mash-framework/application-servers/image-server/*.py .

    mkdir pymash
    cp /mash-framework/pymash/*.py pymash/
    
    if [ -f /mash-current/config/image-server-config.py ]
    then
        cp /mash-current/config/image-server-config.py config.py
    fi

    chown -R image-server:image-server .
        
    cd ..
fi


# Maze Server
if [ -d maze-server ]
then
    echo "Maze Server already installed"
else
    echo "Installing Maze Server..."

    mkdir maze-server
    chown maze-server:maze-server maze-server
    
    cd maze-server

    mkdir logs

    chown maze-server:maze-server logs

    cp /mash-build/bin/maze-server .
    cp /mash-build/bin/libmash-network.so .
    cp /mash-build/bin/libmash-utils.so .

    chown -R maze-server:maze-server .
        
    cd ..
fi


# Goal-planning Simulator
if [ -d goalplanning-simulator ]
then
    echo "Goal-planning Simulator already installed"
else
    echo "Installing Goal-planning Simulator..."

    mkdir goalplanning-simulator
    chown goalplanning-simulator:goalplanning-simulator goalplanning-simulator
    
    cd goalplanning-simulator

    mkdir logs

    chown goalplanning-simulator:goalplanning-simulator logs

    cp /mash-build/bin/goalplanning-simulator .
    cp /mash-build/bin/libmash-network.so .
    cp /mash-build/bin/libmash-utils.so .

    cp /mash-framework/application-servers/goalplanning-simulator/config_template.txt .

    if [ -f /mash-current/config/goalplanning-simulator-config.txt ]
    then
        cp /mash-current/config/goalplanning-simulator-config.txt config.txt
    fi

    chown -R goalplanning-simulator:goalplanning-simulator .
        
    cd ..
fi
