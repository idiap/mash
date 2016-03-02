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


/** @file   communication_channel.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'CommunicationChannel' class
*/

#include "communication_channel.h"
#include <assert.h>
#include <unistd.h>
#include <memory.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

using namespace std;
using namespace Mash;


/********************************** CONSTANTS *********************************/

const unsigned int DEFAULT_BUFFER_SIZE = 1024;


/************************* CONSTRUCTION / DESTRUCTION *************************/

CommunicationChannel::CommunicationChannel()
: _endPoint(ENDPOINT_MASTER), _writefd(-1), _readfd(-1), _lastError(ERROR_NONE),
  _pBuffers(0)
{
}


CommunicationChannel::CommunicationChannel(const CommunicationChannel& channel)
: _endPoint(channel._endPoint), _writefd(-1), _readfd(-1),
  _lastError(channel._lastError), _pBuffers(channel._pBuffers)
{
    if ((channel._writefd >= 0) && (channel._readfd >= 0))
    {
        _writefd = dup(channel._writefd);
        _readfd = dup(channel._readfd);

        // The master use non-blocking file descriptors
        if (_endPoint == ENDPOINT_MASTER)
        {
            int flags = fcntl(_readfd, F_GETFD, 0);
            fcntl(_readfd, F_SETFD, flags | O_NONBLOCK);
        
            flags = fcntl(_writefd, F_GETFD, 0);
            fcntl(_writefd, F_SETFD, flags | O_NONBLOCK);
        }
    }

    if (_pBuffers)
        ++_pBuffers->refCounter;
        
    _outStream = channel._outStream;

    _outStream << "[PID " << getpid() << "] Communication channel created as a copy of "
               << (_endPoint == ENDPOINT_MASTER ? "MASTER" : "SLAVE") << "/" << channel._writefd
               << "/" << channel._readfd << ", using file descriptors " << _writefd
               << " (write) and " << _readfd << " (read)" << endl;
}


CommunicationChannel::~CommunicationChannel()
{
    close();
}


/***************************** CHANNEL MANAGEMENT *****************************/

bool CommunicationChannel::create(CommunicationChannel* master, CommunicationChannel* slave)
{
    // Assertions
    assert(master);
    assert(slave);
    
    // Close the endpoints (just in case)
    master->close();
    slave->close();

    // Create the pipes
    int serverToClientPipe[2];
    int clientToServerPipe[2];

    if (pipe(serverToClientPipe) == -1)
        return false;

    if (pipe(clientToServerPipe) == -1)
    {
        ::close(serverToClientPipe[0]);
        ::close(serverToClientPipe[1]);
        return false;
    }

    // We will lose the pipe as soon as the child process dies, hence let's
    // ignore the corresponding signal and deal with it through the errors of
    // write/read
    signal(SIGPIPE, SIG_IGN);

    // Setup the master endpoint
    master->open(ENDPOINT_MASTER, serverToClientPipe[1], clientToServerPipe[0]);

    // Setup the slave endpoint
    slave->open(ENDPOINT_SLAVE, clientToServerPipe[1], serverToClientPipe[0]);
    
    return true;
}


void CommunicationChannel::open(tEndPoint endPoint, int writefd, int readfd)
{
    close();
    
    // Initialisations
    _endPoint   = endPoint;
    _writefd    = writefd;
    _readfd     = readfd;
    
    // The master use non-blocking file descriptors
    if (endPoint == ENDPOINT_MASTER)
    {
        int flags = fcntl(_readfd, F_GETFD, 0);
        fcntl(_readfd, F_SETFD, flags | O_NONBLOCK);
        
        flags = fcntl(_writefd, F_GETFD, 0);
        fcntl(_writefd, F_SETFD, flags | O_NONBLOCK);
    }
    
    _pBuffers                       = new tBuffers();
    _pBuffers->refCounter           = 1;

    _pBuffers->write.data           = new char[DEFAULT_BUFFER_SIZE];
    _pBuffers->write.packet_start   = _pBuffers->write.data;
    _pBuffers->write.current        = _pBuffers->write.data;
    _pBuffers->write.packet_size    = 0;
    _pBuffers->write.content_size   = 0;
    _pBuffers->write.buffer_size    = DEFAULT_BUFFER_SIZE;
                                    
    _pBuffers->read.data            = new char[DEFAULT_BUFFER_SIZE];
    _pBuffers->read.packet_start    = _pBuffers->read.data;
    _pBuffers->read.current         = _pBuffers->read.data;
    _pBuffers->read.packet_size     = 0;
    _pBuffers->read.content_size    = 0;
    _pBuffers->read.buffer_size     = DEFAULT_BUFFER_SIZE;
    
    _outStream << "[PID " << getpid() << "] Communication channel opened as "
               << (endPoint == ENDPOINT_MASTER ? "MASTER" : "SLAVE") << ", using file descriptors "
               << writefd << " (write) and " << readfd << " (read)" << endl;
}


