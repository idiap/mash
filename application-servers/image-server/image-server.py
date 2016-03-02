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


################################################################################
#                                                                              #
# Image Server                                                                 #
#                                                                              #
# This application server provides access to databases of images.              #
#                                                                              #
################################################################################

from pymash import Server, ServerListener, OutStream, Message
import sys
import os
import traceback
from optparse import OptionParser
from database import Database, DatabaseException


PROTOCOL = '1.2'


############################### SERVER LISTENER ################################

#-------------------------------------------------------------------------------
# The listener that will handle the incoming connections
#-------------------------------------------------------------------------------
class ImageServerListener(ServerListener):

    # Class attributes
    databases = None
    logFolder = 'logs'
    
    handlers = {
        'STATUS':                       'handleStatusCommand',
        'INFO':                         'handleInfoCommand',
        'DONE':                         'handleDoneCommand',
        'LOGS':                         'handleLogsCommand',
        'RESET':                        'handleResetCommand',

        'USE_GLOBAL_SEED':              'handleUseGlobalSeedCommand',

        'LIST_DATABASES':               'handleListDatabasesCommand',
        'SELECT_DATABASE':              'handleSelectDatabaseCommand',

        'LIST_LABEL_NAMES':             'handleListLabelNamesCommand',
        'ENABLE_LABELS':                'handleEnableLabelsCommand',
        'RESET_ENABLED_LABELS':         'handleResetEnabledLabelsCommand',

        'ENABLE_BACKGROUND_IMAGES':     'handleEnableBackgroundImagesCommand',
        'DISABLE_BACKGROUND_IMAGES':    'handleDisableBackgroundImagesCommand',
        
        'REPORT_PREFERRED_IMAGE_SIZE':  'handleReportPreferredImageSizeCommand',
        'REPORT_PREFERRED_ROI_SIZE':    'handleReportPreferredRoiSizeCommand',

        'ENABLE_PREPROCESSED_IMAGES':   'handleEnablePreprocessedImagesCommand',
        'DISABLE_PREPROCESSED_IMAGES':  'handleDisablePreprocessedImagesCommand',
        
        'LIST_OBJECTS':                 'handleListObjectsCommand',

        'REPORT_URL_PREFIX':            'handleReportUrlPrefixCommand',
        'IMAGE':                        'handleImageCommand',
    }
    
    #---------------------------------------------------------------------------
    # Constructor
    #
    # @param socket     The socket on which the connection is established
    #---------------------------------------------------------------------------
    def __init__(self, socket, channel):
        super(ImageServerListener, self).__init__(socket, channel)
        self.database                   = None
        self.enabledLabels              = None
        self.enabledImages              = None
        self.backgroundImagesEnabled    = True
        self.outStream.open('ImageServer', os.path.join(ImageServerListener.logFolder, 'listener-$TIMESTAMP.log'))
    
    #---------------------------------------------------------------------------
    # Called when a command was received
    #
    # @param command    The command
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleCommand(self, command):
        try:
            method = self.__getattribute__(ImageServerListener.handlers[command.name])
            return method(command.parameters)
        except:
            self.outStream.write('--------------------------------------------------------------------------------\n')
            self.outStream.write("Error while processing the command '%s'\n" % command.toString())
            self.outStream.write(traceback.format_exc())
            self.outStream.write('\n')
            
            if not(self.sendResponse(Message('UNKNOWN_COMMAND', [command.name]))):
                return ServerListener.ACTION_CLOSE_CONNECTION
            
            return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'STATUS' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleStatusCommand(self, arguments):
        if not(self.sendResponse('READY')):
            return ServerListener.ACTION_CLOSE_CONNECTION
            
        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'INFO' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleInfoCommand(self, arguments):
        if not(self.sendResponse(Message('TYPE', ['ApplicationServer']))):
            return ServerListener.ACTION_CLOSE_CONNECTION
            
        if not(self.sendResponse(Message('SUBTYPE', ['Images']))):
            return ServerListener.ACTION_CLOSE_CONNECTION
            
        if not(self.sendResponse(Message('PROTOCOL', [PROTOCOL]))):
            return ServerListener.ACTION_CLOSE_CONNECTION
            
        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'DONE' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleDoneCommand(self, arguments):
        self.sendResponse('GOODBYE')
        return ServerListener.ACTION_CLOSE_CONNECTION

    #---------------------------------------------------------------------------
    # Called when a 'LOGS' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleLogsCommand(self, arguments):
        content = self.outStream.dump(200 * 1024)
        if content is not None:
            self.sendResponse(Message('LOG_FILE', ['ImageServer.log', len(content)]))
            self.sendData(content)

        if not(self.sendResponse('END_LOGS')):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'RESET' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleResetCommand(self, arguments):
        self.database                   = None
        self.enabledLabels              = None
        self.enabledImages              = None
        self.backgroundImagesEnabled    = True
        
        self.sendResponse('OK')
        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'USE_GLOBAL_SEED' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleUseGlobalSeedCommand(self, arguments):
        if len(arguments) != 1:
            if not(self.sendResponse(Message('INVALID_ARGUMENTS', arguments))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        self.sendResponse('OK')
        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'LIST_DATABASES' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleListDatabasesCommand(self, arguments):
        for db_name in ImageServerListener.databases.keys():
            self.sendResponse(Message('DATABASE', [db_name]))
        
        if not(self.sendResponse('END_LIST_DATABASES')):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'SELECT_DATABASE' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleSelectDatabaseCommand(self, arguments):
        if len(arguments) != 1:
            if not(self.sendResponse(Message('INVALID_ARGUMENTS', arguments))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        if ImageServerListener.databases.has_key(arguments[0]):
            self.database = ImageServerListener.databases[arguments[0]]
            if not(self.sendResponse(Message('NB_IMAGES', [self.database.imagesCount()]))):
                return ServerListener.ACTION_CLOSE_CONNECTION
                
            if not(self.sendResponse(Message('NB_LABELS', [self.database.labelsCount()]))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            if not(self.sendResponse(Message('NB_OBJECTS', [self.database.objectsCount()]))):
                return ServerListener.ACTION_CLOSE_CONNECTION
        else:
            if not(self.sendResponse(Message('UNKNOWN_DATABASE', [arguments[0]]))):
                return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'LIST_LABEL_NAMES' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleListLabelNamesCommand(self, arguments):
        if self.database is None:
            if not(self.sendResponse('NO_DATABASE_SELECTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION
        
            return ServerListener.ACTION_NONE

        names = self.database.labelNamesList()
        
        if names is None:
            names = []
            for i in range(0, self.database.labelsCount()):
                names.append(str(i))

        if self.enabledLabels is not None:
            filtered_names = []
            for i in range(0, len(names)):
                if i in self.enabledLabels:
                    filtered_names.append(names[i])
        else:
            filtered_names = names

        for name in filtered_names:
            self.sendResponse(Message('LABEL_NAME', [name]))

        if not(self.sendResponse('END_LIST_LABEL_NAMES')):
            return ServerListener.ACTION_CLOSE_CONNECTION
        
        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'ENABLE_LABELS' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleEnableLabelsCommand(self, arguments):
        if self.database is None:
            if not(self.sendResponse('NO_DATABASE_SELECTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        if len(arguments) == 0:
            if not(self.sendResponse(Message('INVALID_ARGUMENTS', arguments))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        self.enabledLabels = []
        
        try:
            for arg in arguments:
                if isinstance(arg, str) or isinstance(arg, unicode):
                    values = arg.split('-')
                else:
                    values = [arg]

                if len(values) == 2:
                    start = int(values[0])
                    end = int(values[1])
                    if (start > end) or (start >= self.database.labelsCount()) or (end >= self.database.labelsCount()):
                        self.enabledLabels = None

                        if not(self.sendResponse(Message('INVALID_ARGUMENTS', arguments))):
                            return ServerListener.ACTION_CLOSE_CONNECTION

                        return ServerListener.ACTION_NONE
    
                    self.enabledLabels.extend(range(start, end + 1))

                elif len(values) == 1:
                    label = int(values[0])
                    if label >= self.database.labelsCount():
                        self.enabledLabels = None

                        if not(self.sendResponse(Message('INVALID_ARGUMENTS', arguments))):
                            return ServerListener.ACTION_CLOSE_CONNECTION

                        return ServerListener.ACTION_NONE

                    self.enabledLabels.append(label)

        except ValueError:
            self.enabledLabels = None

            if not(self.sendResponse(Message('INVALID_ARGUMENTS', arguments))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE
        
        self.enabledLabels.sort()
        
        (nb_images, nb_labels, nb_objects) = self._computeDatabaseInfos()
        if not(self.sendResponse(Message('NB_IMAGES', [nb_images]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        if not(self.sendResponse(Message('NB_LABELS', [nb_labels]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        if not(self.sendResponse(Message('NB_OBJECTS', [nb_objects]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'RESET_ENABLED_LABELS' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleResetEnabledLabelsCommand(self, arguments):
        if self.database is None:
            if not(self.sendResponse('NO_DATABASE_SELECTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        self.enabledLabels = None
        
        (nb_images, nb_labels, nb_objects) = self._computeDatabaseInfos()
        if not(self.sendResponse(Message('NB_IMAGES', [nb_images]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        if not(self.sendResponse(Message('NB_LABELS', [nb_labels]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        if not(self.sendResponse(Message('NB_OBJECTS', [nb_objects]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'ENABLE_BACKGROUND_IMAGES' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleEnableBackgroundImagesCommand(self, arguments):
        if self.database is None:
            if not(self.sendResponse('NO_DATABASE_SELECTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        self.backgroundImagesEnabled = True

        (nb_images, nb_labels, nb_objects) = self._computeDatabaseInfos()
        if not(self.sendResponse(Message('NB_IMAGES', [nb_images]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        if not(self.sendResponse(Message('NB_LABELS', [nb_labels]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        if not(self.sendResponse(Message('NB_OBJECTS', [nb_objects]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE
    
    #---------------------------------------------------------------------------
    # Called when a 'DISABLE_BACKGROUND_IMAGES' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleDisableBackgroundImagesCommand(self, arguments):
        if self.database is None:
            if not(self.sendResponse('NO_DATABASE_SELECTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        self.backgroundImagesEnabled = False

        (nb_images, nb_labels, nb_objects) = self._computeDatabaseInfos()
        if not(self.sendResponse(Message('NB_IMAGES', [nb_images]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        if not(self.sendResponse(Message('NB_LABELS', [nb_labels]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        if not(self.sendResponse(Message('NB_OBJECTS', [nb_objects]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'REPORT_PREFERRED_IMAGE_SIZE' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleReportPreferredImageSizeCommand(self, arguments):
        if self.database is None:
            if not(self.sendResponse('NO_DATABASE_SELECTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        preferredSize = self.database.preferredImageSize()
        
        if preferredSize is not None:
            if not(self.sendResponse(Message('PREFERRED_IMAGE_SIZE', [preferredSize[0], preferredSize[1]]))):
                return ServerListener.ACTION_CLOSE_CONNECTION
        else:
            if not(self.sendResponse('NOT_SUPPORTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'REPORT_PREFERRED_ROI_SIZE' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleReportPreferredRoiSizeCommand(self, arguments):
        if self.database is None:
            if not(self.sendResponse('NO_DATABASE_SELECTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        preferredSize = self.database.preferredRoiSize()

        if preferredSize is not None:
            if not(self.sendResponse(Message('PREFERRED_ROI_SIZE', [preferredSize]))):
                return ServerListener.ACTION_CLOSE_CONNECTION
        else:
            if not(self.sendResponse('NOT_SUPPORTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'ENABLE_PREPROCESSED_IMAGES' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleEnablePreprocessedImagesCommand(self, arguments):
        if self.database is None:
            if not(self.sendResponse('NO_DATABASE_SELECTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        if not(self.database.usePreprocessedImages(True)):
            if not(self.sendResponse('NOT_SUPPORTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION
        else:
            if not(self.sendResponse('OK')):
                return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'DISABLE_PREPROCESSED_IMAGES' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleDisablePreprocessedImagesCommand(self, arguments):
        if self.database is None:
            if not(self.sendResponse('NO_DATABASE_SELECTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        if not(self.database.usePreprocessedImages(False)):
            if not(self.sendResponse('NOT_SUPPORTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION
        else:
            if not(self.sendResponse('OK')):
                return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'LIST_OBJECTS' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleListObjectsCommand(self, arguments):
        if self.database is None:
            if not(self.sendResponse('NO_DATABASE_SELECTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        if self.backgroundImagesEnabled or (self.enabledImages is None):
            indices = range(0, self.database.imagesCount())
        else:
            indices = self.enabledImages
        
        for i in range(0, len(indices)):
            index = indices[i]
            objects = self.database.objectsInImage(index)

            if self.enabledLabels is not None:
                filtered_objects = []
                for obj in objects:
                    if obj.label in self.enabledLabels:
                        filtered_objects.append(obj)
            else:
                filtered_objects = objects
            
            self.sendResponse(Message('IMAGE', [i]))
            
            size = self.database.imageSize(index)
            self.sendResponse(Message('IMAGE_SIZE', [size[0], size[1]]))

            image_set = self.database.imageSet(index)
            if image_set == Database.TRAINING_SET:
                self.sendResponse(Message('SET', ['TRAINING']))
            elif image_set == Database.TEST_SET:
                self.sendResponse(Message('SET', ['TEST']))
            else:
                self.sendResponse(Message('SET', ['NONE']))

            self.sendResponse(Message('NB_OBJECTS', [len(filtered_objects)]))
            
            for obj in filtered_objects:
                if self.enabledLabels is not None:
                    self.sendResponse(Message('OBJECT_LABEL', [self.enabledLabels.index(obj.label)]))
                else:
                    self.sendResponse(Message('OBJECT_LABEL', [obj.label]))
                self.sendResponse(Message('OBJECT_COORDINATES', [obj.topLeft[0], obj.topLeft[1], obj.bottomRight[0], obj.bottomRight[1]]))

        if not(self.sendResponse('END_LIST_OBJECTS')):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'REPORT_URL_PREFIX' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleReportUrlPrefixCommand(self, arguments):
        if self.database is None:
            if not(self.sendResponse('NO_DATABASE_SELECTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        if arguments is not None:
            if not(self.sendResponse(Message('INVALID_ARGUMENTS', arguments))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        if not(self.sendResponse(Message('URL_PREFIX', [self.database.urlPrefix()]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'IMAGE' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleImageCommand(self, arguments):
        if self.database is None:
            if not(self.sendResponse('NO_DATABASE_SELECTED')):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE
    
        if len(arguments) != 1:
            if not(self.sendResponse(Message('INVALID_ARGUMENTS', arguments))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        index = int(arguments[0])

        if self.backgroundImagesEnabled or (self.enabledImages is None):
            indices = range(0, self.database.imagesCount())
        else:
            indices = self.enabledImages

        if (index >= len(indices)) or (index < 0):
            if not(self.sendResponse('UNKNOWN_IMAGE')):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        if not(self.sendResponse(Message('IMAGE_NAME', [self.database.fullImageName(indices[index])]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    def _computeDatabaseInfos(self):
        if self.database is None:
            return (0, 0, 0)

        self.enabledImages = None

        if self.enabledLabels is None:
            if self.backgroundImagesEnabled:
                return (self.database.imagesCount(), self.database.labelsCount(), self.database.objectsCount())
            
            nb_total_images = self.database.imagesCount()
            self.enabledImages = []

            for i in range(0, nb_total_images):
                objects = self.database.objectsInImage(i)
                if len(objects) > 0:
                    self.enabledImages.append(i)

            return (len(self.enabledImages), self.database.labelsCount(), self.database.objectsCount())
        else:
            if not(self.backgroundImagesEnabled):
                self.enabledImages = []

            nb_total_images = self.database.imagesCount()
            nb_objects = 0
        
            for i in range(0, nb_total_images):
                objects = self.database.objectsInImage(i)

                if len(objects) > 0:
                    old = nb_objects                  
                    for obj in objects:
                        if obj.label in self.enabledLabels:
                            nb_objects += 1

                    if not(self.backgroundImagesEnabled) and (old != nb_objects):
                        self.enabledImages.append(i)

            if not(self.backgroundImagesEnabled):
                return (len(self.enabledImages), len(self.enabledLabels), nb_objects)
            else:
                return (self.database.imagesCount(), len(self.enabledLabels), nb_objects)


##################################### MAIN #####################################

def run():

    print """********************************************************************************
* Image Server
* Protocol: %s
********************************************************************************
""" % PROTOCOL

    # Setup of the command-line arguments parser
    usage = "Usage: %prog [options]"
    parser = OptionParser(usage, version="%prog 1.0")
    parser.add_option("--host", action="store", default="127.0.0.1", type="string",
                      dest="hostname", metavar="ADDRESS",
                      help="The host name or IP address that the server must listen on (default: 127.0.0.1)")
    parser.add_option("--port", action="store", default=11000, type="int",
                      dest="port", metavar="PORT",
                      help="The port that the server must listen on (default: 11000)")
    parser.add_option("--instance", action="store", default="", type="string",
                      dest="instance", metavar="INSTANCE",
                      help="Name of this server instance (default: '')")
    parser.add_option("--config", action="store", default="config", type="string",
                      dest="configurationFile", metavar="FILE", help="Path to the configuration file")
    parser.add_option("--verbose", action="store_true", default=False,
                      dest="verbose", metavar="FILE", help="Verbose mode")

    # Handling of the arguments
    (options, args) = parser.parse_args()

    # Import the configuration
    path = os.path.dirname(os.path.abspath(options.configurationFile))
    if len(path) != 0:
        sys.path.append(path)

    module_name = os.path.basename(options.configurationFile)
    if module_name.endswith('.py'):
        module_name = module_name[:-3]

    config = __import__(module_name)
    
    # Creation of the server
    logFolder = 'logs/%s' % options.instance
    server = Server(logFolder=logFolder)

    # Small trick to have the following infos logged both to the console and the
    # log file even in non-verbose mode
    def log(text):
        server.outStream.write(text + '\n')
        print text

    # Build the list of databases
    log("Loading databases...")
    databases = {}
    for (name, dbconfig) in config.databases.items():
        try:
            if not(dbconfig['enabled']):
                continue
        
            module = __import__(dbconfig['class'])
        
            databases[name] = module.__getattribute__(dbconfig['class'])(dbconfig)

            log("Loaded database '%s'" % name)
        except KeyboardInterrupt:
            raise
        except DatabaseException, e:
            log("Failed to load the '%s' database" % name)
            log("Reason:")
            log(str(e))
            log("")
        except:
            log("Failed to load the '%s' database" % name)
            log("Reason:")
            log(traceback.format_exc())

    if len(databases) == 0:
        log("No database found")
        sys.exit(1)
    else:
        log("\nReady\n")

    ImageServerListener.databases = databases
    ImageServerListener.logFolder = logFolder
    OutStream.outputToConsole = options.verbose

    sys.stdout.flush()

    # Listen for incoming connections
    server.run(options.hostname, options.port, ImageServerListener)



if __name__ == "__main__":
    try:
        run()
    except KeyboardInterrupt:
        print
        print "Shutdown"
        sys.exit(0)
    except SystemExit:
        pass
    except:
        print
        print "An exception occured!"
        print "Details:"
        print traceback.format_exc()
        sys.exit(1)
    