/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Philip Abbet (philip.abbet@idiap.ch)
*
* This file is part of the MASH Framework.
*
* The MASH Framework is free software: you can redistribute it and/or modify
* it under the terms of either the GNU General Public License version 2 or
* the GNU General Public License version 3 as published by the Free
* Software Foundation, whichever suits the most your needs.
*
* The MASH Framework is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public Licenses
* along with the MASH Framework. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/


/** @file   communication_channel.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'CommunicationChannel' class
*/

#ifndef _MASH_COMMUNICATIONCHANNEL_H_
#define _MASH_COMMUNICATIONCHANNEL_H_

#include <mash-utils/declarations.h>
#include <mash-utils/outstream.h>
#include "sandbox_messages.h"
#include <string>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Represents a communication channel between two processes
    ///
    /// The two endpoints of the channel are called 'master' and 'slave',
    /// because the master is very rude with its slave and doesn't tolerate any
    /// miscommunication or delay.
    ///
    /// It is up to the user to choose the role assumed by each process, and to
    /// correctly call the setEndPoint() method before doing any read/write on
    /// the channel.
    ///
    /// Here is a simple example of utilisation of this class:
    /// @code
    /// CommunicationChannel channel;
    ///
    /// pid_t pid = fork();
    /// if (pid == 0)
    /// {
    ///     channel.setEndPoint(CommunicationChannel::ENDPOINT_SLAVE);
    ///     _exit(0);
    /// }
    ///
    /// channel.setEndPoint(CommunicationChannel::ENDPOINT_MASTER);
    /// @endcode
    //--------------------------------------------------------------------------
    class MASH_SYMBOL CommunicationChannel
    {
        //_____ Internal types __________
    public:
        enum tEndPoint
        {
            ENDPOINT_SLAVE,
            ENDPOINT_MASTER
        };


    private:
        struct tBuffer
        {
            char*           data;
            char*           packet_start;
            char*           current;
            size_t          packet_size;
            size_t          content_size;
            size_t          buffer_size;
        };

        struct tBuffers
        {
            tBuffer         write;
            tBuffer         read;
            unsigned int    refCounter;
        };


#pragma pack(push)
#pragma pack(1)

        struct tPacketHeader
        {
            tSandboxMessage message;
            size_t          size;
        };
        
#pragma pack(pop)

        
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        CommunicationChannel();

        //----------------------------------------------------------------------
        /// @brief  Copy constructor
        //----------------------------------------------------------------------
        CommunicationChannel(const CommunicationChannel& channel);

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~CommunicationChannel();


        //_____ Channel management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Create a pair of channels objects that communicates together
        //----------------------------------------------------------------------
        static bool create(CommunicationChannel* master, CommunicationChannel* slave);
        
        //----------------------------------------------------------------------
        /// @brief  Indicates if the channel can be used
        ///
        /// @remark When an error occurs, since the master is very intolerant,
        ///         the channel can't be used anymore (the slave was killed).
        //----------------------------------------------------------------------
        inline bool good() const
        {
            return ((_lastError == ERROR_NONE) && (_writefd >= 0) && (_readfd >= 0));
        }
        
        //----------------------------------------------------------------------
        /// @brief  Open the channel
        ///
        /// @param  endPoint    The type of endpoint
        /// @param  writefd     File descriptor used to write into the channel
        /// @param  readfd      File descriptor used to read from the channel
        //----------------------------------------------------------------------
        void open(tEndPoint endPoint, int writefd, int readfd);

        //----------------------------------------------------------------------
        /// @brief  Close the channel
        //----------------------------------------------------------------------
        void close();

        //----------------------------------------------------------------------
        /// @brief  Assignement operator
        //----------------------------------------------------------------------
        void operator=(const CommunicationChannel& channel);

        //----------------------------------------------------------------------
        /// @brief  Returns the file descriptor used to write in the channel
        //----------------------------------------------------------------------
        inline int writefd() const
        {
            return _writefd;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the file descriptor used to read from the channel
        //----------------------------------------------------------------------
        inline int readfd() const
        {
            return _readfd;
        }

        //----------------------------------------------------------------------
        /// @brief  Set the output stream to use for logging
        //----------------------------------------------------------------------
        inline void setOutputStream(const OutStream& outStream)
        {
            _outStream = outStream;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the last error that occured
        //----------------------------------------------------------------------
        inline tError getLastError() const
        {
            return _lastError;
        }
    
    private:
        void reallocateBuffer(tBuffer* pBuffer, size_t size);
        void dumpData(char* pData, size_t size, unsigned int nbBytesToDump=320);


        //_____ Data transmission __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Start an new packet of data
        /// @return An error code
        //----------------------------------------------------------------------
        tError startPacket(tSandboxMessage message);

        //----------------------------------------------------------------------
        /// @brief  Add a error code to the packet
        //----------------------------------------------------------------------
        inline void add(tError value)
        {
            add((char*) &value, sizeof(value));
        }

        //----------------------------------------------------------------------
        /// @brief  Add an unsigned integer to the packet
        //----------------------------------------------------------------------
        inline void add(unsigned int value)
        {
            add((char*) &value, sizeof(value));
        }

        //----------------------------------------------------------------------
        /// @brief  Add an integer to the packet
        //----------------------------------------------------------------------
        inline void add(int value)
        {
            add((char*) &value, sizeof(value));
        }

        //----------------------------------------------------------------------
        /// @brief  Add an unsigned short to the packet
        //----------------------------------------------------------------------
        inline void add(unsigned short value)
        {
            add((char*) &value, sizeof(value));
        }

        //----------------------------------------------------------------------
        /// @brief  Add a short to the packet
        //----------------------------------------------------------------------
        inline void add(short value)
        {
            add((char*) &value, sizeof(value));
        }

        //----------------------------------------------------------------------
        /// @brief  Add an unsigned char to the packet
        //----------------------------------------------------------------------
        inline void add(unsigned char value)
        {
            add((char*) &value, sizeof(value));
        }

        //----------------------------------------------------------------------
        /// @brief  Add a char to the packet
        //----------------------------------------------------------------------
        inline void add(char value)
        {
            add((char*) &value, sizeof(value));
        }

        //----------------------------------------------------------------------
        /// @brief  Add a boolean to the packet
        //----------------------------------------------------------------------
        inline void add(bool value)
        {
            add((char*) &value, sizeof(value));
        }

        //----------------------------------------------------------------------
        /// @brief  Add a float to the packet
        //----------------------------------------------------------------------
        inline void add(float value)
        {
            add((char*) &value, sizeof(value));
        }

        //----------------------------------------------------------------------
        /// @brief  Add a double to the packet
        //----------------------------------------------------------------------
        inline void add(double value)
        {
            add((char*) &value, sizeof(value));
        }

        //----------------------------------------------------------------------
        /// @brief  Add a string to the packet
        //----------------------------------------------------------------------
        void add(const std::string& value);

        //----------------------------------------------------------------------
        /// @brief  Add a string to the packet
        //----------------------------------------------------------------------
        inline void add(const char* value)
        {
            add(std::string(value));
        }

        //----------------------------------------------------------------------
        /// @brief  Add a buffer of data to the packet
        ///
        /// @param  pData   The buffer
        /// @param  size    Number of bytes in the buffer
        //----------------------------------------------------------------------
        void add(char* pData, size_t size);

        //----------------------------------------------------------------------
        /// @brief  Send the packet
        /// @return An error code
        //----------------------------------------------------------------------
        tError sendPacket();


        //_____ Data reception __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Begin the reception of a new packet of data
        ///
        /// @remark If the previous packet wasn't totally processed, it will be
        ///         lost.
        ///
        /// @param  timeout     (Master only) Delay before which the slave must
        ///                     send a response, in milliseconds. If 0, no
        ///                     timeout is used.
        /// @return             An error code
        //----------------------------------------------------------------------
        tError receivePacket(tSandboxMessage* message, unsigned int timeout = 0);

        //----------------------------------------------------------------------
        /// @brief  Indicates if the end of the packet has been reached
        //----------------------------------------------------------------------
        bool endOfPacket() const;

        //----------------------------------------------------------------------
        /// @brief  Read an error code from the packet
        //----------------------------------------------------------------------
        inline bool read(tError* value)
        {
            return read((char*) value, sizeof(*value));
        }

        //----------------------------------------------------------------------
        /// @brief  Read an unsigned integer from the packet
        //----------------------------------------------------------------------
        inline bool read(unsigned int* value)
        {
            return read((char*) value, sizeof(*value));
        }

        //----------------------------------------------------------------------
        /// @brief  Read an integer from the packet
        //----------------------------------------------------------------------
        inline bool read(int* value)
        {
            return read((char*) value, sizeof(*value));
        }

        //----------------------------------------------------------------------
        /// @brief  Read an unsigned short from the packet
        //----------------------------------------------------------------------
        inline bool read(unsigned short* value)
        {
            return read((char*) value, sizeof(*value));
        }

        //----------------------------------------------------------------------
        /// @brief  Read a short from the packet
        //----------------------------------------------------------------------
        inline bool read(short* value)
        {
            return read((char*) value, sizeof(*value));
        }

        //----------------------------------------------------------------------
        /// @brief  Read an unsigned char from the packet
        //----------------------------------------------------------------------
        inline bool read(unsigned char* value)
        {
            return read((char*) value, sizeof(*value));
        }

        //----------------------------------------------------------------------
        /// @brief  Read a char from the packet
        //----------------------------------------------------------------------
        inline bool read(char* value)
        {
            return read((char*) value, sizeof(*value));
        }

        //----------------------------------------------------------------------
        /// @brief  Read a boolean from the packet
        //----------------------------------------------------------------------
        inline bool read(bool* value)
        {
            return read((char*) value, sizeof(*value));
        }

        //----------------------------------------------------------------------
        /// @brief  Read a float from the packet
        //----------------------------------------------------------------------
        inline bool read(float* value)
        {
            return read((char*) value, sizeof(*value));
        }

        //----------------------------------------------------------------------
        /// @brief  Read a double from the packet
        //----------------------------------------------------------------------
        inline bool read(double* value)
        {
            return read((char*) value, sizeof(*value));
        }

        //----------------------------------------------------------------------
        /// @brief  Read a string from the packet
        //----------------------------------------------------------------------
        bool read(std::string* value);

        //----------------------------------------------------------------------
        /// @brief  Read a buffer of data from the packet
        ///
        /// @param  pData   The buffer
        /// @param  size    Number of bytes in the buffer
        //----------------------------------------------------------------------
        bool read(char* pData, size_t size);

    private:
        unsigned int readData(char* pData, size_t size, char* pPacketStart,
                              unsigned int timeout);


        //_____ Attributes __________
    private:
        tEndPoint   _endPoint;
        int         _writefd;
        int         _readfd;
        tError      _lastError;
        tBuffers*   _pBuffers;
        OutStream   _outStream;
    };
}

#endif