void CommunicationChannel::close()
{
    if (_writefd >= 0)
        ::close(_writefd);

    if (_readfd >= 0)
        ::close(_readfd);

    if (_pBuffers)
    {
        --_pBuffers->refCounter;

        _outStream << "[PID " << getpid() << "] Communication channel "
                   << (_endPoint == ENDPOINT_MASTER ? "MASTER" : "SLAVE")
                   << "/" << _writefd << "/" << _readfd << ", refCounter = "
                   << _pBuffers->refCounter << endl;

        if (_pBuffers->refCounter == 0)
        {
            delete[] _pBuffers->write.data;
            delete[] _pBuffers->read.data;
            delete _pBuffers;

            _outStream << "[PID " << getpid() << "] Communication channel "
                       << (_endPoint == ENDPOINT_MASTER ? "MASTER" : "SLAVE")
                       << "/" << _writefd << "/" << _readfd << " closed" << endl;
        }

        _pBuffers = 0;
    }

    _writefd = -1;
    _readfd = -1;
    
    _outStream.deleteFile();
}


void CommunicationChannel::operator=(const CommunicationChannel& channel)
{
    close();

    _endPoint = channel._endPoint;
    _lastError = channel._lastError;
    
    if ((channel._writefd >= 0) && (channel._readfd >= 0))
    {
        _writefd = dup(channel._writefd);
        _readfd = dup(channel._readfd);

        // The master use non-blocking file descriptors
        if (_endPoint == ENDPOINT_MASTER)
        {
            int flags = fcntl(_readfd, F_GETFD, 0);
            fcntl(_readfd, F_SETFD, flags | O_NONBLOCK);
        
            flags = fcntl(_writefd, F_GETFD, 0);
            fcntl(_writefd, F_SETFD, flags | O_NONBLOCK);
        }
    }

    _pBuffers = channel._pBuffers;
    if (_pBuffers)
        ++_pBuffers->refCounter;

    _outStream = channel._outStream;

    _outStream << "[PID " << getpid() << "] Communication channel copied from "
               << (_endPoint == ENDPOINT_MASTER ? "MASTER" : "SLAVE")
               << "/" << channel._writefd << "/" << channel._readfd
               << ", using file descriptors " << _writefd << " (write) and "
               << _readfd << " (read)" << endl;
}


void CommunicationChannel::reallocateBuffer(tBuffer* pBuffer, size_t size)
{
    // Assertions
    assert(pBuffer);
    assert(size > 0);

    _outStream << "[Channel] Reallocation of the buffer, from " << pBuffer->buffer_size
               << " to " << size << " bytes" << endl;

    pBuffer->content_size -= pBuffer->packet_start - pBuffer->data;

    // Reallocate the buffer
    char* tmp = pBuffer->data;

    pBuffer->data = new char[size];
    memcpy(pBuffer->data, pBuffer->packet_start, pBuffer->content_size);
        
    delete[] tmp;

    // Setup the buffer
    pBuffer->packet_start   = pBuffer->data;
    pBuffer->current        = pBuffer->packet_start + pBuffer->content_size;
    pBuffer->buffer_size    = size;
}


void CommunicationChannel::dumpData(char* pData, size_t size, unsigned int nbBytesToDump)
{
    if (OutStream::verbosityLevel < 5)
        return;
    
    nbBytesToDump = min(nbBytesToDump, (unsigned int) size);
    
    char* pSrc = pData;
    unsigned int nbLines = (nbBytesToDump + 15) >> 4;
    
    _outStream << "[Channel] Data dump (" << size << " bytes):" << endl;
    
    for (unsigned int i = 0; i < nbLines; ++i)
    {
        _outStream << setbase(16) << "    ";

        for (unsigned int j = 0; (j < 16) && (i * 16 + j < nbBytesToDump); ++j)
        {
            unsigned char v = (unsigned char) *pSrc;
            
            if (v < 0x10)
                _outStream << "0";
            
            _outStream << (unsigned int) v << " ";
            
            if (j == 7)
                _outStream << " ";
                
            ++pSrc;
        }
        
        _outStream << setbase(10) << endl;
    
        if ((nbLines * 16 < size) && (i + 1 == (nbLines >> 1)))
        {
            _outStream << "    ..." << endl;
            
            unsigned int lastLineIndex = ((unsigned int) size - 1) >> 4;
            unsigned int lineOffset = (lastLineIndex - (nbLines - i - 1)) * 16;
            
            pSrc = pData + lineOffset;
            
            nbBytesToDump = i * 16 + size - lineOffset;
        }
    }
}


/***************************** DATA TRANSMISSION ******************************/

tError CommunicationChannel::startPacket(tSandboxMessage message)
{
    // Assertions
    assert(_pBuffers);
    
    if (_lastError != ERROR_NONE)
        return _lastError;

    // Reset the buffer
    _pBuffers->write.packet_start   = _pBuffers->write.data;
    _pBuffers->write.current        = _pBuffers->write.packet_start + sizeof(tPacketHeader);
    _pBuffers->write.packet_size    = sizeof(tPacketHeader);
    _pBuffers->write.content_size   = _pBuffers->write.packet_size;

    // Setup the buffer header
    tPacketHeader* pHeader  = (tPacketHeader*) _pBuffers->write.packet_start;
    pHeader->message        = message;
    pHeader->size           = _pBuffers->write.packet_size;
    
    return ERROR_NONE;
}


void CommunicationChannel::add(const std::string& value)
{
    // Assertions
    assert(_pBuffers);
    
    add((unsigned int) value.size());
    
    if (value.size() > 0)
        add((char*) value.c_str(), value.size());
}


void CommunicationChannel::add(char* pData, size_t size)
{
    // Assertions
    assert(_pBuffers);
    
    if (_lastError != ERROR_NONE)
        return;

    if (_pBuffers->write.buffer_size - _pBuffers->write.packet_size < size)
        reallocateBuffer(&_pBuffers->write, _pBuffers->write.packet_size + size + DEFAULT_BUFFER_SIZE);

    memcpy(_pBuffers->write.current, pData, size);
    _pBuffers->write.current += size;
    _pBuffers->write.packet_size += size;
    _pBuffers->write.content_size += size;
}


tError CommunicationChannel::sendPacket()
{
    // Assertions
    assert(_pBuffers);
    
    if (_lastError != ERROR_NONE)
        return _lastError;

    if (_pBuffers->write.packet_size < sizeof(tPacketHeader))
        return ERROR_NONE;
    
    // Setup the buffer header
    tPacketHeader* pHeader = (tPacketHeader*) _pBuffers->write.packet_start;
    pHeader->size = _pBuffers->write.packet_size;

    if (pHeader->message != SANDBOX_MESSAGE_KEEP_ALIVE)
    {
        _outStream << "[Channel] Sending packet (" << pHeader->size << " bytes, message = "
                   << pHeader->message << ")" << endl;
         dumpData(_pBuffers->write.packet_start, _pBuffers->write.packet_size);
    }

    // Send the packet
    size_t size = _pBuffers->write.packet_size;
    char* pData = _pBuffers->write.packet_start;
    if (_endPoint == ENDPOINT_MASTER)
    {
        fd_set writefds;
        struct timeval tv;

        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        while (size > 0)
        {
            // Check that the slave is ready to receive more data
            FD_ZERO(&writefds);
            FD_SET(_writefd, &writefds);

            int nb = select(_writefd + 1, 0, &writefds, 0, &tv);
            if (nb == 0)
            {
                _lastError = ERROR_CHANNEL_SLAVE_TIMEOUT;
                return _lastError;
            }
            
            if (FD_ISSET(_writefd, &writefds))
            {
                // Write the data to the pipe. If any error occur, we consider that
                // the slave died.
                ssize_t count = write(_writefd, pData, size);
                if (count > 0)
                {
                    pData += count;
                    size -= count;
                }
                else
                {
                    _lastError = ERROR_CHANNEL_SLAVE_CRASHED;
                    return _lastError;
                }
            }
        }
    }
    else
    {
        while (size > 0)
        {
            // Write the data to the pipe. If any error occur, the slave commit
            // suicide.
            ssize_t count = write(_writefd, pData, size);
            if (count > 0)
            {
                pData += count;
                size -= count;
            }
            else
            {
                _exit(0);
            }
        }
    }

    // Reset the buffer
    _pBuffers->write.packet_start   = _pBuffers->write.data;
    _pBuffers->write.current        = _pBuffers->write.packet_start + sizeof(tPacketHeader);
    _pBuffers->write.packet_size    = sizeof(tPacketHeader);
    _pBuffers->write.content_size   = _pBuffers->write.packet_size;

    // Setup the buffer header
    pHeader         = (tPacketHeader*) _pBuffers->write.packet_start;
    pHeader->size   = _pBuffers->write.packet_size;

    return _lastError;
}


/******************************* DATA RECEPTION *******************************/

tError CommunicationChannel::receivePacket(tSandboxMessage* message, unsigned int timeout)
{
    // Assertions
    assert(_pBuffers);

    // Declarations
    tPacketHeader* pHeader = 0;

    if (_lastError != ERROR_NONE)
        return _lastError;

    // Test if the buffer already contain another packet
    if (_pBuffers->read.packet_start + _pBuffers->read.packet_size - _pBuffers->read.data < _pBuffers->read.content_size)
    {
        _pBuffers->read.packet_start += _pBuffers->read.packet_size;

        // If the header of the packet isn't totally contained in the buffer, retrieve the missing part
        if (_pBuffers->read.content_size - (_pBuffers->read.packet_start - _pBuffers->read.data) < sizeof(tPacketHeader))
        {
            _pBuffers->read.packet_size = _pBuffers->read.packet_start - _pBuffers->read.data - _pBuffers->read.content_size;
        
            reallocateBuffer(&_pBuffers->read, _pBuffers->read.buffer_size);
        
            // Read the end of the packet
            _pBuffers->read.content_size += readData(_pBuffers->read.packet_start + _pBuffers->read.content_size,
                                                     _pBuffers->read.buffer_size - _pBuffers->read.content_size,
                                                     _pBuffers->read.packet_start, timeout);
            if (_lastError != ERROR_NONE)
                return _lastError;
        
            pHeader                     = (tPacketHeader*) _pBuffers->read.packet_start;
            _pBuffers->read.packet_size = pHeader->size;
        
            // Test if the packet is totally contained in the buffer, otherwise we reallocate the buffer
            // and retrieve the missing part
            if (pHeader->size > _pBuffers->read.content_size)
            {
                reallocateBuffer(&_pBuffers->read, pHeader->size + DEFAULT_BUFFER_SIZE);
        
                pHeader = (tPacketHeader*) _pBuffers->read.packet_start;
        
                // Read the end of the packet
                _pBuffers->read.content_size += readData(_pBuffers->read.packet_start + _pBuffers->read.content_size,
                                                         pHeader->size - _pBuffers->read.content_size,
                                                         0, timeout);
                if (_lastError != ERROR_NONE)
                    return _lastError;
            }
        }
        else
        {
            pHeader = (tPacketHeader*) _pBuffers->read.packet_start;
            _pBuffers->read.packet_size = pHeader->size;

            // If the packet is totally contained in the buffer, we are done
            if (_pBuffers->read.packet_start + _pBuffers->read.packet_size - _pBuffers->read.data <= _pBuffers->read.content_size)
            {
                _pBuffers->read.current = _pBuffers->read.packet_start + sizeof(tPacketHeader);
            }
        
            // Otherwise we reallocate the buffer, and retrieve the missing part
            else
            {
                reallocateBuffer(&_pBuffers->read, _pBuffers->read.packet_size + DEFAULT_BUFFER_SIZE);

                // Read the end of the packet
                _pBuffers->read.content_size += readData(_pBuffers->read.packet_start + _pBuffers->read.content_size,
                                                         _pBuffers->read.packet_size - _pBuffers->read.content_size,
                                                         0, timeout);
                if (_lastError != ERROR_NONE)
                    return _lastError;

                pHeader                     = (tPacketHeader*) _pBuffers->read.packet_start;
                _pBuffers->read.packet_size = pHeader->size;
            }
        }
    }
    else
    {
        // Reset the buffer
        _pBuffers->read.packet_start    = _pBuffers->read.data;
        _pBuffers->read.current         = _pBuffers->read.packet_start + sizeof(tPacketHeader);
        _pBuffers->read.packet_size     = sizeof(tPacketHeader);
        _pBuffers->read.content_size    = _pBuffers->read.packet_size;

        // Setup the buffer header
        pHeader         = (tPacketHeader*) _pBuffers->read.packet_start;
        pHeader->size   = _pBuffers->read.packet_size;

        // Read the next packet
        _pBuffers->read.content_size = readData(_pBuffers->read.packet_start, _pBuffers->read.buffer_size,
                                                _pBuffers->read.packet_start, timeout);
        if (_lastError != ERROR_NONE)
            return _lastError;

        // Test if the packet is totally contained in the buffer, otherwise we reallocate the buffer
        // and retrieve the missing part
        if (pHeader->size > _pBuffers->read.content_size)
        {
            reallocateBuffer(&_pBuffers->read, pHeader->size + DEFAULT_BUFFER_SIZE);

            pHeader = (tPacketHeader*) _pBuffers->read.packet_start;

            // Read the end of the packet
            _pBuffers->read.content_size += readData(_pBuffers->read.packet_start + _pBuffers->read.content_size,
                                                     pHeader->size - _pBuffers->read.content_size,
                                                     0, timeout);
            if (_lastError != ERROR_NONE)
                return _lastError;
        }

        // Setup the buffer
        _pBuffers->read.packet_size = pHeader->size;
    }

    // Logging
    if (pHeader->message != SANDBOX_MESSAGE_KEEP_ALIVE)
    {
        _outStream << "[Channel] Received packet (" << pHeader->size << " bytes, message = "
                   << pHeader->message << ")" << endl;
         dumpData(_pBuffers->read.packet_start, _pBuffers->read.packet_size);
    }

    *message = pHeader->message;

    _pBuffers->read.current = _pBuffers->read.packet_start + sizeof(tPacketHeader);

    return ERROR_NONE;    
}


bool CommunicationChannel::endOfPacket() const
{
    // Assertions
    assert(_pBuffers);
    
    if (_pBuffers->read.buffer_size == 0)
        return true;

    tPacketHeader* pHeader = (tPacketHeader*) _pBuffers->read.packet_start;
    
    return (_pBuffers->read.current - _pBuffers->read.packet_start == _pBuffers->read.packet_size);
}


bool CommunicationChannel::read(std::string* value)
{
    // Assertions
    assert(_pBuffers);
    
    bool result = true;

    (*value) = "";

    unsigned int size;
    result = read(&size);

    if (result && (size > 0))
    {
        char* buffer = new char[size + 1];
        memset(buffer, 0, size + 1);

        result = read(buffer, size);

        if (result)
            (*value) = buffer;

        delete[] buffer;
    }

    return result;
}


bool CommunicationChannel::read(char* pData, size_t size)
{
    // Assertions
    assert(pData);
    assert(size > 0);
    assert(_pBuffers);

    if ((_pBuffers->read.current - _pBuffers->read.packet_start) + size > _pBuffers->read.packet_size)
        return false;

    memcpy(pData, _pBuffers->read.current, size);
    _pBuffers->read.current += size;

    return true;
}


unsigned int CommunicationChannel::readData(char* pData, size_t size, char* pPacketStart,
                                            unsigned int timeout)
{
    if (_lastError != ERROR_NONE)
        return _lastError;

    // Initialisations
    fd_set readfds;
    struct timeval tv;
    unsigned int received = 0;
    bool bHeaderProcessed = false;
    char* pDst = pData;

    if (_endPoint == ENDPOINT_SLAVE)
        timeout = 0;

    while (size > 0)
    {
        // Master: Wait (briefly) for the slave to send some data
        if (_endPoint == ENDPOINT_MASTER)
        {
            FD_ZERO(&readfds);
            FD_SET(_readfd, &readfds);

            if (timeout > 0)
            {
                div_t result = div(timeout, 1000);
                tv.tv_sec = result.quot;
                tv.tv_usec = result.rem * 1000;
            }
            else
            {
                timeout = 0;
            }
        
            int nb = select(_readfd + 1, &readfds, 0, 0, (timeout > 0 ? &tv : 0));
            if (nb == 0)
            {
                _lastError = ERROR_CHANNEL_SLAVE_TIMEOUT;
                return 0;
            }
            else if (nb < 0)
            {
                if (errno == EINTR)
                    continue;

                _lastError = ERROR_CHANNEL_SLAVE_CRASHED;
                return 0;
            }
        }

        // Read the data from the pipe
        ssize_t count = ::read(_readfd, pDst, size);
        if (count > 0)
        {
            pDst += count;
            received += count;

            // Process the buffer header if necessary
            if (pPacketStart && !bHeaderProcessed && ((pDst - pPacketStart) >= sizeof(tPacketHeader)))
            {
                tPacketHeader* pHeader = (tPacketHeader*) pPacketStart;
                size = max(0, int(min(pHeader->size, size) - (pDst - pPacketStart)));
                bHeaderProcessed = true;
            }
            else
            {
                size -= count;
            }
        }
        
        // Master: If any error occur, we consider that the slave died
        else if (_endPoint == ENDPOINT_MASTER)
        {
            _lastError = ERROR_CHANNEL_SLAVE_CRASHED;
            return 0;
        }
        
        // Slave: If any error occur, the slave commit suicide
        else
        {
            _exit(0);
        }
    }

    return received;    
}
